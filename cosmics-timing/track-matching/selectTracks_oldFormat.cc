/////////////////////////////////////////////////////////////////////////////////////////// 
// macro selectTracks()
// 
// Date 06/21/2023 
// Author: M. Vicenzi (BNL), based off work from A. Scarpelli (BNL)
// mailto: mvicenzi@bnl.gov
// 
// Macro to select TPC reconstructed cathode-crossing tracks that match with a PMT reconstructed flash.
// In combination with notebooks/pmt_cosmic_timing.ipynb, it can be used to produce PMT timing corrections 
// 
// Input: The name of the list with the path pointing to the calibration ntuples files containing the TTree 
//        with TPC,PMT and CRT information produced at the end of the stage1
// Output: The name of the .csv file reporting the result of the selection to be fed into notebooks
//
// To execute the macro:
//     
//      root -l loadLib.cc selectTracks.cc( <input>, <output> ) 
// 
// NB: Calibration ntuples can be enough if the latest/proper timing corrections from laser
//     measurement have been correctly applied. This is true from recent data, so for old data you need
//     add it now by passing the correct table. 
//     If some were applied and you want to change them or you simply don't know... good luck!
//
///////////////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>

#include "DataStructures.h"
#include "Utils.h"

#include "TFile.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>

#include "TH1D.h"
#include "TH2D.h"

void selectTracks_oldFormat( 
    std::string const & run="8413",
    std::string const & fileList="inputs/run8413_tracks_BNB_files.txt",
    std::string const & outfilename="output/TEST_10us_5cm_run8413_matched_light_tracks.txt"
){
    bool _LIMIT = false;
    double MAX_DT_US = 10;
    double MAX_DZ_CM = 5;

    // Output file 
    ofstream outf( outfilename.c_str() );
    // Header 
    outf << "run" << ","
         << "event" << ","
         << "cryo" << ","
         << "flashID" << ","
         << "flashTime" << ","
         << "trackTime" << ","
         << "trackChargeZ" << ","
         << "flashLightZ" << ","
         << "flashLightY" << ","
         << "trackStartX" << ","
         << "trackStartY" << ","
         << "trackStartZ" << ","
         << "trackEndX" << ","
         << "trackEndY" << ","
         << "trackEndZ" << ","
         << "trackDirX" << ","
         << "trackDirY" << ","
         << "trackDirZ" << ","
         << "trackLenght"
         << std::endl;

    //Read the list of files 
    //NB: keep files separated per run because we only use the event number to match
    std::vector<std::string> filenames = makeFilesVector( fileList );

    // Define the ttrees
    std::vector< std::string > lightTtrees{ "simpleLightAna/opflashCryoE_flashtree", 
                                            "simpleLightAna/opflashCryoW_flashtree"  }; // keep the order

    std::vector<std::string >  chargeTTrees{ "caloskimE/TrackCaloSkim", 
                                             "caloskimW/TrackCaloSkim" }; // keep the order
    // Now let the action begin 
    TFile *tfile;
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
	auto myPMTMap = readPMTInfo_oldFormat( lightTtrees, tfile, 300.);
	        
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


    outf.close();
    

    std::cout << "\nselectTracks_oldFormat() macro: ALL DONE" << std::endl;
    
}
