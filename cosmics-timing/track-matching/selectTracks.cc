/////////////////////////////////////////////////////////////////////////////////////////// 
// macro selectTracks()
// 
// Date 12/02/2024 
// Author: M. Vicenzi (BNL), based off work from A. Scarpelli (BNL)
// mailto: mvicenzi@bnl.gov
// 
// Macro to select TPC reconstructed cathode-crossing tracks that match with a PMT reconstructed flash.
// In combination with notebooks/pmt_cosmic_timing.ipynb, it can be used to produce PMT timing corrections 
// 
// Input: 
//   - list with the paths of calibration ntuples files containing the TTree with TPC and PMT information
//   - optional removal of previously applied corrections from timing database
//   - optional addition of corrections from timing database
// Output: root file with the result of the selection to be fed into notebooks
//
// To execute the macro:
//     
//      root -l loadLib.cc selectTracks.cc( <input1>, <input2>, ... ) 
// 
// NB: Sometimes calibration ntuples are not produced with the latest/proper timing corrections.
//     This is true for recent data if the calibration database was not yet updated.
//     The best way is to remove all corrections (except cable ones) and re-apply them.
//     Re-applying them can be also done in the python notebooks.
//     
///////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "DataStructures.h"
#include "Utils.h"

#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>

void selectTracks(
  std::string const & run = "",        // run number
  bool const & _REMOVE = true,         // remove corrections?
  std::string RM_laser = "",   // path of to-be-removed laser corrections
  std::string RM_cosmics = "", // path of to-be-removed cosmics corrections
  bool const & _ADD = false,           // add corrections?
  std::string ADD_laser = "",  // path of to-be-added laser corrections
  std::string ADD_cosmics = "" // path of to-be-added cosmics corrections
  )
{
    
  bool _LIMIT = true;
  int _MAX_FILES = 4000; 
  double START_TRACK_Y = 125.;
  double END_TRACK_Y = -175.; 
  double PE_CUT = 50; 
  double MAX_DT_US = 10;
  double MAX_DZ_CM = 30;
 
  std::string const & fileList="/exp/icarus/data/users/mvicenzi/pmt-calibration/input_caltuples/files-caltuple-run" + run + ".list";
  std::string const & outfilename="/exp/icarus/data/users/mvicenzi/pmt-calibration/track_matches/run" + run + "_matched_light_tracks.root";

  std::cout << "Input list: " << fileList << std::endl;
  std::cout << "Output: " << outfilename << std::endl;
  
  // define the ttrees
  std::vector< std::string > lightTtrees{ "simpleLightAna/opflashCryoE_flashtree", 
                                          "simpleLightAna/opflashCryoW_flashtree"  }; // keep the order

  std::vector<std::string >  chargeTTrees{ "caloskimE/TrackCaloSkim", 
                                           "caloskimW/TrackCaloSkim" }; // keep the order

  std::vector<std::string >  matchTTrees{ "trackLightMatchE",
                                          "trackLightMatchW" }; // keep the order

  // output file 
  TFile *outfile = new TFile(outfilename.c_str(), "RECREATE");
  std::vector<TTree *> outtrees;

  int runnum, ev, cryo, flash_id;
  float flash_time;
  float flash_y, flash_z;
  int flash_nhits;
  float track_time;
  float track_charge_z;
  float track_start_x, track_start_y, track_start_z;
  float track_end_x, track_end_y, track_end_z;
  float track_dir_x, track_dir_y, track_dir_z;
  float track_length;
  std::vector<int> channel_id;
  std::vector<double> pmt_time;
  std::vector<double> pmt_pe;
  std::vector<double> pmt_amplitude;  
  std::vector<double> pmt_x;  
  std::vector<double> pmt_y;  
  std::vector<double> pmt_z;  

  for(auto const &label : matchTTrees)
  {
    std::string info = "TTree for PMT-TPC matched tracks";
    TTree *t = new TTree(label.c_str(),info.c_str());

    t->Branch("run",&runnum);
    t->Branch("event",&ev);
    t->Branch("cryo",&cryo);
    t->Branch("flash_id",&flash_id);
    t->Branch("flash_time",&flash_time);
    t->Branch("flash_y",&flash_y);
    t->Branch("flash_z",&flash_z);
    t->Branch("track_T0",&track_time);
    t->Branch("track_charge_z",&track_charge_z);
    t->Branch("track_start_x",&track_start_x);
    t->Branch("track_start_y",&track_start_y);
    t->Branch("track_start_z",&track_start_z);
    t->Branch("track_end_x",&track_end_x);
    t->Branch("track_end_y",&track_end_y);
    t->Branch("track_end_z",&track_end_z);
    t->Branch("track_dir_x",&track_dir_x);
    t->Branch("track_dir_y",&track_dir_y);
    t->Branch("track_dir_z",&track_dir_z);
    t->Branch("track_length",&track_length);
    t->Branch("flash_nhits",&flash_nhits);
    t->Branch("channel_id",&channel_id);
    t->Branch("pmt_time",&pmt_time);
    t->Branch("pmt_pe",&pmt_pe);
    t->Branch("pmt_amplitude",&pmt_amplitude);
    t->Branch("pmt_x",&pmt_x);
    t->Branch("pmt_y",&pmt_y);
    t->Branch("pmt_z",&pmt_z);
   
    outtrees.push_back(t);
  }
  
  //Read the list of files 
  //NB: keep files separated per run because we only use the event number to match
  std::vector<std::string> filenames = makeFilesVector( fileList );
  std::cout << "There are " << filenames.size() << " files in the list!" << std::endl;

  // Correction map to-be-removed. 
  // If "" is passed as argument that particular level of correction is not removed
  // Input is a .csv file mapping the value of the correction to the PMT channelID 
  // Result is stored inside rmCorrectionsMap
    
  if( !_REMOVE ){ //if NO REMOVAL
    RM_laser = "";  
    RM_cosmics = ""; 
  }

  std::cout << "\nPreparing to REMOVE corrections..." << std::endl;

  std::map<int, double> rmCorrectionsMap;
  loadPMTTimeCorrections(
    RM_laser,   // laser equalization
    RM_cosmics, // cosmics equalization  
    rmCorrectionsMap
  ); 

  // Correction map to-be-applied. 
  // If "" is passed as argument that particular level of correction is not applied
  // Input is a .csv file mapping the value of the correction to the PMT channelID 
  // Result is stored inside addCorrectionsMap
    
  if( !_ADD ){ //if NO ADD
    ADD_laser = "";  
    ADD_cosmics = ""; 
  }
    
  std::cout << "\nPreparing to ADD corrections..." << std::endl;
  
  std::map<int, double> addCorrectionsMap;
  loadPMTTimeCorrections(
    ADD_laser,   // laser equalization
    ADD_cosmics, // cosmics equalization  
    addCorrectionsMap
  ); 

  // -----------------------------------------------------------------------------------------------------

  std::cout << "\nNow buckle-up and let the action begin!" << std::endl;  
  TFile *tfile;
  int matches = 0;
  
  for( size_t idxFile=0; idxFile<filenames.size(); idxFile++ ){   

    // limit the number of files to process, if enabled
    if( _LIMIT && idxFile > _MAX_FILES ){ break; }

      std::string filename = filenames[idxFile];
      std::cout << "Processing file #" << idxFile << " : " << filename << std::endl;

      tfile = TFile::Open(filename.c_str(), "READ");
      if( !tfile || tfile->IsZombie() ){ 
        std::cout << "FILE IS NOT RESPONSIVE, SKIP IT" << std::endl;
        continue; 
      }
		
   // Read TPC Info
   auto myTPCMap = readTPCInfo( chargeTTrees, tfile, START_TRACK_Y, END_TRACK_Y );

   // Read the PMT Info
   auto myPMTMap = readPMTInfo( lightTtrees, tfile, PE_CUT, rmCorrectionsMap, addCorrectionsMap);

   // Loop on the events
   for( auto const & [ event, tracks ] : myTPCMap ){
    
     std::vector<OpFlash> flashes = myPMTMap[event];

     // And loop on the tracks
     for( const Track & track : tracks ){

       for ( size_t idx=0 ;idx<flashes.size(); idx++ ){

         OpFlash const& flash = flashes[idx];

         // Criteria followed to select a match between a flash and a track
         bool selCryo     = track.cryo == flash.cryostat;
         bool selPosition = fabs(track.chargeCenter.Z-flash.flashZ)<MAX_DZ_CM;
         bool selTiming   = (track.T0/1000.-flash.time)>0 && (track.T0/1000.-flash.time)<MAX_DT_US;

	 if( selPosition && selTiming && selCryo ) {
  
           // clear vectors from the previous match
           channel_id.clear();
           pmt_time.clear();
           pmt_pe.clear();
           pmt_amplitude.clear();  
           pmt_x.clear();  
           pmt_y.clear();  
           pmt_z.clear();  
           
           // save flash-track match variables for output tree
           // these are all variables immediately available
           runnum = std::stoi(run);
           ev = event;
           cryo = track.cryo;
           flash_id = flash.flashID;
           flash_time = flash.time;
           flash_y = flash.flashY;
           flash_z = flash.flashZ;
           flash_nhits = flash.ophits.size();
           track_time = track.T0;
           track_charge_z = track.chargeCenter.Z;
           track_start_x = track.start.X;
           track_start_y = track.start.Y;
           track_start_z = track.start.Z;
           track_end_x = track.end.X;
           track_end_y = track.end.Y;
           track_end_z = track.end.Z;
           track_dir_x = track.dir.X;
           track_dir_y = track.dir.Y;
           track_dir_z = track.dir.Z;
           track_length = track.length;
           
           // save the first ophit for each pmt in the flash
           // PE_CUT applies, so tune it for how much to save
           for( auto ophit : flash.ophits ){ 
             channel_id.push_back(ophit.channel_id);
             pmt_time.push_back(ophit.startTime);
             pmt_pe.push_back(ophit.pe);
             pmt_amplitude.push_back(ophit.amplitude);
             pmt_x.push_back(ophit.pmt.X);
             pmt_y.push_back(ophit.pmt.Y);
             pmt_z.push_back(ophit.pmt.Z);
           }
      
           // FIXME: is this important? removing it for now, pe cut is in python notebooks...
           // Ignore this match if there are no PMTs at the highest Y coordinate 
           // (might happen for rare case of misreconstruction or mistmatch)
           /* std::vector< OpHit > selOpHits;
           for( auto ophit : flash.ophits ){ 
             if( ophit.pmt.Y > 80 && ophit.pmt_pe>300. ) // pe_cut was 300 before..
               selOpHits.push_back( ophit );
           }
           if( selOpHits.size() == 0 ){
             // Match the flash only once 
             flashes.erase( flashes.begin()+idx );
             break;
           }*/

           // TODO FIXME: computing the mean time per y COULD here would make things easier!
           // however, it means a new ROOT file for every set of corrections...
           // this would potentially lead to much data to save...
           // FOR NOW: stick to do that in the python code, keeping more flexibility         
           // Work on the mean time! 
           // We do the calculation separated per wall within the same cryostat 
           // and then mean the two sides
           /*std::map< std::pair<double, double>, std::vector<OpHit> > mapOpHitSide;
           std::map<double, double> meanTimeMap;
           for( auto ophit : flash.ophits ){

             std::pair key = std::make_pair( ophit.pmt.X, ophit.pmt.Y );
             mapOpHitSide[key].push_back( ophit );
           }

           for( auto const & [key, ophits] : mapOpHitSide ){

             // Keep only the last side
             if( key.second > 80 ){ continue; }

             if( ophits.size() ==0 ){ continue; }

             // Meantime Not weighed 
             double time = std::accumulate( ophits.begin(), ophits.end(), 0.0, 
                           []( double sum, OpHit const & a ){ return sum+=a.startTime; }) / ophits.size();

             meanTimeMap[ key.first ] = time ;

           }

           if( meanTimeMap.size() !=2  ){
             // Match the flash only once 
             flashes.erase( flashes.begin()+idx );
             break;
           }

           double meanTime = 0;
           for( auto const & [ x, time ] : meanTimeMap ){
             meanTime += time;
           }
           meanTime /= meanTimeMap.size();
           */

           outtrees[cryo]->Fill();
	   matches++;

           // Match the flash only once and erase it from the list if 
           // matching is successufl
           flashes.erase( flashes.begin()+idx );
           break;

           } // end if matches 

        } // end flashes

      } // end cathode crossing tracks in event

    } // Event looper (follow the track loop)

    tfile->Close();

  } // end loop over files

  outfile->cd();
  for(auto t : outtrees) t->Write(); 
  outfile->Close();
    
  std::cout << matches << " flash/track matches found" << std::endl;
  std::cout << "\nselectTracks() macro: ALL DONE" << std::endl;
  gApplication->Terminate(0);   
}
