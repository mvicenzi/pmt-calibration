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
  bool _SAVE_CRT = false;

  double START_TRACK_Y = 125.;
  double END_TRACK_Y = -175.; 
  double PE_CUT = 50; 
  double MAX_DT_US = 10;
  double MAX_DZ_CM = 30;
 
  std::string const & fileList="/exp/icarus/data/users/mvicenzi/pmt-calibration/input_caltuples/files-caltuple-run" + run + ".list";
  //std::string const & fileList="/exp/icarus/app/users/mvicenzi/timework/jobs/prods/files_calibnutples_inter_xrootd.list";
  
  std::string const & outfilename="/exp/icarus/data/users/mvicenzi/pmt-calibration/track_matches/run" + run + "_matched_light_tracks.root";
  //std::string const & outfilename="/exp/icarus/data/users/mvicenzi/pmt-calibration/track_matches/run" + run + "_matched_light_tracks_INTER_nocosmics.root";

  std::cout << "Input list: " << fileList << std::endl;
  std::cout << "Output: " << outfilename << std::endl;
  
  // define the ttrees
  std::vector< std::string > const lightTtrees{ "simpleLightAna/opflashCryoE_flashtree", 
                                          "simpleLightAna/opflashCryoW_flashtree"  }; // keep the order

  std::vector<std::string >  const chargeTTrees{ "caloskimE/TrackCaloSkim", 
                                           "caloskimW/TrackCaloSkim" }; // keep the order

  std::string const timeCRTTree = "CRTAnalysis/matchTree";

  std::vector<std::string > const matchTTrees{ "trackLightMatchE",
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
  std::vector<double> pmt_time_rise;
  std::vector<double> pmt_time_start;
  std::vector<double> pmt_time_peak;
  std::vector<double> pmt_pe;
  std::vector<double> pmt_amplitude;  
  std::vector<double> pmt_x;  
  std::vector<double> pmt_y;  
  std::vector<double> pmt_z;  
  int crt_nhits;
  std::vector<int> crthit_region;
  std::vector<double> crthit_x;  
  std::vector<double> crthit_y;  
  std::vector<double> crthit_z;  
  std::vector<double> crthit_time_us;
  std::vector<double> crthit_pe;

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
    t->Branch("pmt_time_start",&pmt_time_start);
    t->Branch("pmt_time_rise",&pmt_time_rise);
    t->Branch("pmt_time_peak",&pmt_time_peak);
    t->Branch("pmt_pe",&pmt_pe);
    t->Branch("pmt_amplitude",&pmt_amplitude);
    t->Branch("pmt_x",&pmt_x);
    t->Branch("pmt_y",&pmt_y);
    t->Branch("pmt_z",&pmt_z);
   
    if(_SAVE_CRT)
    {
      t->Branch("crt_nhits",&crt_nhits);
      t->Branch("crthit_region",&crthit_region);
      t->Branch("crthit_x",&crthit_x);
      t->Branch("crthit_y",&crthit_y);
      t->Branch("crthit_z",&crthit_z);
      t->Branch("crthit_time_us",&crthit_time_us);
      t->Branch("crthit_pe",&crthit_pe);
    }

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

   // Read the CRT matches info
   std::map<int, std::vector<CRTPMTMatch>> myCRTMatches;
   if(_SAVE_CRT) myCRTMatches = readCRTMatchInfo( timeCRTTree, tfile );

   // Loop on the events
   for( auto const & [ event, tracks ] : myTPCMap ){
    
     std::vector<OpFlash> flashes = myPMTMap[event];
     std::vector<CRTPMTMatch> crtMatches = myCRTMatches[event];

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
          pmt_time_start.clear();
          pmt_time_rise.clear();
          pmt_time_peak.clear();
          pmt_pe.clear();
          pmt_amplitude.clear();  
          pmt_x.clear();  
          pmt_y.clear();  
          pmt_z.clear(); 
          crt_nhits = 0;
          crthit_region.clear();
          crthit_x.clear();
          crthit_y.clear();
          crthit_z.clear();
          crthit_time_us.clear();
          crthit_pe.clear();

          // find matching CRT hits if they exist
          if(_SAVE_CRT)
          {
           for( auto const & crtMatch : crtMatches ){

             bool selFlashTime = fabs( crtMatch.flash_time_us - flash.time ) < 0.01;     // should match perfectly (same source)
             bool selFlashPosZ = fabs( crtMatch.flash_position.Z - flash.flashZ ) < 10.; // some tolerance, different reconstruction
             bool selFlashPosY = fabs( crtMatch.flash_position.Y - flash.flashY ) < 10.; // some tolerance, different reconstruction

             if( selFlashTime & selFlashPosY & selFlashPosZ ){
               crt_nhits = crtMatch.ncrthits;

               for( auto const & crthit : crtMatch.crthits ){
                 crthit_region.push_back( crthit.region );
                 crthit_x.push_back( crthit.position.X );
                 crthit_y.push_back( crthit.position.Y );
                 crthit_z.push_back( crthit.position.Z );
                 crthit_time_us.push_back( crthit.time );
                 crthit_pe.push_back( crthit.pe );
               }
               break;
             }
           }
          }
           
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
            pmt_time_start.push_back(ophit.startTime);
            pmt_time_rise.push_back(ophit.riseTime);
            pmt_time_peak.push_back(ophit.peakTime);
            pmt_pe.push_back(ophit.pe);
            pmt_amplitude.push_back(ophit.amplitude);
            pmt_x.push_back(ophit.pmt.X);
            pmt_y.push_back(ophit.pmt.Y);
            pmt_z.push_back(ophit.pmt.Z);
          }
      
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
