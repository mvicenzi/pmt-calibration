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
int nsum=4;


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



////////////////////////////////////


void single_fit(int run, int channel){

	//int run = 9230;
	std::string inputFilename="histograms/pulseDistributionHist_run" + std::to_string(run) + ".root";
	//std::string  destinationFolder="calibrationdb";

	std::cout << "Reading from " << inputFilename << std::endl;

	int debug = 1;
	//int channel = 23; 

	float rangeLow=0.3, rangeHigh=2.0;
	int modelPedestal=0;


	// The function to fit is initialized here
	if( modelPedestal == 1 ) 
		hasExponential=true;

	if ( debug>0 && hasExponential )
		std::cout << "Use the IdealPMTResponse function between [" << rangeLow << " : " << rangeHigh << "] with exponential background model" << std::endl; 
	else if ( debug>0 && !hasExponential )
		std::cout << "Use the IdealPMTResponse function between [" << rangeLow << " : " << rangeHigh << "] without exponential background model" << std::endl; 

	int nparameters = 6;
	TF1 *fitf = new TF1( "fitFunction", IdealPMTResponse, rangeLow, rangeHigh, nparameters);

	// Here we open the input file and get the first timestamp to give a time reference
	TFile *tfile = new TFile(inputFilename.c_str(), "READ");

	if( !tfile->IsOpen() ){
	std::cout << inputFilename << " not found!" << std::endl;
	return 1;
	}

	std::cout << "Perform fit for run: " << run << ", channel: " << channel << std::endl;

	// Now we fit the histograms
	char histname[100]; 
	sprintf(histname, "bkgcalibration/hintegral%d", channel);

	TH1D *hist;

	// Just ignore the keys not found
	try{
	hist = (TH1D*)tfile->Get( histname );
	}
	catch (...) {
	std::cout << histname << "is a key not found! Ignore" << std::endl;
	return;
	}

	if( !hist ){ return; }

	int nentries = hist->GetEntries();
	if( nentries < 100 ){ 
	std::cout << "Not enough entries!" << std::endl;
	return; 
	}
	
	hist->GetXaxis()->SetRangeUser(0.,3.);
	hist->Draw();

	// Do the fit 
	fitf->SetParameters( 0.1, 0.8, 0.3, hist->Integral()*0.2, hist->Integral()*0.8, -3. );
	fitf->SetParLimits( 0, 0.0, 2.0 );
	fitf->SetParLimits( 1, 0.0, 2.0 );
	fitf->SetParLimits( 2, 0.0, 2.0 );
	fitf->SetParLimits( 3, 0.0, hist->Integral()*10 );
	fitf->SetParLimits( 4, 0.0, hist->Integral()*10 );
	fitf->SetParLimits( 5, -10, 0.0 );

	int fitstatus = hist->Fit("fitFunction", "R", "", rangeLow, rangeHigh);

	int nrefit = 0;
	int maxrefit = 4;

	while( fitstatus !=0 && nrefit < maxrefit ){

		fitstatus = hist->Fit("fitFunction", "R", "", rangeLow, rangeHigh);
		nrefit++;

	}

	// Write the results to file
	std::string line = "pmt,nentries,mu,emu,q,eq,sigma,esigma,amplitude,eamplitude,chi2,ndf,fitstatus\n";
	if( debug>0 )
	std::cout << line << std::endl;

	std::cout << std::to_string(channel) + "," + std::to_string(nentries) + "," ;

	for( int i=0; i<4; i++ )
	std::cout << std::to_string(fitf->GetParameter(i)) + "," + std::to_string(fitf->GetParError(i)) + "," ;

	std::cout << std::to_string(fitf->GetChisquare()) + "," + std::to_string(fitf->GetNDF()) + "," ;
	std::cout << std::to_string(fitstatus) + "\n" ;


	//tfile->Close();
}
