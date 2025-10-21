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
std::map<int, TH1F*> histos;

// ------------------------------------



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


// --------------------------------------------------

std::vector<std::string> makeFilesVector( std::string const & filename ){
  /*
    Read a list of files from a list with name "filename"
    return the same files from the list, but saved in a vector of string
  */
  std::vector<std::string> filesList;
  ifstream ifile(filename.c_str());
  std::string line; 

  while ( std::getline( ifile, line ) ) {
    filesList.push_back(line);
  }

  ifile.close();
  return filesList;

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
    if( iswetopb(ch) ) time -= 0.008; //add -8ns correction
    else if ( iseetopb(ch) ) time += 0.008; // remove -8ns correction
    else if ( iseetopc(ch) ) time += 0.008; // remove -8ns correction
    else if ( iseebotb(ch) ) time += 0.008; // remove -8ns correction
    else if ( iseebotc(ch) ) time += 0.008; // remove -8ns correction
    else if ( isewbotc(ch) ) time += 0.008; // remove -8ns correction
  }
  else if ( run > 12777 )
  {
    if ( iseetopb(ch) ) time += 0.008; // remove -8ns correction
    else if ( iseetopc(ch) ) time += 0.008; // remove -8ns correction
    else if ( iseebotb(ch) ) time += 0.008; // remove -8ns correction
    else if ( iseebotc(ch) ) time += 0.008; // remove -8ns correction
    else if ( isewbotc(ch) ) time += 0.008; // remove -8ns correction
  }
}


void process_flashes(TChain *t, std::string name){

	int entries = t->GetEntries();
	TTreeReader reader(t);

	// main info
	TTreeReaderValue<int> run(reader,"run");
	TTreeReaderValue<int> event(reader,"event");
	TTreeReaderValue<int> timestamp(reader,"timestamp");

	TTreeReaderValue<int> flash_id(reader,"flash_id");
	TTreeReaderValue<float> flash_time(reader,"flash_time");
	TTreeReaderArray<double> stimes(reader,"time_pmt");

        int nhits = 360;

	int entry = 0;
	int rrun = 0;
	int rtimestamp = -1;
	int prev = -1;
      
	while( reader.Next() ){

		int perc = int(float(entry)/entries*100.);
		if( (perc % 10 == 0) && (perc>prev) ){ 
			std::cout << "Processing.. " << entry << "/" << entries <<"  [" << perc << "%]" << std::endl;
			prev = perc;
		}			

		rrun = *run;
		if ( rtimestamp <0 ) rtimestamp = *timestamp;
		else if( rtimestamp > *timestamp) rtimestamp = *timestamp; 
		
		for(int i=0; i<nhits; i++){

			int ch = i;
			double htime = stimes[i];
                        if(htime < 0.0000001 && htime > -0.0000001) continue; // zero means not in the flash
	
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
	for(auto it = histos.begin(); it != histos.end(); it++){

		//std::cout << it->first << " : " << it->second->GetEntries() << std::endl;
		//gr->SetPoint(j,it->first,it->second->GetMean());
		double mean = it->second->GetMean();
                double median = getMedian(it->second);
		outcsv << rrun << "," <<  rtimestamp << "," << it->first << "," << it->second->GetEntries() << "," << mean << "," << median << std::endl;
		//fout->cd(name.c_str());
		//it->second->Write();
		j++;
	}
}

// --- Helper: draw and print one histogram ---
void saveToPdf(TH1F* hist, const std::string& pdfname) {
    if (!hist) return;

    char cname[256];
    sprintf(cname, "c_%s", hist->GetName());
    TCanvas *c = new TCanvas(cname, "Histogram", 800, 600);

    hist->SetLineColor(kBlue + 1);
    hist->SetLineWidth(2);
    hist->Draw();

    double mean = hist->GetMean();
    double median = getMedian(hist);
    double ymax = 1.2 * hist->GetMaximum();

    // --- Mean and Median lines (red) ---
    TLine *lMean = new TLine(mean, 0, mean, ymax);
    lMean->SetLineColor(kRed);
    lMean->SetLineWidth(2);
    lMean->SetLineStyle(2);
    lMean->Draw("SAME");

    TLine *lMedian = new TLine(median, 0, median, ymax);
    lMedian->SetLineColor(kGreen);
    lMedian->SetLineWidth(2);
    lMedian->SetLineStyle(9);
    lMedian->Draw("SAME");

    // --- Add to PDF ---
    c->Print(pdfname.c_str(), "pdf");

    delete c;
}

// --- Main function to save all histos in one multi-page PDF ---
void save_histos(const std::string& basename) {
    if (histos.empty()) {
        std::cout << "No histograms to save!" << std::endl;
        return;
    }

    std::string start = basename + "(";
    std::string mid   = basename;
    std::string end   = basename + ")";

    int i = 0;
    int n = histos.size();
    
    for (auto it=histos.begin(); it!=histos.end(); it++) {
        if (i == 0)
            saveToPdf(it->second, start);
        else if (i == n - 1)
            saveToPdf(it->second, end);
        else
            saveToPdf(it->second, mid);
        i++;
    }
}


void check_offsets_caltuple(int run_number, int pdf=0, int corr=0)
{
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
	std::string path = "/exp/icarus/data/users/mvicenzi/pmt-calibration/input_caltuples/";
	std::string filename = path + "/files-caltuple-run" + run + ".list";  

 	std::vector<std::string> list = makeFilesVector(filename);     
        
        TChain *Echain = new TChain("simpleLightAna/opflashCryoE_flashtree"); 
        TChain *Wchain = new TChain("simpleLightAna/opflashCryoW_flashtree"); 

        for(int i=0; i<list.size(); i++){
          Echain->Add(list[i].c_str());
          Wchain->Add(list[i].c_str());

	  if( i>1999 ) break;
        }

	std::string ocsv = "/exp/icarus/data/users/mvicenzi/pmt-timing/csv/timediff_run" + run + ".csv";
 
        if(useCorrections) ocsv = "/exp/icarus/data/users/mvicenzi/pmt-timing/csv/timediff_corr_run" + run + ".csv";
	outcsv.open(ocsv, ios::out);
	outcsv << "run,timestamp,channel,entries,mean_tdiff,median_tdiff" << std::endl;

	process_flashes(Wchain, "west");
	process_flashes(Echain, "east");

	std::string pdfpath = "/exp/icarus/app/users/mvicenzi/pmt-calibration/cosmics-timing/track-matching/pdfs/offset_run" + run + ".pdf";
	if( pdf>0 ) save_histos(pdfpath); 

	outcsv.close();
	gApplication->Terminate(0);
}




