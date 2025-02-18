#include <TChain.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <string>


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

void check_tracks(const int run)
{

    std::string list = "/exp/icarus/data/users/mvicenzi/pmt-calibration/input_caltuples/files-caltuple-run" + std::to_string(run) + ".list";
    std::string tnameE = "caloskimE/TrackCaloSkim";
    std::string tnameW = "caloskimW/TrackCaloSkim";
 
    std::vector<std::string> frun = getFiles(list);
   
    TChain chain_E(tnameE.c_str());
    TChain chain_W(tnameW.c_str());

    for(auto f : frun)
    { 
      chain_E.Add(f.c_str());
      chain_W.Add(f.c_str());
    }

    std::string _selY = "(start.y>125.&&end.y<-175.)";
    std::string _selCW = "((start.x-210)*(end.x-210)<0)";
    std::string _selCE = "((start.x+210)*(end.x+210)<0)";

    std::string _selE = _selY + "&&" + _selCE;
    std::string _selW = _selY + "&&" + _selCW;

    std::cout << "Applying cuts" << std::endl;

    chain_E.Draw("start.x >> hrE1", "");
    chain_E.Draw("start.x >> hrE2", _selCE.c_str());
    chain_E.Draw("start.x >> hrE3", _selE.c_str());
    chain_W.Draw("start.x >> hrW1", "");
    chain_W.Draw("start.x >> hrW2", _selCW.c_str());
    chain_W.Draw("start.x >> hrW3", _selW.c_str());
    
    std::cout << "Rescaling" << std::endl;

    TH1F *hr_E_1 = (TH1F*)gDirectory->Get("hrE1");
    TH1F *hr_E_2 = (TH1F*)gDirectory->Get("hrE2");
    TH1F *hr_E_3 = (TH1F*)gDirectory->Get("hrE3");
    TH1F *hr_W_1 = (TH1F*)gDirectory->Get("hrW1");
    TH1F *hr_W_2 = (TH1F*)gDirectory->Get("hrW2");
    TH1F *hr_W_3 = (TH1F*)gDirectory->Get("hrW3");
    
    hr_E_1->Scale(1.0 / hr_E_1->Integral());
    hr_W_1->Scale(1.0 / hr_W_1->Integral());
    hr_E_2->Scale(1.0 / hr_E_2->Integral());
    hr_W_2->Scale(1.0 / hr_W_2->Integral());
    hr_E_3->Scale(1.0 / hr_E_3->Integral());
    hr_W_3->Scale(1.0 / hr_W_3->Integral());

    TCanvas *c1 = new TCanvas("c1", "start.x", 1200, 500);
    c1->Divide(2,1);
    c1->cd(1);
    hr_E_1->Draw("HIST");
    hr_E_2->Draw("HIST SAME");
    hr_E_3->Draw("HIST SAME");

    std::string nm = "Run " + std::to_string(run) + " - East";
    hr_E_1->SetTitle(nm.c_str());
    hr_E_1->SetLineColor(kBlack);
    hr_E_2->SetLineColor(kBlue);
    hr_E_3->SetLineColor(kRed);
    hr_E_1->SetStats(0);
    hr_E_2->SetStats(0);
    hr_E_3->SetStats(0);

    TLegend *leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg->AddEntry(hr_E_1, "All", "l");
    leg->AddEntry(hr_E_2, "selCR", "l");
    leg->AddEntry(hr_E_3, "selCR + selY", "l");
    leg->Draw();
    
    c1->cd(2);
    hr_W_1->Draw("HIST");
    hr_W_2->Draw("HIST SAME");
    hr_W_3->Draw("HIST SAME");
    
    nm = "Run " + std::to_string(run) + " - West";
    hr_W_1->SetTitle(nm.c_str());
    hr_W_1->SetLineColor(kBlack);
    hr_W_2->SetLineColor(kBlue);
    hr_W_3->SetLineColor(kRed);
    hr_W_1->SetStats(0);
    hr_W_2->SetStats(0);
    hr_W_3->SetStats(0);

    TLegend *legW = new TLegend(0.7, 0.7, 0.9, 0.9);
    legW->AddEntry(hr_W_1, "All", "l");
    legW->AddEntry(hr_W_2, "selCR", "l");
    legW->AddEntry(hr_W_3, "selCR + selY", "l");
    legW->Draw();
    
    std::string savename = "run" + std::to_string(run) + "_tracks.pdf";
    c1->SaveAs(savename.c_str());
}
