#include "Utils.h"

#include "TVector3.h"
#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>


struct Vector{

    double X;
    double Y;
    double Z;

};

//-------------------------------------------------------------------------------
//------------------------------    TRIGGER -------------------------------------
//-------------------------------------------------------------------------------

struct Trigger{
    
    uint64_t triggerTime;
    uint64_t gateTime ;
    uint64_t timeDiff;
    unsigned int gateType;

    std::vector<uint64_t> lvdsStateEast;
    std::vector<uint64_t> lvdsStateWest;

    //std::vector<int> activeChannels;

    //Vector position;

};


std::map<int, Trigger> readTriggerInfo(
    const std::string & ttreename, TFile *myTFile
 ){

    std::map<int, Trigger> myTriggerMap;

    TTreeReader myReaderTrigger(ttreename.c_str(), myTFile);

    TTreeReaderValue<int> event(myReaderTrigger, "event"); 
    TTreeReaderValue<unsigned long long> triggerTimestamp(myReaderTrigger, "trigger_timestamp");
    TTreeReaderValue<unsigned long long> gateTimestamp(myReaderTrigger, "gate_start_timestamp");
    TTreeReaderValue<unsigned long long> timeDifference(myReaderTrigger, "trigger_gate_diff");
    TTreeReaderArray<unsigned long long> lvdsCryoW(myReaderTrigger, "lvdsCryoW");
    TTreeReaderArray<unsigned long long> lvdsCryoE(myReaderTrigger, "lvdsCryoE");
    TTreeReaderValue<unsigned char> gateType(myReaderTrigger, "gate_type");

    while( myReaderTrigger.Next() ){
        /*
        //std::vector<int> lvdsExclude{ 0,1,2 };

        std::vector<pmtPair_t> activePMTPairs;
        activeLVDSToPMT( 0, lvdsCryoE, 
            pmtLVDSMap, activePMTPairs ); 
        activeLVDSToPMT( 1, lvdsCryoW, 
            pmtLVDSMap, activePMTPairs ); 
            
        if( activePMTPairs.size() == 0 ){ continue; } 

        double centerX=0, centerY=0, centerZ=0, centerR=0; 
        std::vector<int> countActivePMTs;
        for( pmtPair_t const & activePair : activePMTPairs ){
            for( pmtInfo const & pairInfo : activePair ){
                if( found(pairInfo.channelId, lvdsExclude) ){continue;}
                centerX += pairInfo.X;
                centerY += pairInfo.Y;
                centerZ += pairInfo.Z;
                countActivePMTs.push_back(pairInfo.channelId,);
            }
        }
        centerX /= countActivePMTs.size();
        centerY /= countActivePMTs.size();
        centerZ /= countActivePMTs.size();
        */

        Trigger thisTrigger{ 
            (*triggerTimestamp),
            (*gateTimestamp),
            (*timeDifference),
            (unsigned int)(*gateType),
            {lvdsCryoE[0],lvdsCryoE[1]},
            {lvdsCryoW[0],lvdsCryoW[1]}
            //countActivePMTs,
            //{centerX, centerY, centerZ}
        };

        myTriggerMap[(*event)] = thisTrigger;

    }


    return myTriggerMap;

};


/// Dummy function to select flashes in time
bool flashInTime( double const & flashTime, Trigger const & trigger ){

    std::map<unsigned int, double> gateLength{
        {1, 6.2 },  //BNB
        {2, 14.1 }, // NuMI 
        {3, 6.2 },  // BNB offbeam
        {4, 14 }  // NuMI Offbeam
    };

    double vetoOffset = 3.5; 

    double activeGate = gateLength[trigger.gateType] - vetoOffset;
    double relFlashTime = flashTime + trigger.timeDiff/1000. - vetoOffset;

    //std::cout << flashTime << " " << relFlashTime << " " << activeGate << " " << trigger.timeDiff/1000. << std::endl;

    return ( (relFlashTime > 0) && (relFlashTime < activeGate) ); 

}


//----------------------------------------------------------------------------------------------------------------

std::vector<std::string> 
    makeFilesVector( std::string const & filename ){

        /* 
        Read a list of files from a list with name "filename"
        return the same files from the list, but saved in a vector of string
        */
        std::vector<std::string> filesList;
        ifstream ifile(filename.c_str());
        std::string line; 

        while ( std::getline( ifile, line ) ) {
            filesList.push_back(line);
        }

        ifile.close();
        return filesList;
};

void loadPMTTimeCorrections( 
    std::string const & laserCorrections,
    std::string const & muonsCosmicCorrections, 
    std::map<int, double> & timeMap ){

    std::ifstream ifile;

    if( !laserCorrections.empty() ){

	std::cout << "loadPMTTimeCorrections: " << laserCorrections << std::endl;

        ifile.open( laserCorrections, ios::in );
        std::string line;

        //This is the first line, so the header
        std::getline(ifile, line);

        while( std::getline(ifile, line) ){

            std::stringstream ss(line);
            std::string stringItem;
            std::vector<std::string> row;

            while( std::getline(ss, stringItem, ',') ){
                row.push_back( stringItem );
            }

            //Here we save the items that we want from the row
            int channelId = stoi( row[0] );
            double electronTransitTime = stod( row[5] );
            
            timeMap[ channelId ] += ((-electronTransitTime)/1000.);
        }

        ifile.close();

    } else {
        std::cout << "loadPMTTimeCorrections: skip laser corrections" << std::endl;
    }


    if( !muonsCosmicCorrections.empty() ){
        
	std::cout << "loadPMTTimeCorrections: " << muonsCosmicCorrections << std::endl;

        ifile.open( muonsCosmicCorrections, ios::in );
        std::string line;

        //This is the first line, so the header
        std::getline(ifile, line);

        while( std::getline(ifile, line) ){

            std::stringstream ss(line);
            std::string stringItem;
            std::vector<std::string> row;

            while( std::getline(ss, stringItem, ',') ){
                row.push_back( stringItem );
            }

            //Here we save the items that we want from the row
            int channelId = stoi( row[0] );
            double muonResidual = stod( row[6] );
            
            timeMap[ channelId ] += ((-muonResidual)/1000.);
        }

        ifile.close();

    } else {
        std::cout << "loadPMTTimeCorrections: skip muon cosmics corrections" << std::endl;
    }

};

//-------------------------------------------------------------------------------
//------------------------------  TPC STUFF   -----------------------------------
//-------------------------------------------------------------------------------

struct Track{

    //int id;
    size_t cryo;
    double T0;
    Vector start;
    Vector end;
    Vector dir;
    Vector chargeCenter;

    double length;

};


bool cathodeCross(int cryoidx, float startx, float endx){

	float xcathode[2] = { -210.29, 210.29 }; //cathode for east/west
	float c = xcathode[cryoidx];
	// start/end must be on opposite side of cathode
	if ( (startx-c)*(endx-c) < 0 ) return true;
	return false;	
}


std::map<int, std::vector<Track>> 
    readTPCInfo( const std::vector<std::string> & ttreename, TFile *myTFile, const double startTrackY, const double endTrackY ){

    std::map<int, std::vector<Track>> myTrackMap; 
		
    for( size_t cryoidx=0; cryoidx<ttreename.size(); cryoidx++ ){

            TTreeReader myReaderTPC(ttreename[cryoidx].c_str(), myTFile);

            TTreeReaderValue<int>   runTPC(myReaderTPC, "meta.run"); 
            TTreeReaderValue<int>   eventTPC(myReaderTPC, "meta.evt"); 
            TTreeReaderValue<float> startz(myReaderTPC, "start.z"); 
            TTreeReaderValue<float> t0(myReaderTPC, "t0PFP"); 
            TTreeReaderValue<float> endz(myReaderTPC, "end.z"); 
            TTreeReaderValue<float> length(myReaderTPC, "length");
            TTreeReaderValue<float> startx(myReaderTPC, "start.x");
            TTreeReaderValue<float> starty(myReaderTPC, "start.y");
            TTreeReaderValue<float> endx(myReaderTPC, "end.x");
            TTreeReaderValue<float> endy(myReaderTPC, "end.y");
            TTreeReaderValue<float> dirx(myReaderTPC, "dir.x");
            TTreeReaderValue<float> diry(myReaderTPC, "dir.y");
            TTreeReaderValue<float> dirz(myReaderTPC, "dir.z");
            TTreeReaderArray<float> hz(myReaderTPC, "hits2.h.sp.z"); // hits on plane 2 (Collection)
            TTreeReaderArray<float> hy(myReaderTPC, "hits2.h.sp.y"); // hits on plane 2 (Collection)
            TTreeReaderArray<float> hx(myReaderTPC, "hits2.h.sp.x"); // hits on plane 2 (Collection)
            TTreeReaderArray<float> hi(myReaderTPC, "hits2.h.integral"); // hits on plane 2 (Collection)
		
            while( myReaderTPC.Next() ){

                // Get the charge barycenter 
                float average_z_charge=0;
                float average_y_charge=0;
                float average_x_charge=0;
                float total_charge=0;
                
                for (unsigned i = 0; i < hz.GetSize(); i++) {
         
                    if(hz[i]>-1000 && hz[i]<1000){
                        average_z_charge+=hz[i]*hi[i];
                        average_y_charge+=hy[i]*hi[i];
                        average_x_charge+=hx[i]*hi[i];
                        total_charge+=hi[i];
                    }
                }

                average_z_charge=average_z_charge/total_charge;
                average_y_charge=average_y_charge/total_charge;
                average_x_charge=average_x_charge/total_charge;

                double dx = (*endx)-(*startx);
                double dy = (*endy)-(*starty);
                double dz = (*endz)-(*startz);

                double length = sqrt( dx*dx + dy*dy + dz*dz );

                // Keep only the tracks that are crossing the cathode at steep angle
                // This also forces the track to be longer than 300cm
                if( cathodeCross(cryoidx, *startx, *endx) &&
		    (*starty)>startTrackY && (*endy)<endTrackY ) {

                    Track thisTrack{ 
                        cryoidx,
                        (*t0),
                        {(*startx), (*starty), (*startz)},
                        {(*endx), (*endy), (*endz)},
                        {(*dirx), (*diry), (*dirz)}, 
                        {average_x_charge, average_y_charge, average_z_charge},
                        length
                    };

                    myTrackMap[ (*eventTPC) ].push_back( thisTrack );
                }
            }
        }


        return myTrackMap;

};

//----------------------------------------------------------------------------------------------------------------
//----------------------------    PMT STUFF  ---------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------

struct OpHit {

    unsigned int channel_id;
    double startTime;
    double riseTime;
    double peakTime;
    double pe;
    double amplitude;
    Vector pmt;

};


struct OpFlash {
  
  int flashID;
  size_t cryostat;
  double time;
  double flashZ;
  double flashY;
  std::vector<OpHit> ophits; 
  
};


std::map<int, std::vector<OpFlash>>
    readPMTInfo( const std::vector<std::string> & ttreename, 
        TFile *myTFile, const float & peCut, std::map<int, double> rmCorrectionsMap, std::map<int,double> addCorrectionsMap ){

    std::map<int, std::vector<OpFlash>> myFlashMap; 

    for( size_t cryoidx=0; cryoidx<ttreename.size(); cryoidx++ ){

        TTreeReader myReaderPMT(ttreename[cryoidx].c_str(), myTFile);

        TTreeReaderValue<int> event(myReaderPMT, "event");
        TTreeReaderValue<int> flashID(myReaderPMT, "flash_id");
        TTreeReaderValue<float> flashTime(myReaderPMT, "flash_time");
        TTreeReaderValue<float> flashY(myReaderPMT, "flash_y");
        TTreeReaderValue<float> flashZ(myReaderPMT, "flash_z");
        TTreeReaderValue<std::vector<double>> flashPMTPe(myReaderPMT, "pe_pmt"); //tot pe per pmt
        TTreeReaderValue<std::vector<double>> flashPMTTimeStart(myReaderPMT, "time_pmt"); //first time per pmt
        //TTreeReaderValue<std::vector<double>> flashPMTTimeStart(myReaderPMT, "time_start_pmt"); //first time per pmt
        //TTreeReaderValue<std::vector<double>> flashPMTTimeRise(myReaderPMT, "time_rise_pmt"); //rise time per pmt
        //TTreeReaderValue<std::vector<double>> flashPMTTimePeak(myReaderPMT, "time_peak_pmt"); //peak time per pmt
        TTreeReaderValue<std::vector<double>> flashPMTAmpl(myReaderPMT, "amplitude_pmt"); //max per pmt
        TTreeReaderValue<std::vector<float>> pmtX(myReaderPMT, "pmt_x");
        TTreeReaderValue<std::vector<float>> pmtY(myReaderPMT, "pmt_y");
        TTreeReaderValue<std::vector<float>> pmtZ(myReaderPMT, "pmt_z");

        while( myReaderPMT.Next() ){

            std::vector<OpHit> opHits;

            for( unsigned int pmtIdx=0; pmtIdx<360; pmtIdx++ ){
                    
                // Reduce a little bit the load and save only the PMTs with the correct amount of PEs 
                // also skip the empty channels in the flash (time is 0, sigh) 
                if( ((*flashPMTPe)[pmtIdx] < peCut) || ((*flashPMTTimeStart)[pmtIdx] == 0.) ){ continue; } 


                    OpHit thisOpHit{ 
                        pmtIdx, 
                        // corrections are already negative: so add with +, remove with -
                        // if no add or no removal, the corresponding map is empty
                        (*flashPMTTimeStart)[pmtIdx] + addCorrectionsMap[pmtIdx] - rmCorrectionsMap[pmtIdx],
                        0., //placeholder for rise time
                        0., //placeholder for peak time
                        (*flashPMTPe)[pmtIdx], 
                        (*flashPMTAmpl)[pmtIdx],
                        {(*pmtX)[pmtIdx], (*pmtY)[pmtIdx], (*pmtZ)[pmtIdx]} 
                    };

                    opHits.push_back( thisOpHit );

            } //end loop over opHits 


            OpFlash thisFlash{ (*flashID), cryoidx, (*flashTime), (*flashZ), (*flashY), opHits };
            myFlashMap[(*event)].push_back( thisFlash );

        }

    }

    return myFlashMap;

};

// same function, but for old file format (info splitted in different TTree)
// ignore ophits --> pulled at notebook stage
std::map<int, std::vector<OpFlash>>
    readPMTInfo_oldFormat( const std::vector<std::string> & ttreename, TFile *myTFile, const float & peCut){

    std::map<int, std::vector<OpFlash>> myFlashMap; 

    // for each cryostat
    for( size_t cryoidx=0; cryoidx<ttreename.size(); cryoidx++ ){

        TTreeReader myReaderPMT(ttreename[cryoidx].c_str(), myTFile);

        TTreeReaderValue<int> event(myReaderPMT, "event");
        TTreeReaderValue<int> flashID(myReaderPMT, "flash_id");
        TTreeReaderValue<float> flashTime(myReaderPMT, "flash_time");
        TTreeReaderValue<float> flashY(myReaderPMT, "flash_y");
        TTreeReaderValue<float> flashZ(myReaderPMT, "flash_z");

        while( myReaderPMT.Next() ){

            std::vector<OpHit> opHits; //empty

            OpFlash thisFlash{ (*flashID), cryoidx, (*flashTime), (*flashZ), (*flashY), opHits };
            myFlashMap[(*event)].push_back( thisFlash );

        }

    }

    return myFlashMap;

};

//---------------------------------------------------------------------------------------------------------------
//--------------------- CRT STUFF
//----------------------------------------------------------------------------------------------------------------

struct CRTHit
{
    Vector position;
    int region; 
    double pe;
    double time;

};

struct CRTPMTMatch
{
    int event;
    double flash_time_us;
    double flash_pe;
    Vector flash_position;
    int ncrthits;
    std::vector<CRTHit> crthits;

};

std::map<int, std::vector<CRTPMTMatch>>
    readCRTMatchInfo( const std::string ttreename, TFile *myTFile){

    std::map<int, std::vector<CRTPMTMatch>> myCRTMatchMap; 

    TTreeReader myReaderCRT(ttreename.c_str(), myTFile);

    TTreeReaderValue<int> event(myReaderCRT, "event");
    TTreeReaderValue<double> flashTime(myReaderCRT, "opFlash_time_us");
    TTreeReaderValue<std::vector<double>> flashPos(myReaderCRT, "opFlash_pos");
    TTreeReaderValue<double> flashPE(myReaderCRT, "opFlashPE");
    TTreeReaderValue<int> CRTmatches(myReaderCRT, "CRT_matches");
    TTreeReaderArray<int> crthitRegion(myReaderCRT, "CRT_region");
    TTreeReaderArray<std::vector<double>> crthitPos(myReaderCRT, "CRT_pos");
    TTreeReaderArray<double> crthitTime(myReaderCRT, "CRT_Time_us");
    TTreeReaderArray<double> crthitPE(myReaderCRT, "CRT_pes");

    while( myReaderCRT.Next() ){

        std::vector<CRTHit> crtHits;
        for( int crtIdx=0; crtIdx<(*CRTmatches); crtIdx++ ){
   
            CRTHit thisCRTHit{ 
                Vector{crthitPos[crtIdx][0], crthitPos[crtIdx][1], crthitPos[crtIdx][2]},
                crthitRegion[crtIdx],
                crthitPE[crtIdx],
                crthitTime[crtIdx]
            };
            crtHits.push_back( thisCRTHit );

        } //end loop over CRT hits 

        CRTPMTMatch thisCRTMatch{ 
                (*event), 
                (*flashTime), 
                (*flashPE), 
                Vector{(*flashPos)[0], (*flashPos)[1], (*flashPos)[2]}, 
                (*CRTmatches),
                crtHits
            };

        myCRTMatchMap[(*event)].push_back( thisCRTMatch );

    }

return myCRTMatchMap;

};
