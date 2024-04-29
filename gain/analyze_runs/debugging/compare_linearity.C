#include <vector>
#include <string>

void compare_linearity() {

    std::string pathrun3 = "/exp/icarus/data/users/mvicenzi/pmt-calibration/test/pulseDistributionHist_T1.0_run11590.root";
    std::string pathrun2 = "/exp/icarus/data/users/mvicenzi/pmt-calibration/test/pulseDistributionHist_T1.0_run9342.root";
    
    std::vector<int> colors { kBlack, kRed }; 

    TFile* f2 = new TFile(pathrun2.c_str());
    TFile* f3 = new TFile(pathrun3.c_str());
    
    std::string treename = "bkgcalibration/ophits";

    TTree* trun2 = (TTree*)f2->Get(treename.c_str());
    TTree* trun3 = (TTree*)f3->Get(treename.c_str());

    TTreeReader reader2(trun2);
    TTreeReader reader3(trun3);

    TTreeReaderValue<int> event2(reader2,"event");
    TTreeReaderValue<int> channel2(reader2,"channel");
    TTreeReaderValue<double> amplitude2(reader2,"amplitude");
    TTreeReaderValue<double> integral2(reader2,"integral");
    TTreeReaderValue<double> time2(reader2,"time");
    TTreeReaderValue<int> event3(reader3,"event");
    TTreeReaderValue<int> channel3(reader3,"channel");
    TTreeReaderValue<double> amplitude3(reader3,"amplitude");
    TTreeReaderValue<double> integral3(reader3,"integral");
    TTreeReaderValue<double> time3(reader3,"time");

    TH2D* hrun2 = new TH2D("hrun2","",2500.,0.,25000,1900.,0.,1900.);
    TH2D* hrun3 = new TH2D("hrun3","",2500.,0.,25000,1900.,0.,1900.);
    
    // zoom
    TH2D* hrun2_zoom = new TH2D("hrun2z","",5000.,0.,5000,500.,0.,500.);
    TH2D* hrun3_zoom = new TH2D("hrun3z","",5000.,0.,5000,500.,0.,500.);
    
    // zoom-zoom
    TH2D* hrun2_zoom2 = new TH2D("hrun2z2","",1500.,0.,1500,250.,0.,250.);
    TH2D* hrun3_zoom2 = new TH2D("hrun3z2","",1500.,0.,1500,250.,0.,250.);
    
    // zoom-zoom-zoom
    TH2D* hrun2_zoom3 = new TH2D("hrun2z3","",100.,0.,10,100.,0.,10.);
    TH2D* hrun3_zoom3 = new TH2D("hrun3z3","",100.,0.,10,100.,0.,10.);

    //double prev_time = -999.;
    //int prev_channel = -999.;
    int nevents_run2 = 0;
    int prev_event = -999;   
    
    while( reader2.Next() ){
    
      if( *time2 < -3. || *time2 > 7. ) continue;

      hrun2->Fill(*integral2,*amplitude2);
      if( *integral2 < 5000. && *amplitude2 < 500. )
        hrun2_zoom->Fill(*integral2,*amplitude2);
      if( *integral2 < 1500. && *amplitude2 < 250. )
        hrun2_zoom2->Fill(*integral2,*amplitude2);
      if( *integral2 < 10. && *amplitude2 < 10. )
        hrun2_zoom3->Fill(*integral2,*amplitude2);
     
      if( *event2 != prev_event ) nevents_run2++;
      prev_event = *event2;
  
      //if(*channel2==prev_channel && *time2-prev_time < 1.0 ) std::cout << *time2 << " " << prev_time << std::endl;
      //prev_channel = *channel2;
      //prev_time = *time2;

    }
    
    int nevents_run3 = 0;
    prev_event = -999;   
    
    while( reader3.Next() ){
      
      if( *time3 < -3. || *time3 > 7. ) continue;
    
      hrun3->Fill(*integral3,*amplitude3);
      if( *integral3 < 5000. && *amplitude3 < 500. )
        hrun3_zoom->Fill(*integral3,*amplitude3);
      if( *integral3 < 1500. && *amplitude3 < 250. )
        hrun3_zoom2->Fill(*integral3,*amplitude3);
      if( *integral3 < 10. && *amplitude3 < 10. )
        hrun3_zoom3->Fill(*integral3,*amplitude3);
      
      if( *event3 != prev_event ) nevents_run3++;
      prev_event = *event3;

    }
    
    int r2_entries = hrun2->GetEntries();
    int r3_entries = hrun3->GetEntries();

    std::cout << "N evts RUN2 " << nevents_run2 << " ophits " << r2_entries << std::endl;
    std::cout << "N evts RUN3 " << nevents_run3 << " ophits " << r3_entries << std::endl;

    hrun2->SetStats(0);
    hrun3->SetStats(0);
    hrun2->SetMarkerColor(colors[0]);
    hrun3->SetMarkerColor(colors[1]);
    hrun2->SetFillColor(colors[0]);
    hrun3->SetFillColor(colors[1]);
    
    hrun2_zoom->SetStats(0);
    hrun3_zoom->SetStats(0);
    hrun2_zoom->SetMarkerColor(colors[0]);
    hrun3_zoom->SetMarkerColor(colors[1]);
    hrun2_zoom->SetFillColor(colors[0]);
    hrun3_zoom->SetFillColor(colors[1]);
    
    hrun2_zoom2->SetStats(0);
    hrun3_zoom2->SetStats(0);
    hrun2_zoom2->SetMarkerColor(colors[0]);
    hrun3_zoom2->SetMarkerColor(colors[1]);
    hrun2_zoom2->SetFillColor(colors[0]);
    hrun3_zoom2->SetFillColor(colors[1]);
    
    hrun2_zoom3->SetStats(0);
    hrun3_zoom3->SetStats(0);
    hrun2_zoom3->SetMarkerColor(colors[0]);
    hrun3_zoom3->SetMarkerColor(colors[1]);
    hrun2_zoom3->SetFillColor(colors[0]);
    hrun3_zoom3->SetFillColor(colors[1]);

    TCanvas *c0 = new TCanvas();
    c0->cd();    

    hrun2->Draw();
    hrun3->Draw("SAME");
    hrun2->GetYaxis()->SetTitle("Ophit amplitude [mV]");
    hrun2->GetYaxis()->SetTitleOffset(1.3);
    hrun2->GetXaxis()->SetTitle("Ophit integral [10^7 electrons]");

    TLegend* legend = new TLegend(0.6, 0.5, 0.9, 0.7); 
    std::string label2 = "RUN-2 (Run 9342)";   
    legend->AddEntry( hrun2, label2.c_str(), "f");
    std::string label3 = "RUN-3 (Run 11590)";   
    legend->AddEntry( hrun3, label3.c_str(), "f");
    legend->Draw();   
  
    c0->SaveAs("ophits_T1.0_amp_vs_int.png");   
    
    TCanvas *c1 = new TCanvas();
    c1->cd();    

    hrun2_zoom->Draw();
    hrun3_zoom->Draw("SAME");
    hrun2_zoom->GetYaxis()->SetTitle("Ophit amplitude [mV]");
    hrun2_zoom->GetYaxis()->SetTitleOffset(1.3);
    hrun2_zoom->GetXaxis()->SetTitle("Ophit integral [10^7 electrons]");

    TLegend* legend1 = new TLegend(0.7, 0.7, 0.9, 0.9); 
    label2 = "RUN-2 (Run 9342)";   
    legend1->AddEntry( hrun2_zoom, label2.c_str(), "f");
    label3 = "RUN-3 (Run 11590)";   
    legend1->AddEntry( hrun3_zoom, label3.c_str(), "f");
    legend1->Draw();   
  
    c1->SaveAs("ophits_T1.0_amp_vs_int_zoom.png");   
    
    TCanvas *c2 = new TCanvas();
    c2->cd();    

    hrun2_zoom2->Draw();
    hrun3_zoom2->Draw("SAME");
    hrun2_zoom2->GetYaxis()->SetTitle("Ophit amplitude [mV]");
    hrun2_zoom2->GetYaxis()->SetTitleOffset(1.3);
    hrun2_zoom2->GetXaxis()->SetTitle("Ophit integral [10^7 electrons]");

    TLegend* legend2 = new TLegend(0.7, 0.7, 0.9, 0.9); 
    label2 = "RUN-2 (Run 9342)";   
    legend2->AddEntry( hrun2_zoom2, label2.c_str(), "f");
    label3 = "RUN-3 (Run 11590)";   
    legend2->AddEntry( hrun3_zoom2, label3.c_str(), "f");
    legend2->Draw();   
  
    c2->SaveAs("ophits_T1.0_amp_vs_int_zoom2.png");   
    
    TCanvas *c3 = new TCanvas();
    c3->cd();    

    hrun2_zoom2->SetTitle(label2.c_str());
    //hrun2_zoom2->Draw("COLZ");
    hrun2_zoom2->Draw("");
    hrun2_zoom2->GetYaxis()->SetTitle("Ophit amplitude [mV]");
    hrun2_zoom2->GetYaxis()->SetTitleOffset(1.3);
    hrun2_zoom2->GetXaxis()->SetTitle("Ophit integral [10^7 electrons]");

    c3->SaveAs("ophits_T1.0_run2_amp_vs_int.png");   
    
    TCanvas *c4 = new TCanvas();
    c4->cd();    

    hrun3_zoom2->SetTitle(label3.c_str());
    hrun3_zoom2->Draw("COLZ");
    hrun3_zoom2->GetYaxis()->SetTitle("Ophit amplitude [mV]");
    hrun3_zoom2->GetYaxis()->SetTitleOffset(1.3);
    hrun3_zoom2->GetXaxis()->SetTitle("Ophit integral [10^7 electrons]");

    c4->SaveAs("ophits_T1.0_run3_amp_vs_int.png");   
    
    TCanvas *c5 = new TCanvas();
    c5->cd();    

    hrun2_zoom3->Draw();
    hrun3_zoom3->Draw("SAME");
    hrun2_zoom3->GetYaxis()->SetTitle("Ophit amplitude [mV]");
    hrun2_zoom3->GetYaxis()->SetTitleOffset(1.3);
    hrun2_zoom3->GetXaxis()->SetTitle("Ophit integral [10^7 electrons]");

    TLegend* legend5 = new TLegend(0.7, 0.7, 0.9, 0.9); 
    label2 = "RUN-2 (Run 9342)";   
    legend5->AddEntry( hrun2_zoom3, label2.c_str(), "f");
    label3 = "RUN-3 (Run 11590)";   
    legend5->AddEntry( hrun3_zoom3, label3.c_str(), "f");
    legend5->Draw();   
  
    c5->SaveAs("ophits_T1.0_amp_vs_int_zoom3.png");   
}
