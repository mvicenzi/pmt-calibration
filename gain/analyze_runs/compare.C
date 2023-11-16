#include <TFile.h>
#include <TH1.h>
#include <TCanvas.h>

void CompareHistograms(const char* file1, const char* file2, const int pmt) {
    // Open the ROOT files
    TFile* f1 = TFile::Open(file1);
    TFile* f2 = TFile::Open(file2);

    if (!f1 || f1->IsZombie()) {
        std::cerr << "Error: Cannot open file1 - " << file1 << std::endl;
        return;
    }
    if (!f2 || f2->IsZombie()) {
        std::cerr << "Error: Cannot open file2 - " << file2 << std::endl;
        return;
    }

    char histogramName[100]; 
    sprintf(histogramName, "bkgcalibration/hintegral%d", pmt);
    
    char histogramName2[100]; 
    sprintf(histogramName2, "bkgcalibration/hamplitude%d", pmt);

    // Get the histograms from each file
    TH1D* hist1 = (TH1D*)f1->Get(histogramName);
    TH1D* hist2 = (TH1D*)f2->Get(histogramName);
    TH1D* hist11 = (TH1D*)f1->Get(histogramName2);
    TH1D* hist22 = (TH1D*)f2->Get(histogramName2);

    if (!hist1) {
        std::cerr << "Error: Histogram not found in file1 - " << histogramName << std::endl;
        f1->Close();
        f2->Close();
        return;
    }
    if (!hist2) {
        std::cerr << "Error: Histogram not found in file2 - " << histogramName << std::endl;
        f1->Close();
        f2->Close();
        return;
    }

    // Create a canvas to draw the histograms side-by-side
    TCanvas* canvas = new TCanvas("canvas", "Histogram Comparison", 800, 600);
    canvas->cd();

    // Draw the histograms with different colors for comparison
    hist1->SetLineColor(kBlue);
    hist2->SetLineColor(kRed);
    hist1->GetXaxis()->SetRangeUser(0,3);
    hist2->GetXaxis()->SetRangeUser(0,3);

    hist1->Draw("HIST");
    hist2->Draw("HIST SAMES");

    gPad->Update();
    TPaveStats* stats1 = (TPaveStats*)hist1->GetListOfFunctions()->FindObject("stats");
    TPaveStats* stats2 = (TPaveStats*)hist2->GetListOfFunctions()->FindObject("stats");
    stats1->SetLineColor(kBlue);
    stats2->SetLineColor(kRed);

    // Reposition the second statistics box to avoid overlap
    stats2->SetX1NDC(stats2->GetX1NDC() - 0.2);
    stats2->SetX2NDC(stats2->GetX2NDC() - 0.2);
    
    // Add a legend to the canvas
    TLegend* legend = new TLegend(0.7, 0.6, 0.9, 0.7);
    legend->AddEntry(hist1, Form("%s - %d", "v09_67_00", pmt), "l");
    legend->AddEntry(hist2, Form("%s - %d", "v09_7x_xx", pmt), "l");
    legend->Draw();
    
    // Create a canvas to draw the histograms side-by-side
    TCanvas* canvas2 = new TCanvas("canvas2", "Histogram Comparison", 800, 600);
    canvas2->cd();

    // Draw the histograms with different colors for comparison
    hist11->SetLineColor(kBlue);
    hist22->SetLineColor(kRed);
    hist11->GetXaxis()->SetRangeUser(0,20);
    hist22->GetXaxis()->SetRangeUser(0,20);

    hist11->Draw("HIST");
    hist22->Draw("HIST SAMES");

    gPad->Update();
    TPaveStats* stats11 = (TPaveStats*)hist11->GetListOfFunctions()->FindObject("stats");
    TPaveStats* stats22 = (TPaveStats*)hist22->GetListOfFunctions()->FindObject("stats");
    stats11->SetLineColor(kBlue);
    stats22->SetLineColor(kRed);

    // Reposition the second statistics box to avoid overlap
    stats22->SetX1NDC(stats22->GetX1NDC() - 0.2);
    stats22->SetX2NDC(stats22->GetX2NDC() - 0.2);
    
    // Add a legend to the canvas
    TLegend* legend2 = new TLegend(0.7, 0.6, 0.9, 0.7);
    legend2->AddEntry(hist11, Form("%s - %d", "v09_67_00", pmt), "l");
    legend2->AddEntry(hist22, Form("%s - %d", "v09_77_00", pmt), "l");
    legend2->Draw();

}

void compare() {
    const char* file1 = "/exp/icarus/data/users/mvicenzi/pmt-calibration/histograms/pulseDistributionHist_run10040.root";
    const char* file2 = "/exp/icarus/data/users/mvicenzi/pmt-calibration/histograms/pulseDistributionHist_run10040_v09_77_00.root";
    const int pmt = 349;

    CompareHistograms(file1, file2, pmt);
}
