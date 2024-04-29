#include <TFile.h>
#include <TH1.h>
#include <TCanvas.h>
#include <vector>
#include <string>


int getIdFromChannel(int channel){

	std::vector<int> channels{ 350, 248, 190, 161, 139, 127, 131,  59,  52,  21, 5 };
	std::vector<int> ids{ 1 , 111, 166, 192, 230, 238, 222, 302, 309, 340, 353 };

	for( int jj=0; jj<channels.size(); jj++){
		if( channels[jj] == channel)
			return ids[jj];
	}	

	return -1;
}

std::vector<int> getVoltages(int ch){
    
	if (ch == 52){ //309
		std::vector<int> buf{1500, 1530, 1550, 1570, 1600, 1650, 1700, 1800, 1900};
		return buf;
	}else if( ch == 59){ //302
		std::vector<int> buf{1500, 1530, 1470, 1450, 1430, 1478, 1478, 1454, 1454};
		return buf;
	}else if( ch == 131){ //222
    		std::vector<int> buf{1500, 1530, 1550, 1470, 1450, 1559, 1559, 1531, 1531};
		return buf;
	}
    	std::vector<int> buf{1500, 1530, 1550, 1470, 1450, 1600, 1700, 1800, 1900};
	return buf;
}

TCanvas *canvas;
TCanvas *ampcanvas;

void multi_compare(int channel) {

    std::string srcpath = "/exp/icarus/data/users/mvicenzi/pmt-calibration/histograms/pulseDistributionHist_run";
    std::vector<std::string> runs{ "11246", "11258", "11260", "11262,11263", "11265", "11275", "11276,11279", "11327", "11328" }; 
    std::vector<int> colors{ kBlue, kCyan, kGreen, kRed, kMagenta, kYellow, kBlack, kOrange, kViolet };
    std::vector<int> voltages = getVoltages(channel);

    // Create a canvas to draw the histograms side-by-side
    std::string title = "Channel " + std::to_string(getIdFromChannel(channel));
    std::string name = "c_" + std::to_string(getIdFromChannel(channel));
    std::string ampname = "ampc_" + std::to_string(getIdFromChannel(channel));
    canvas = new TCanvas(name.c_str(), title.c_str(), 800, 600);
    ampcanvas = new TCanvas(ampname.c_str(), title.c_str(), 800, 600);
    TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    TLegend* amplegend = new TLegend(0.7, 0.7, 0.9, 0.9);
    
    TFile *f;
    TH1D* hist; 

    canvas->cd();
    for(int ii=runs.size()-1; ii>-1; ii--){

	std::string file = srcpath + runs[ii] + ".root";
	std::cout << file << std::endl;
    	f = new TFile(file.c_str());
    
	std::string histogramName = "bkgcalibration/hintegral" + std::to_string(channel);

    	hist = (TH1D*)f->Get(histogramName.c_str());
	std::string htitle = "PMT " + std::to_string(getIdFromChannel(channel)) + " (channel " + std::to_string(channel) + ")"; 
	hist->SetTitle( htitle.c_str() ); 
   	hist->SetStats(0);
	hist->SetLineColor(colors[ii]);
 	hist->SetLineWidth(2);
	hist->GetXaxis()->SetRangeUser(0,2);

    	if( ii==runs.size()-1) hist->Draw("HIST");
	else hist->Draw("HIST SAMES");

	std::string label = std::to_string(voltages[ii]) + "V";
	legend->AddEntry(hist, label.c_str(), "l");
    }
    legend->Draw();

    ampcanvas->cd();
    for(int ii=runs.size()-1; ii>-1; ii--){

	std::string file = srcpath + runs[ii] + ".root";
	std::cout << file << std::endl;
    	f = new TFile(file.c_str());
    
	std::string histogramName = "bkgcalibration/hamplitude" + std::to_string(channel);

    	hist = (TH1D*)f->Get(histogramName.c_str());
	std::string htitle = "PMT " + std::to_string(getIdFromChannel(channel)) + " (channel " + std::to_string(channel) + ")"; 
	hist->SetTitle( htitle.c_str() ); 
   	hist->SetStats(0);
	hist->SetLineColor(colors[ii]);
 	hist->SetLineWidth(2);
	hist->GetXaxis()->SetRangeUser(0,20);

    	if( ii==runs.size()-1) hist->Draw("HIST");
	else hist->Draw("HIST SAMES");

	std::string label = std::to_string(voltages[ii]) + "V";
	amplegend->AddEntry(hist, label.c_str(), "l");
    }
    amplegend->Draw();

}


void print_compare() {

    std::vector<int> channels{ 350, 248, 190, 161, 139, 127, 131, 59, 52, 21, 5};
    for(int jj=0; jj<channels.size(); jj++){
		
	multi_compare(channels[jj]);
    	if( getIdFromChannel(channels[jj]) == 1 ){
		canvas->Print("plots.pdf(","pdf");	
		ampcanvas->Print("ampplots.pdf(","pdf");	
    	}else if( getIdFromChannel(channels[jj]) == 353 ){
		canvas->Print("plots.pdf)","pdf");
		ampcanvas->Print("ampplots.pdf)","pdf");
    	}else {	
		canvas->Print("plots.pdf","pdf");
		ampcanvas->Print("ampplots.pdf","pdf");
	}	
    }
    
    gApplication->Terminate(0);
}
