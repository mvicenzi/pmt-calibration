#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <map>
#include <sqlite3.h>

// global outout definitions
TFile *fout;
std::ofstream outcsv;
bool useCorrections = false;

std::vector<std::string> getFiles(std::string path)
{
  std::vector<std::string> v;
  std::ifstream ifile(path.c_str());
    if (!ifile.is_open())
    {
        std::cerr << "Error opening file: " << path << std::endl;
        return v;
    }

    std::string filename;
    while (std::getline(ifile, filename))
    {
        if (!filename.empty())
        {
            v.push_back(filename);
            std::cout << "Added file: " << filename << std::endl;
        }
    }
    ifile.close();
    return v;
}



// convert channels to board/crate
std::map<int, std::string> board_by_channel;
static int Callback(void* data, int argc, char** argv, char** azColName)
{
	int i;
	int ch = -1;
	int pmt = -1;
	std::string crate = "";
	std::string board = "";

	//fprintf(stderr, "%s: \n", (const char*)data);

	for (i = 0; i < argc; i++) {
		//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		if( std::string(azColName[i]) == "digitizer_label" )
		{
			board = std::string(argv[i]);
		}
		if( string(azColName[i]) == "channel_id" )
			ch = atoi(argv[i]);
		if( string(azColName[i]) == "pmt_id" )
			pmt = atoi(argv[i]);
	}
	//std::cout << board << " " << crate << std::endl;

	board_by_channel[ch] = board;
	return 0;
}

bool iswetopb(int ch){
        if( board_by_channel.at(ch) == "WE-TOP-B" ) return true;
	return false;
}
bool iswetopc(int ch){
        if( board_by_channel.at(ch) == "WE-TOP-C" ) return true;
	return false;
}
bool iseetopb(int ch){   
        if( board_by_channel.at(ch) == "EE-TOP-B" ) return true;
	return false;
}
bool iseetopc(int ch){
        if( board_by_channel.at(ch) == "EE-TOP-C" ) return true;
	return false;
}
bool iseebotb(int ch){
        if( board_by_channel.at(ch) == "EE-BOT-B" ) return true;
	return false;
}
bool iseebotc(int ch){
        if( board_by_channel.at(ch) == "EE-BOT-C" ) return true;
	return false;
}
bool isewbotc(int ch){
        if( board_by_channel.at(ch) == "EW-BOT-C" ) return true;
	return false;
}

// --------------

float getMedian(TH1F *h){

  double med, q=0.5;
  h->GetQuantiles(1,&med,&q);
  return med;
}

// --------------

void apply_correction(int ch, int run, double &time){

  if (run > 12455 && run < 12519 )
  {
  
  if( iswetopc(ch) ) time -= 0.008; //add -8ns correction
  else if( iswetopb(ch) ) time -= 0.008; //add -8ns correction
  //else if ( iseetopb(ch) ) time += 0.008; // remove -8ns correction
  //else if ( iseetopc(ch) ) time += 0.008; // remove -8ns correction
  //else if ( iseebotb(ch) ) time += 0.008; // remove -8ns correction
  //else if ( iseebotc(ch) ) time += 0.008; // remove -8ns correction
  //else if ( isewbotc(ch) ) time += 0.008; // remove -8ns correction
   
  }
  else if ( run > 12777 )
  {
  
  //if ( iseetopb(ch) ) time += 0.008; // remove -8ns correction
  //else if ( iseetopc(ch) ) time += 0.008; // remove -8ns correction
  //else if ( iseebotb(ch) ) time += 0.008; // remove -8ns correction
  //else if ( iseebotc(ch) ) time += 0.008; // remove -8ns correction
  //else if ( isewbotc(ch) ) time += 0.008; // remove -8ns correction

  }

}


void process_flashes(TChain *t, int run_number, std::string name){

	int entries = t->GetEntries();
	TTreeReader reader(t);

	// main info
	TTreeReaderValue<int> run(reader,"run");
	TTreeReaderValue<int> event(reader,"event");
	TTreeReaderValue<int> timestamp(reader,"timestamp");

	TTreeReaderValue<int> flash_id(reader,"flash_id");
	TTreeReaderValue<double> flash_time(reader,"flash_time");
	TTreeReaderValue<int> nhits(reader,"flash_nhits");
	TTreeReaderArray<int> channels(reader,"channels");
	TTreeReaderArray<double> rtimes(reader,"hit_rise_time");
	TTreeReaderArray<double> stimes(reader,"hit_start_time");

	int entry = 0;
	int rrun = 0;
	int rtimestamp = -1;
	int prev = -1;

	std::map<int, TH1F*> histos;

	while( reader.Next() ){

		int perc = int(float(entry)/entries*100.);
		if( (perc % 10 == 0) && (perc>prev) ){ 
			std::cout << "Processing.. " << entry << "/" << entries <<"  [" << perc << "%]" << std::endl;
			prev = perc;
		}		
		
		if(*run != run_number)
		{
		  entry++;
		  continue;
                }
		
		rrun = *run;
		if ( rtimestamp <0 ) rtimestamp = *timestamp;
		else if( rtimestamp > *timestamp) rtimestamp = *timestamp; 
		
		for(int i=0; i<*nhits; i++){

			int ch = channels[i];
			double htime = stimes[i];
	
			if(useCorrections) apply_correction(ch,rrun,htime);		
			double tdiff = htime - *flash_time;

			if( histos.find(ch) == histos.end()){ //first time
				std::string hn = "h" + std::to_string(ch);
				std::string ht = "thit - tflash for ch " + std::to_string(ch);
				TH1F *h = new TH1F(hn.c_str(),ht.c_str(),100,-0.05,0.15);
				h->Fill(tdiff);
				histos[ch] = h;
			} else { //already exists in map
				histos[ch]->Fill(tdiff);
			}
		}
		entry++;
	}

	
	int j=0;
	std::string fname = name + ".pdf";
	std::string sfname = fname + "(";
	std::string efname = fname + ")";
 
	for(auto it = histos.begin(); it != histos.end(); it++){

		//std::cout << it->first << " : " << it->second->GetEntries() << std::endl;
		//gr->SetPoint(j,it->first,it->second->GetMean());
		double mean = it->second->GetMean();
                double median = getMedian(it->second);
		outcsv << rrun << "," <<  rtimestamp << "," << it->first << "," << it->second->GetEntries() << "," << mean << "," << median << std::endl;
		//fout->cd(name.c_str());
		//it->second->Write();
	        if( useCorrections ) {
            	std::string cname = "c_" + std::to_string(it->first) + "_" + board_by_channel.at(it->first);
		TCanvas *c = new TCanvas(cname.c_str(),cname.c_str(),800.,600.);		c->cd();
		it->second->Draw();
                if( j==0) c->Print(sfname.c_str(),"pdf");
                else if( j==histos.size()-1) c->Print(efname.c_str(),"pdf");
                else  c->Print(fname.c_str(),"pdf");
                }
		j++;
	}
}


void check_offsets_beamana(int run_number, int corr=0)
{
	//std::string inputlist = "/exp/icarus/data/users/mvicenzi/pmt-timing/input/files-caltuple-run4.list";
	std::string inputlist = "/pnfs/icarus/scratch/users/mvicenzi/pmt-rwm/BNBmininal/beamnocorr/out/files_calibntuples_xrootd.list";
        std::string path = "/exp/icarus/data/users/mvicenzi/pmt-timing/";

        gROOT->SetBatch(kTRUE);
	if ( corr > 0 ) useCorrections = true;
	
        std::cout << "Reading channel mapping..." << std::endl;

	sqlite3 *mapping_db;
	std::string MappingDBName = "/cvmfs/icarus.opensciencegrid.org/products/icarus/icarus_data/v09_79_02/icarus_data/database/ChannelMapICARUS_20230829.db";
	std::string MappingTableName = "pmt_placements_29Aug2023";
        if( run_number < 10441 ) MappingTableName = "pmt_placements_23Aug2023";
        if( run_number < 10369 ) MappingTableName = "pmt_placements";
	
        if( sqlite3_open( MappingDBName.c_str(), &mapping_db) != 0){
	  std::cout << "!!! ERROR !!! cannot open mapping db." << std::endl;
          return;
	}
	std::string data("CALLBACK FUNCTION");
	int rc = sqlite3_exec(mapping_db, Form("SELECT * FROM %s;", MappingTableName.c_str()), Callback, (void*)data.c_str(), NULL);

	std::string run = std::to_string(run_number);
	std::string filename = path + "/hittiming_run" + run + ".root";  

	std::vector<std::string> files = getFiles(inputlist);
   
 	TChain *chain_E = new TChain("beamana/beamtiming_opflashCryoE");
        TChain *chain_W = new TChain("beamana/beamtiming_opflashCryoW");

	int n = 0;
        for(auto f : files)
        { 
	  if (n == 500) break;
          chain_E->Add(f.c_str());
          chain_W->Add(f.c_str());
          n++;
	}

	std::string ocsv = "/exp/icarus/data/users/mvicenzi/pmt-timing/csv/timediff_nocorr_run" + run + ".csv";
        if(useCorrections) ocsv = "/exp/icarus/data/users/mvicenzi/pmt-timing/csv/timediff_corr_run" + run + ".csv";
	outcsv.open(ocsv, ios::out);
	outcsv << "run,timestamp,channel,entries,mean_tdiff,median_tdiff" << std::endl;

	process_flashes(chain_W, run_number, "west");
	process_flashes(chain_E, run_number, "east");

	outcsv.close();
	gApplication->Terminate(0);
}




