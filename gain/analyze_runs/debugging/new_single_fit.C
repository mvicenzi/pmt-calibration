#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TF1.h"

#include <stdio.h>
#include <fstream>
#include <iostream>

////////////////////// PMT response

bool hasExponential=false;  
int nstart=1;
int nsum=2;

double pedestal( double x, double a0, double c0 ){

	return a0*TMath::Exp( x*c0 );

}

double poissonGauss( double x, double amplitude, double n, double mu, double q, double sigma ){

	return amplitude*(TMath::Power(mu,n)*TMath::Exp(-1.0*mu)/TMath::Factorial(n)
			*TMath::Exp(-1.0*(x-q*n)*(x-q*n)/(2.0*n*sigma*sigma))/(sigma*TMath::Sqrt(2.0*TMath::Pi()*n))) ;

}

double IdealPMTResponse (double *x, double *par) {

	double mu = par[0];
	double q = par[1];
	double sigma = par[2];
	double amplitude = par[3];

	double val = 0;

	for( int n=nstart; n<nstart+nsum; n++ )
		val+=poissonGauss( x[0], amplitude, n, mu, q, sigma );

	if( hasExponential ){
		double a0 = par[4];
		double c0 = par[5];

		val += pedestal( x[0], a0, c0 );
	}

	return val;

}

double logisticFunction(double *x, double *par){

	double x0 = par[0]; // inflection point
	double k = par[1];  // inflection rate
	return 400./(1.+TMath::Exp(-k*(x[0]-x0)));
}

double logisticPMTResponse (double *x, double *par) {

	double mu = par[0];
	double q = par[1];
	double sigma = par[2];
	double amplitude = par[3];

	double val = 0;

	for( int n=nstart; n<nstart+nsum; n++ )
		val+=poissonGauss( x[0], amplitude, n, mu, q, sigma );

	double x0 = par[4];
	double k0 = par[5];

	double pars[2] = {x0,k0};
	val *= logisticFunction( x, pars );

	return val;

}

TF1* computeLogisticFunction(TH1D* hratio, double x0, double k0)
{

    double lw = 0.;
    double hg = x0+(x0-lw);

    TF1 *fitf = new TF1( "fitFunction", logisticFunction, lw, hg, 2);
    fitf->SetParameters(x0,k0);

    int fitstatus = hratio->Fit("fitFunction", "R", "", lw, hg);
    hratio->SetLineColor(kBlack);
    //hratio->Draw();

    return fitf;
}


// ---------------------------------------------------------------------

void new_single_fit(int run, int channel, double low=0.05, double high=1.0){

  std::ostringstream oss;
  oss << "/exp/icarus/data/users/mvicenzi/pmt-calibration/test/pulseDistributionHist_T0.5_run" << run << ".root";
  
  std::string inputFilename = oss.str();
  std::cout << "Reading from " << inputFilename << std::endl;


  std::cout << "Perform fit for run: " << run << ", channel: " << channel << std::endl;

  // Here we open the input file and get the first timestamp to give a time reference
  // We also prepare to extract the ophits from the tree (if needed later)
  
  TFile *tfile = new TFile(inputFilename.c_str(), "UPDATE");

  if( !tfile->IsOpen() ){
    std::cout << inputFilename << " not found!" << std::endl;
    return;
  }

  int ch = -1;
  double amplitude = 0;
  double integral = 0;

  TTree *ttree = (TTree*)tfile->Get("bkgcalibration/ophits");
  ttree->SetBranchAddress("channel", &ch);
  ttree->SetBranchAddress("amplitude", &amplitude);
  ttree->SetBranchAddress("integral", &integral);
  ttree->GetEntry(0);

  std::cout << "Perform fit for run: " << run << std::endl;


  // if cutOnAmplitude is set to true, we need to build a new
  // histogram using only the ophits that survive the amplitude cut

  TH1D* hintegral;
  double threshold; 

  char histname[100];
  char ampname[100];
  sprintf(histname, "bkgcalibration/hintegral%d", channel);
  sprintf(ampname, "bkgcalibration/hamplitude%d", channel);
  TH1D *hamplitude;

    // Just ignore the keys not found
    try{
      hintegral = (TH1D*)tfile->Get( histname );
      hamplitude = (TH1D*)tfile->Get( ampname );
    }
    catch (...) {
      std::cout << histname << " or " << ampname << " is a key not found! Ignore" << std::endl;
      return;
    }

    // skip if empty
    if( !hintegral || !hamplitude ){ return; }
    int nentries = hintegral->GetEntries();
    if( nentries < 100 ){ return; }
      
    // Find the threshold for the amplituce cut: look for the minimum of the
    // amplitude histogram within tunable boundaries
   
    hamplitude->GetXaxis()->SetRangeUser(1.22,4.);
    int minbin = hamplitude->GetMinimumBin();
    threshold = hamplitude->GetXaxis()->GetBinCenter(minbin);

    // same binning and limits, but reset
    TH1D* hist = (TH1D*)hintegral->Clone("hnew");
    hist->Reset();
    
    //change name to later save it in the file
    sprintf(histname, "hintegralcut%d", channel);
    hist->SetName(histname); 
      
  // Build new integral histogram if needed 
  // we already reset them, just loop the tree
    for( int jj=0; jj<ttree->GetEntries(); jj++){
    
      ttree->GetEntry(jj);
      if ( ch != channel ) continue;
      if( amplitude > threshold ) hist->Fill(integral);
    
    }

    TH1D* hratio = (TH1D*)hist->Clone("hratio");
    hratio->Reset();
    for(int ibin = 1; ibin<=hratio->GetNbinsX(); ibin++){
      
	int total = hintegral->GetBinContent(ibin);
	int cut = hist->GetBinContent(ibin);

	if( total < 1 ) continue;

	hratio->SetBinContent(ibin, float(cut)/total*400 );
	//std::cout << float(cut)/total << std::endl;
    }
   
    std::cout << "Amplitude threshold for " << channel << " is " << threshold << std::endl;

    hist->SetStats(0);
    hist->GetXaxis()->SetRangeUser(0.,.8);
    hintegral->GetXaxis()->SetRangeUser(0.,.8);
    hintegral->SetStats(0);
    hratio->GetXaxis()->SetRangeUser(0.,.8);
    hratio->SetLineWidth(2);
    hratio->SetStats(0);
    
    hintegral->Draw();
    hist->SetLineColor(kMagenta);
    hist->Draw("SAME");
    hratio->SetLineColor(kBlack);
    hratio->Draw("SAME");
      
    TLegend* legend = new TLegend(0.6, 0.7, 0.9, 0.9);
    legend->AddEntry(hintegral, "All OpHits", "l");
    legend->AddEntry(hist, "OpHits w/ amplitude cut", "l");
    legend->AddEntry(hratio, "Ratio", "l");
    
    TF1* f = computeLogisticFunction(hratio, 2., 1.);

    std::cout << "x0 " << f->GetParameter(0) << " k0 " << f->GetParameter(1) << std::endl;

    f->Draw("SAME");
    legend->AddEntry(f, "Logistic function fit", "l");
    legend->Draw();
/*    
    // The function to fit is initialized here

     int nparameters = 6;
     TF1 *fitf = new TF1( "fitFunction", logisticPMTResponse, low, high, nparameters);

	// Do the fit 
	fitf->SetParameters( 0.1, 0.8, 0.3, hist->Integral()*0.2, f->GetParameter(0), f->GetParameter(1) );
	fitf->SetParLimits( 0, 0.0, 2.0 );
	fitf->SetParLimits( 1, 0.0, 2.0 );
	fitf->SetParLimits( 2, 0.0, 2.0 );
	fitf->SetParLimits( 3, 0.0, hist->Integral()*10 );
	fitf->FixParameter( 4, f->GetParameter(0) );
	fitf->FixParameter( 5, f->GetParameter(1) );

	int fitstatus = hist->Fit("fitFunction", "R", "", low, high);

	int nrefit = 0;
	int maxrefit = 4;

	while( fitstatus !=0 && nrefit < maxrefit ){

		fitstatus = hist->Fit("fitFunction", "R", "", low, high);
		nrefit++;

	}

	// Write the results to file
	std::string line = "pmt,nentries,mu,emu,q,eq,sigma,esigma,amplitude,eamplitude,chi2,ndf,fitstatus\n";
	std::cout << line << std::endl;

	std::cout << std::to_string(channel) + "," + std::to_string(nentries) + "," ;

	for( int i=0; i<4; i++ )
	std::cout << std::to_string(fitf->GetParameter(i)) + "," + std::to_string(fitf->GetParError(i)) + "," ;

	std::cout << std::to_string(fitf->GetChisquare()) + "," + std::to_string(fitf->GetNDF()) + "," ;
	std::cout << std::to_string(fitstatus) + "\n" ;
 
  //tfile->Close();
 */
} // End main
