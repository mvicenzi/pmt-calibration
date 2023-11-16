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

void selectTracks( 
    std::string const & run="9337"
){
 
    std::string const & fileList="inputs/run" + run + "_tracks_BNBMAJORITY_files.txt";
    std::string const & outfilename="output/run" + run + "_matched_light_tracks.txt";

    bool _LIMIT = false;
    double MAX_DT_US = 10;
    double MAX_DZ_CM = 30;

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
         << "firstOpHitTime" << ","
         << "meanTime" << ","
         << "firstOpHitPE" << ","
         << "firstOpHitChannel" << ", "
         << "firstOpHitX" << ","
         << "firstOpHitY" << ","
         << "firstOpHitZ" << ","
         << "timeCorr" << ","
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

    // Correction map applied. If "" is passed as argument that particular level of correction is not appleid
    // Input is a .csv file mapping the value of the correction to the PMT channelID 
    // Result is stored inside pmtCorrectionsMap
    std::map<int, double> pmtCorrectionsMap;
    loadPMTTimeCorrections(
        "", // << Hardware/cable corrections (for the sample considered we took care of it during decoding)
        "",//"/exp/icarus/app/users/mvicenzi/cosmics-timing/inputs/laser_time_corrections_20220408.csv",  //<< Laser equalization
	"", //<< Cosmic muons based fine tuning of the above correction  
        pmtCorrectionsMap
    ); 


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
	auto myPMTMap = readPMTInfo( lightTtrees, tfile, 300., pmtCorrectionsMap);
        
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


    outf.close();
    
    std::cout << matches << " flash/track matches found" << std::endl;
    std::cout << "\nselectTracks() macro: ALL DONE" << std::endl;
    
}
