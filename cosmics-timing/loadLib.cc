
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>


void loadLib(){

  char cwd[1024];
  getcwd(cwd, sizeof(cwd));

  //gStyle->SetPalette(kRainBow);
  //gStyle->SetOptFit(11111);

  std::string currentdir( cwd );
  std::string includepath="-I"+currentdir+"/include/";

  gSystem->SetBuildDir("obj",true);
  gSystem->AddIncludePath(includepath.c_str());
  //gROOT->LoadMacro((currentdir+"/source/DataStructures.cc++").c_str());
  gROOT->LoadMacro((currentdir+"/source/Utils.cc+").c_str());

  #define __INITIALIZED__

}//end loadLib
