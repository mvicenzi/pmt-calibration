#include <vector>
#include <string>


void compare_Tfilter() {

    std::string srcpath = "/exp/icarus/data/users/mvicenzi/pmt-calibration/test/pulseDistributionHist_noT_run11590.root";
    std::string srcpath1 = "/exp/icarus/data/users/mvicenzi/pmt-calibration/test/pulseDistributionHist_T0.1_run11590.root";
    std::string srcpath2 = "/exp/icarus/data/users/mvicenzi/pmt-calibration/test/pulseDistributionHist_T0.5_run11590.root";
    std::string srcpath3 = "/exp/icarus/data/users/mvicenzi/pmt-calibration/test/pulseDistributionHist_T1.0_run11590.root";
    
    std::vector<int> colors { kBlack, kRed, kBlue, kMagenta }; 

    TFile* f = new TFile(srcpath.c_str());
    TFile* f2 = new TFile(srcpath1.c_str());
    TFile* f3 = new TFile(srcpath2.c_str());
    TFile* f4 = new TFile(srcpath3.c_str());
      
    TCanvas **cint = new TCanvas*[360]; 
    TCanvas **camp = new TCanvas*[360]; 
   
    int startch = 0, endch = 359;; 

    for( int channel=startch; channel<endch+1; channel++)
    {   
      std::string histoname = "bkgcalibration/hintegral" + std::to_string(channel);
      std::string hampname = "bkgcalibration/hamplitude" + std::to_string(channel);

      TH1D* hist = (TH1D*)f->Get(histoname.c_str());
      TH1D* hist2 = (TH1D*)f2->Get(histoname.c_str());
      TH1D* hist3 = (TH1D*)f3->Get(histoname.c_str());
      TH1D* hist4 = (TH1D*)f4->Get(histoname.c_str());
      
      TH1D* hamp = (TH1D*)f->Get(hampname.c_str());
      TH1D* hamp2 = (TH1D*)f2->Get(hampname.c_str());
      TH1D* hamp3 = (TH1D*)f3->Get(hampname.c_str());
      TH1D* hamp4 = (TH1D*)f4->Get(hampname.c_str());
     
      std::string tt = "Channel ID " + std::to_string(channel) + " - Run 11590";
      hist->SetTitle(tt.c_str());

      hist->SetStats(0);
      hist2->SetStats(0);
      hist3->SetStats(0);
      hist4->SetStats(0);
      hamp->SetStats(0);
      hamp2->SetStats(0);
      hamp3->SetStats(0);
      hamp4->SetStats(0);

      hist->SetLineColor(colors[0]);
      hist->SetLineWidth(2);
      hist->GetXaxis()->SetRangeUser(0,2);
      hist2->SetLineColor(colors[1]);
      hist2->SetLineWidth(2);
      hist2->GetXaxis()->SetRangeUser(0,2);
      hist3->SetLineColor(colors[2]);
      hist3->SetLineWidth(2);
      hist3->GetXaxis()->SetRangeUser(0,2);
      hist4->SetLineColor(colors[3]);
      hist4->SetLineWidth(2);
      hist4->GetXaxis()->SetRangeUser(0,2);
      
      hamp->SetLineColor(colors[0]);
      hamp->SetLineWidth(2);
      hamp->GetXaxis()->SetRangeUser(0,20);
      hamp2->SetLineColor(colors[1]);
      hamp2->SetLineWidth(2);
      hamp2->GetXaxis()->SetRangeUser(0,20);
      hamp3->SetLineColor(colors[2]);
      hamp3->SetLineWidth(2);
      hamp3->GetXaxis()->SetRangeUser(0,20);
      hamp4->SetLineColor(colors[3]);
      hamp4->SetLineWidth(2);
      hamp4->GetXaxis()->SetRangeUser(0,20);

      cint[channel] = new TCanvas(); 
      cint[channel]->cd();

      hist->Draw("HIST");
      hist2->Draw("HIST SAME");
      hist3->Draw("HIST SAME");
      hist4->Draw("HIST SAME");

      TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
      legend->AddEntry(hist, "All ophits", "l");
      legend->AddEntry(hist2, "100 ns veto", "l");
      legend->AddEntry(hist3, "500 ns veto", "l");
      legend->AddEntry(hist4, "1 us veto", "l");
      legend->Draw();
      
      if( channel == startch ) cint[channel]->Print("run11590_compareTfilter.pdf(","pdf");
      else if( channel == endch ) cint[channel]->Print("run11590_compareTfilter.pdf)","pdf");
      else cint[channel]->Print("run11590_compareTfilter.pdf","pdf");
      
      camp[channel] = new TCanvas(); 
      camp[channel]->cd();

      hamp->Draw("HIST");
      hamp2->Draw("HIST SAME");
      hamp3->Draw("HIST SAME");
      hamp4->Draw("HIST SAME");

      TLegend* legend1 = new TLegend(0.7, 0.7, 0.9, 0.9);
      legend1->AddEntry(hamp, "All ophits", "l");
      legend1->AddEntry(hamp2, "100 ns veto", "l");
      legend1->AddEntry(hamp3, "500 ns veto", "l");
      legend1->AddEntry(hamp4, "1 us veto", "l");
      legend1->Draw();
      
      if( channel == startch ) camp[channel]->Print("run11590_ampTfilter.pdf(","pdf");
      else if( channel == endch ) camp[channel]->Print("run11590_ampTfilter.pdf)","pdf");
      else camp[channel]->Print("run11590_ampTfilter.pdf","pdf");
   }

  gApplication->Terminate(0); 
}
