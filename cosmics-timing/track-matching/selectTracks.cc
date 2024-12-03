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
    
  bool _LIMIT = false;
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

  int runnum, event, cryo, flash_id;
  float flash_time;
  float flash_y, flash_width_y;
  float flash_z, flash_width_z;
  float track_charge_z;
  float track_start_x;
  float track_start_y;
  float track_start_z;
  float track_end_x;
  float track_end_y;
  float track_end_z;
  float track_dir_x;
  float track_dir_y;
  float track_dir_z;
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
    t->Branch("event",&event);
    t->Branch("cryo",&cryo);
    t->Branch("flash_id",&flash_id);
    t->Branch("flash_time",&flash_time);
    t->Branch("flash_y",&flash_y);
    t->Branch("flash_width_y",&flash_width_y);
    t->Branch("flash_z",&flash_z);
    t->Branch("flash_width_z",&flash_width_z);
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

  // Correction map to-be-removed. 
  // If "" is passed as argument that particular level of correction is not removed
  // Input is a .csv file mapping the value of the correction to the PMT channelID 
  // Result is stored inside rmCorrectionsMap
    
  if( !_REMOVE ){ //if NO REMOVAL
    RM_laser = "";  
    RM_cosmics = ""; 
  }

  std::cout << "Preparing to remove corrections..." << std::endl;

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
    
  std::cout << "Preparing to add corrections..." << std::endl;
  
  std::map<int, double> addCorrectionsMap;
  loadPMTTimeCorrections(
    ADD_laser,   // laser equalization
    ADD_cosmics, // cosmics equalization  
    addCorrectionsMap
  ); 

  // -----------------------------------------------------------------------------------------------------
/*
  // Now let the action begin 
  TFile *tfile;
  int matches = 0;
  
  for(  size_t idxFile=0; idxFile<filenames.size(); idxFile++ ){   

    if( _LIMIT && idxFile > 100 ){ break; }

      std::string filename = filenames[idxFile];
      std::cout << "Processing file #" << idxFile << " name: " << filename << std::endl;

      tfile = TFile::Open(filename.c_str(), "READ");

      if( !tfile || tfile->IsZombie() ){ 
        std::cout << "FILE IS NOT RESPONSIVE, SKIP IT" << std::endl;
        continue; 
      }
		
   // Read TPC Info
   auto myTPCMap = readTPCInfo( chargeTTrees, tfile );

   // Read the PMT Info
   auto myPMTMap = readPMTInfo( lightTtrees, tfile, 300., rmCorrectionsMap, addCorrectionsMap);
        
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

                        //Find the earliest OpHit in the flash at the highest PMT Y coordinate
                        std::vector<OpHit> selOpHits;
                        for( auto ophit : flash.ophits ){ 
                            if( ophit.pmt.Y > 80 )
                                selOpHits.push_back( ophit );
                        }

                        // Ignore this match if there are no PMTs at the highest Y coordinate 
                        // (might happen for rare case of misreconstruction or mistmatch)
                        if( selOpHits.size() == 0 ){
                            // Match the flash only once 
                            flashes.erase( flashes.begin()+idx );
                            break;
                        }

                        OpHit firstOphit = *std::min_element( selOpHits.begin(), selOpHits.end(), 
                                []( OpHit const &a, OpHit const &b ){ return a.startTime < b.startTime; });
                        
                        // Work on the mean time! 
                        // We do the calculation separated per wall within the same cryostat 
                        // and then mean the two sides
                        std::map< std::pair<double, double>, std::vector<OpHit> > mapOpHitSide;
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

                        // Here entries are written. One line for every successful match
                         outf << run << ","
                         << event << ","
                         << track.cryo << ","
                         << flash.flashID << ","
                         << flash.time << ","
                         << track.T0 << ","
                         << track.chargeCenter.Z << ","
                         << flash.flashZ << ","
                         << flash.flashY << ","
                         << firstOphit.startTime <<","
                         << meanTime << ","
                         << firstOphit.pe << ","
                         << firstOphit.channel_id << ", "
                         << firstOphit.pmt.X << ","
                         << firstOphit.pmt.Y << ","
                         << firstOphit.pmt.Z << ","
                         << pmtCorrectionsMap[firstOphit.channel_id] << ","
                         << track.start.X << ","
                         << track.start.Y << ","
                         << track.start.Z << ","
                         << track.end.X << ","
                         << track.end.Y << ","
                         << track.end.Z << ","
                         << track.dir.X << ","
                         << track.dir.Y << ","
                         << track.dir.Z << ","
                         << track.length
                         << std::endl;

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
*/

  outfile->Close();
    
  // std::cout << matches << " flash/track matches found" << std::endl;
  std::cout << "\nselectTracks() macro: ALL DONE" << std::endl;
  gApplication->Terminate(0);   
}
