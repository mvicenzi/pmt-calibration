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
        TTreeReaderValue<std::vector<double>> flashPMTTime(myReaderPMT, "time_pmt"); //first time per pmt
        TTreeReaderValue<std::vector<double>> flashPMTAmpl(myReaderPMT, "amplitude_pmt"); //max per pmt
        TTreeReaderValue<std::vector<float>> pmtX(myReaderPMT, "pmt_x");
        TTreeReaderValue<std::vector<float>> pmtY(myReaderPMT, "pmt_y");
        TTreeReaderValue<std::vector<float>> pmtZ(myReaderPMT, "pmt_z");

        while( myReaderPMT.Next() ){

            std::vector<OpHit> opHits;

            for( unsigned int pmtIdx=0; pmtIdx<360; pmtIdx++ ){
                    
                // Reduce a little bit the load and save only the PMTs with the correct amount of PEs 
                // also skip the empty channels in the flash (time is 0, sigh) 
                if( ((*flashPMTPe)[pmtIdx] < peCut) || ((*flashPMTTime)[pmtIdx] == 0.) ){ continue; } 


                    OpHit thisOpHit{ 
                        pmtIdx, 
                        // corrections are already negative: so add with +, remove with -
                        // if no add or no removal, the corresponding map is empty
                        (*flashPMTTime)[pmtIdx] + addCorrectionsMap[pmtIdx] - rmCorrectionsMap[pmtIdx], 
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

struct CRTHit{

    Vector position;
    double time;

};

// from F. Poppi fpoppi@unibo.infn.it
unsigned long long 
    getMode(std::vector<unsigned long long > vector) {

    sort(vector.begin(), vector.end(), greater<int>());

    int modecounter = 0;
    int isnewmodecounter = 0;
    ULong64_t Mode = 0;
    ULong64_t isnewMode = 0;
    bool isFirst = true;
    for (auto i : vector) {
        if (!isFirst) {
            if (i == Mode) modecounter++;
            else if (i!=isnewMode) {
                isnewMode = i;
                isnewmodecounter = 1;
            }
            else if (i == isnewMode) {
                isnewmodecounter++;
                if (isnewmodecounter > modecounter) {
                    Mode = isnewMode;
                    modecounter = isnewmodecounter;
                }
            }
        }
        else {
            isFirst = false;
            Mode = i;
            modecounter++;
        }
    }
    return Mode;
};


typedef struct FEB_delay {
    int HW_mac;
    int SW_mac;
    int SW_modID;
    unsigned long long T0_delay;
    unsigned long long T1_delay;
} FEB_delay;


std::map<int, FEB_delay> loadFEBMap() {

    std::map<int, struct FEB_delay> FEBs;

    struct FEB_delay FEB;
    FEB.HW_mac = 81;
    FEB.SW_mac = 198;
    FEB.SW_modID = 271;
    FEB.T0_delay = 283;
    FEB.T1_delay = 2000309;
    FEBs.insert({ 271, FEB });
    FEB.HW_mac = 119;
    FEB.SW_mac = 197;
    FEB.SW_modID = 270;
    FEB.T0_delay = 298;
    FEB.T1_delay = 2000324;
    FEBs.insert({ 270, FEB });
    FEB.HW_mac = 87;
    FEB.SW_mac = 196;
    FEB.SW_modID = 269;
    FEB.T0_delay = 313;
    FEB.T1_delay = 2000339;
    FEBs.insert({ 269, FEB });
    FEB.HW_mac = 92;
    FEB.SW_mac = 195;
    FEB.SW_modID = 268;
    FEB.T0_delay = 329;
    FEB.T1_delay = 2000355;
    FEBs.insert({ 268, FEB });
    FEB.HW_mac = 180;
    FEB.SW_mac = 194;
    FEB.SW_modID = 267;
    FEB.T0_delay = 344;
    FEB.T1_delay = 2000370;
    FEBs.insert({ 267, FEB });
    FEB.HW_mac = 97;
    FEB.SW_mac = 193;
    FEB.SW_modID = 266;
    FEB.T0_delay = 359;
    FEB.T1_delay = 2000385;
    FEBs.insert({ 266, FEB });
    FEB.HW_mac = 174;
    FEB.SW_mac = 192;
    FEB.SW_modID = 265;
    FEB.T0_delay = 374;
    FEB.T1_delay = 2000400;
    FEBs.insert({ 265, FEB });
    FEB.HW_mac = 238;
    FEB.SW_mac = 178;
    FEB.SW_modID = 251;
    FEB.T0_delay = 390;
    FEB.T1_delay = 2000416;
    FEBs.insert({ 251, FEB });
    FEB.HW_mac = 234;
    FEB.SW_mac = 164;
    FEB.SW_modID = 237;
    FEB.T0_delay = 405;
    FEB.T1_delay = 2000431;
    FEBs.insert({ 237, FEB });
    FEB.HW_mac = 189;
    FEB.SW_mac = 224;
    FEB.SW_modID = 297;
    FEB.T0_delay = 420;
    FEB.T1_delay = 2000446;
    FEBs.insert({ 297, FEB });
    FEB.HW_mac = 190;
    FEB.SW_mac = 223;
    FEB.SW_modID = 296;
    FEB.T0_delay = 436;
    FEB.T1_delay = 2000462;
    FEBs.insert({ 296, FEB });
    FEB.HW_mac = 80;
    FEB.SW_mac = 222;
    FEB.SW_modID = 295;
    FEB.T0_delay = 451;
    FEB.T1_delay = 2000477;
    FEBs.insert({ 295, FEB });
    FEB.HW_mac = 162;
    FEB.SW_mac = 221;
    FEB.SW_modID = 294;
    FEB.T0_delay = 466;
    FEB.T1_delay = 2000492;
    FEBs.insert({ 294, FEB });
    FEB.HW_mac = 64;
    FEB.SW_mac = 220;
    FEB.SW_modID = 293;
    FEB.T0_delay = 482;
    FEB.T1_delay = 2000508;
    FEBs.insert({ 293, FEB });
    FEB.HW_mac = 172;
    FEB.SW_mac = 182;
    FEB.SW_modID = 255;
    FEB.T0_delay = 298;
    FEB.T1_delay = 2000324;
    FEBs.insert({ 255, FEB });
    FEB.HW_mac = 114;
    FEB.SW_mac = 181;
    FEB.SW_modID = 254;
    FEB.T0_delay = 313;
    FEB.T1_delay = 2000339;
    FEBs.insert({ 254, FEB });
    FEB.HW_mac = 100;
    FEB.SW_mac = 180;
    FEB.SW_modID = 253;
    FEB.T0_delay = 328;
    FEB.T1_delay = 2000355;
    FEBs.insert({ 253, FEB });
    FEB.HW_mac = 150;
    FEB.SW_mac = 179;
    FEB.SW_modID = 252;
    FEB.T0_delay = 344;
    FEB.T1_delay = 2000370;
    FEBs.insert({ 252, FEB });
    FEB.HW_mac = 176;
    FEB.SW_mac = 165;
    FEB.SW_modID = 238;
    FEB.T0_delay = 359;
    FEB.T1_delay = 2000385;
    FEBs.insert({ 238, FEB });
    FEB.HW_mac = 67;
    FEB.SW_mac = 151;
    FEB.SW_modID = 224;
    FEB.T0_delay = 374;
    FEB.T1_delay = 2000400;
    FEBs.insert({ 224, FEB });
    FEB.HW_mac = 138;
    FEB.SW_mac = 150;
    FEB.SW_modID = 223;
    FEB.T0_delay = 390;
    FEB.T1_delay = 2000416;
    FEBs.insert({ 223, FEB });
    FEB.HW_mac = 170;
    FEB.SW_mac = 136;
    FEB.SW_modID = 209;
    FEB.T0_delay = 405;
    FEB.T1_delay = 2000431;
    FEBs.insert({ 209, FEB });
    FEB.HW_mac = 101;
    FEB.SW_mac = 122;
    FEB.SW_modID = 195;
    FEB.T0_delay = 420;
    FEB.T1_delay = 2000446;
    FEBs.insert({ 195, FEB });
    FEB.HW_mac = 142;
    FEB.SW_mac = 108;
    FEB.SW_modID = 181;
    FEB.T0_delay = 435;
    FEB.T1_delay = 2000462;
    FEBs.insert({ 181, FEB });
    FEB.HW_mac = 139;
    FEB.SW_mac = 206;
    FEB.SW_modID = 279;
    FEB.T0_delay = 451;
    FEB.T1_delay = 2000477;
    FEBs.insert({ 279, FEB });
    FEB.HW_mac = 185;
    FEB.SW_mac = 207;
    FEB.SW_modID = 280;
    FEB.T0_delay = 466;
    FEB.T1_delay = 2000492;
    FEBs.insert({ 280, FEB });
    FEB.HW_mac = 6;
    FEB.SW_mac = 109;
    FEB.SW_modID = 182;
    FEB.T0_delay = 481;
    // TEMPORARY
//  FEB.T1_delay = 2000447;
    FEB.T1_delay = 2000508;
    FEBs.insert({ 182, FEB });
    FEB.HW_mac = 177;
    FEB.SW_mac = 123;
    FEB.SW_modID = 196;
    FEB.T0_delay = 497;
    FEB.T1_delay = 2000523;
    FEBs.insert({ 196, FEB });
    FEB.HW_mac = 61;
    FEB.SW_mac = 137;
    FEB.SW_modID = 210;
    FEB.T0_delay = 512;
    FEB.T1_delay = 2000538;
    FEBs.insert({ 210, FEB });
    FEB.HW_mac = 125;
    FEB.SW_mac = 183;
    FEB.SW_modID = 256;
    FEB.T0_delay = 298;
    FEB.T1_delay = 2000325;
    FEBs.insert({ 256, FEB });
    FEB.HW_mac = 116;
    FEB.SW_mac = 169;
    FEB.SW_modID = 242;
    FEB.T0_delay = 314;
    FEB.T1_delay = 2000340;
    FEBs.insert({ 242, FEB });
    FEB.HW_mac = 104;
    FEB.SW_mac = 168;
    FEB.SW_modID = 241;
    FEB.T0_delay = 329;
    FEB.T1_delay = 2000355;
    FEBs.insert({ 241, FEB });
    FEB.HW_mac = 91;
    FEB.SW_mac = 167;
    FEB.SW_modID = 240;
    FEB.T0_delay = 344;
    FEB.T1_delay = 2000371;
    FEBs.insert({ 240, FEB });
    FEB.HW_mac = 88;
    FEB.SW_mac = 166;
    FEB.SW_modID = 239;
    FEB.T0_delay = 360;
    FEB.T1_delay = 2000386;
    FEBs.insert({ 239, FEB });
    FEB.HW_mac = 120;
    FEB.SW_mac = 152;
    FEB.SW_modID = 225;
    FEB.T0_delay = 375;
    FEB.T1_delay = 2000401;
    FEBs.insert({ 225, FEB });
    FEB.HW_mac = 132;
    FEB.SW_mac = 138;
    FEB.SW_modID = 211;
    FEB.T0_delay = 390;
    FEB.T1_delay = 2000417;
    FEBs.insert({ 211, FEB });
    FEB.HW_mac = 95;
    FEB.SW_mac = 124;
    FEB.SW_modID = 197;
    FEB.T0_delay = 405;
    FEB.T1_delay = 2000432;
    FEBs.insert({ 197, FEB });
    FEB.HW_mac = 232;
    FEB.SW_mac = 110;
    FEB.SW_modID = 183;
    FEB.T0_delay = 421;
    FEB.T1_delay = 2000447;
    FEBs.insert({ 183, FEB });
    FEB.HW_mac = 165;
    FEB.SW_mac = 208;
    FEB.SW_modID = 281;
    FEB.T0_delay = 436;
    FEB.T1_delay = 2000463;
    FEBs.insert({ 281, FEB });
    FEB.HW_mac = 148;
    FEB.SW_mac = 209;
    FEB.SW_modID = 282;
    FEB.T0_delay = 451;
    FEB.T1_delay = 2000478;
    FEBs.insert({ 282, FEB });
    FEB.HW_mac = 237;
    FEB.SW_mac = 111;
    FEB.SW_modID = 184;
    FEB.T0_delay = 467;
    FEB.T1_delay = 2000493;
    FEBs.insert({ 184, FEB });
    FEB.HW_mac = 102;
    FEB.SW_mac = 125;
    FEB.SW_modID = 198;
    FEB.T0_delay = 482;
    FEB.T1_delay = 2000508;
    FEBs.insert({ 198, FEB });
    FEB.HW_mac = 94;
    FEB.SW_mac = 139;
    FEB.SW_modID = 212;
    FEB.T0_delay = 497;
    FEB.T1_delay = 2000524;
    FEBs.insert({ 212, FEB });
    FEB.HW_mac = 130;
    FEB.SW_mac = 153;
    FEB.SW_modID = 226;
    FEB.T0_delay = 513;
    FEB.T1_delay = 2000539;
    FEBs.insert({ 226, FEB });
    FEB.HW_mac = 181;
    FEB.SW_mac = 184;
    FEB.SW_modID = 257;
    FEB.T0_delay = 284;
    FEB.T1_delay = 2000310;
    FEBs.insert({ 257, FEB });
    FEB.HW_mac = 124;
    FEB.SW_mac = 170;
    FEB.SW_modID = 243;
    FEB.T0_delay = 299;
    FEB.T1_delay = 2000325;
    FEBs.insert({ 243, FEB });
    FEB.HW_mac = 152;
    FEB.SW_mac = 156;
    FEB.SW_modID = 229;
    FEB.T0_delay = 314;
    FEB.T1_delay = 2000341;
    FEBs.insert({ 229, FEB });
    FEB.HW_mac = 98;
    FEB.SW_mac = 155;
    FEB.SW_modID = 228;
    FEB.T0_delay = 329;
    FEB.T1_delay = 2000356;
    FEBs.insert({ 228, FEB });
    FEB.HW_mac = 173;
    FEB.SW_mac = 154;
    FEB.SW_modID = 227;
    FEB.T0_delay = 345;
    FEB.T1_delay = 2000371;
    FEBs.insert({ 227, FEB });
    FEB.HW_mac = 169;
    FEB.SW_mac = 140;
    FEB.SW_modID = 213;
    FEB.T0_delay = 360;
    FEB.T1_delay = 2000387;
    FEBs.insert({ 213, FEB });
    FEB.HW_mac = 144;
    FEB.SW_mac = 126;
    FEB.SW_modID = 199;
    FEB.T0_delay = 375;
    FEB.T1_delay = 2000402;
    FEBs.insert({ 199, FEB });
    FEB.HW_mac = 239;
    FEB.SW_mac = 112;
    FEB.SW_modID = 185;
    FEB.T0_delay = 391;
    FEB.T1_delay = 2000417;
    FEBs.insert({ 185, FEB });
    FEB.HW_mac = 147;
    FEB.SW_mac = 210;
    FEB.SW_modID = 283;
    FEB.T0_delay = 406;
    FEB.T1_delay = 2000433;
    FEBs.insert({ 283, FEB });
    FEB.HW_mac = 105;
    FEB.SW_mac = 211;
    FEB.SW_modID = 284;
    FEB.T0_delay = 421;
    FEB.T1_delay = 2000448;
    FEBs.insert({ 284, FEB });
    FEB.HW_mac = 231;
    FEB.SW_mac = 113;
    FEB.SW_modID = 186;
    FEB.T0_delay = 437;
    FEB.T1_delay = 2000463;
    FEBs.insert({ 186, FEB });
    FEB.HW_mac = 117;
    FEB.SW_mac = 127;
    FEB.SW_modID = 200;
    FEB.T0_delay = 452;
    FEB.T1_delay = 2000478;
    FEBs.insert({ 200, FEB });
    FEB.HW_mac = 126;
    FEB.SW_mac = 141;
    FEB.SW_modID = 214;
    FEB.T0_delay = 467;
    FEB.T1_delay = 2000494;
    FEBs.insert({ 214, FEB });
    FEB.HW_mac = 90;
    FEB.SW_mac = 142;
    FEB.SW_modID = 215;
    FEB.T0_delay = 482;
    FEB.T1_delay = 2000509;
    FEBs.insert({ 215, FEB });
    FEB.HW_mac = 183;
    FEB.SW_mac = 128;
    FEB.SW_modID = 201;
    FEB.T0_delay = 498;
    FEB.T1_delay = 2000524;
    FEBs.insert({ 201, FEB });
    FEB.HW_mac = 241;
    FEB.SW_mac = 114;
    FEB.SW_modID = 187;
    FEB.T0_delay = 513;
    FEB.T1_delay = 2000540;
    FEBs.insert({ 187, FEB });
    FEB.HW_mac = 113;
    FEB.SW_mac = 212;
    FEB.SW_modID = 285;
    FEB.T0_delay = 528;
    FEB.T1_delay = 2000555;
    FEBs.insert({ 285, FEB });
    FEB.HW_mac = 233;
    FEB.SW_mac = 185;
    FEB.SW_modID = 258;
    FEB.T0_delay = 283;
    FEB.T1_delay = 2000310;
    FEBs.insert({ 258, FEB });
    FEB.HW_mac = 164;
    FEB.SW_mac = 171;
    FEB.SW_modID = 244;
    FEB.T0_delay = 299;
    FEB.T1_delay = 2000325;
    FEBs.insert({ 244, FEB });
    FEB.HW_mac = 161;
    FEB.SW_mac = 157;
    FEB.SW_modID = 230;
    FEB.T0_delay = 314;
    FEB.T1_delay = 2000341;
    FEBs.insert({ 230, FEB });
    FEB.HW_mac = 203;
    FEB.SW_mac = 158;
    FEB.SW_modID = 231;
    FEB.T0_delay = 329;
    FEB.T1_delay = 2000356;
    FEBs.insert({ 231, FEB });
    FEB.HW_mac = 122;
    FEB.SW_mac = 159;
    FEB.SW_modID = 232;
    FEB.T0_delay = 345;
    FEB.T1_delay = 2000371;
    FEBs.insert({ 232, FEB });
    FEB.HW_mac = 2;
    FEB.SW_mac = 145;
    FEB.SW_modID = 218;
    FEB.T0_delay = 360;
    FEB.T1_delay = 2000387;
    FEBs.insert({ 218, FEB });
    FEB.HW_mac = 112;
    FEB.SW_mac = 131;
    FEB.SW_modID = 204;
    FEB.T0_delay = 375;
    FEB.T1_delay = 2000402;
    FEBs.insert({ 204, FEB });
    FEB.HW_mac = 62;
    FEB.SW_mac = 117;
    FEB.SW_modID = 190;
    FEB.T0_delay = 391;
    FEB.T1_delay = 2000417;
    FEBs.insert({ 190, FEB });
    FEB.HW_mac = 133;
    FEB.SW_mac = 215;
    FEB.SW_modID = 288;
    FEB.T0_delay = 406;
    FEB.T1_delay = 2000432;
    FEBs.insert({ 288, FEB });
    FEB.HW_mac = 168;
    FEB.SW_mac = 214;
    FEB.SW_modID = 287;
    FEB.T0_delay = 421;
    FEB.T1_delay = 2000448;
    FEBs.insert({ 287, FEB });
    FEB.HW_mac = 182;
    FEB.SW_mac = 116;
    FEB.SW_modID = 189;
    FEB.T0_delay = 436;
    FEB.T1_delay = 2000463;
    FEBs.insert({ 189, FEB });
    FEB.HW_mac = 107;
    FEB.SW_mac = 130;
    FEB.SW_modID = 203;
    FEB.T0_delay = 452;
    FEB.T1_delay = 2000478;
    FEBs.insert({ 203, FEB });
    FEB.HW_mac = 252;
    FEB.SW_mac = 144;
    FEB.SW_modID = 217;
    FEB.T0_delay = 467;
    FEB.T1_delay = 2000494;
    FEBs.insert({ 217, FEB });
    FEB.HW_mac = 141;
    FEB.SW_mac = 143;
    FEB.SW_modID = 216;
    FEB.T0_delay = 482;
    FEB.T1_delay = 2000509;
    FEBs.insert({ 216, FEB });
    FEB.HW_mac = 160;
    FEB.SW_mac = 129;
    FEB.SW_modID = 202;
    FEB.T0_delay = 498;
    // TEMPORARY
    //FEB.T1_delay = 2000424;
    FEB.T1_delay = 2000524;
    FEBs.insert({ 202, FEB });
    FEB.HW_mac = 137;
    FEB.SW_mac = 115;
    FEB.SW_modID = 188;
    FEB.T0_delay = 513;
    FEB.T1_delay = 2000540;
    FEBs.insert({ 188, FEB });
    FEB.HW_mac = 179;
    FEB.SW_mac = 213;
    FEB.SW_modID = 286;
    FEB.T0_delay = 528;
    FEB.T1_delay = 2000555;
    FEBs.insert({ 286, FEB });
    FEB.HW_mac = 66;
    FEB.SW_mac = 186;
    FEB.SW_modID = 259;
    FEB.T0_delay = 298;
    FEB.T1_delay = 2000325;
    FEBs.insert({ 259, FEB });
    FEB.HW_mac = 247;
    FEB.SW_mac = 172;
    FEB.SW_modID = 245;
    FEB.T0_delay = 314;
    FEB.T1_delay = 2000340;
    FEBs.insert({ 245, FEB });
    FEB.HW_mac = 198;
    FEB.SW_mac = 173;
    FEB.SW_modID = 246;
    FEB.T0_delay = 329;
    FEB.T1_delay = 2000356;
    FEBs.insert({ 246, FEB });
    FEB.HW_mac = 243;
    FEB.SW_mac = 174;
    FEB.SW_modID = 247;
    FEB.T0_delay = 344;
    FEB.T1_delay = 2000371;
    FEBs.insert({ 247, FEB });
    FEB.HW_mac = 72;
    FEB.SW_mac = 175;
    FEB.SW_modID = 248;
    FEB.T0_delay = 360;
    FEB.T1_delay = 2000386;
    FEBs.insert({ 248, FEB });
    FEB.HW_mac = 250;
    FEB.SW_mac = 161;
    FEB.SW_modID = 234;
    FEB.T0_delay = 375;
    FEB.T1_delay = 2000401;
    FEBs.insert({ 234, FEB });
    FEB.HW_mac = 249;
    FEB.SW_mac = 147;
    FEB.SW_modID = 220;
    FEB.T0_delay = 390;
    FEB.T1_delay = 2000417;
    FEBs.insert({ 220, FEB });
    FEB.HW_mac = 248;
    FEB.SW_mac = 133;
    FEB.SW_modID = 206;
    FEB.T0_delay = 405;
    FEB.T1_delay = 2000432;
    FEBs.insert({ 206, FEB });
    FEB.HW_mac = 60;
    FEB.SW_mac = 119;
    FEB.SW_modID = 192;
    FEB.T0_delay = 421;
    FEB.T1_delay = 2000447;
    FEBs.insert({ 192, FEB });
    FEB.HW_mac = 145;
    FEB.SW_mac = 217;
    FEB.SW_modID = 290;
    FEB.T0_delay = 436;
    FEB.T1_delay = 2000463;
    FEBs.insert({ 290, FEB });
    FEB.HW_mac = 110;
    FEB.SW_mac = 216;
    FEB.SW_modID = 289;
    FEB.T0_delay = 451;
    FEB.T1_delay = 2000478;
    FEBs.insert({ 289, FEB });
    FEB.HW_mac = 59;
    FEB.SW_mac = 118;
    FEB.SW_modID = 191;
    FEB.T0_delay = 467;
    FEB.T1_delay = 2000493;
    FEBs.insert({ 191, FEB });
    FEB.HW_mac = 202;
    FEB.SW_mac = 132;
    FEB.SW_modID = 205;
    FEB.T0_delay = 482;
    FEB.T1_delay = 2000509;
    FEBs.insert({ 205, FEB });
    FEB.HW_mac = 135;
    FEB.SW_mac = 146;
    FEB.SW_modID = 219;
    FEB.T0_delay = 497;
    FEB.T1_delay = 2000524;
    FEBs.insert({ 219, FEB });
    FEB.HW_mac = 246;
    FEB.SW_mac = 160;
    FEB.SW_modID = 233;
    FEB.T0_delay = 513;
    FEB.T1_delay = 2000539;
    FEBs.insert({ 233, FEB });
    FEB.HW_mac = 253;
    FEB.SW_mac = 187;
    FEB.SW_modID = 260;
    FEB.T0_delay = 342;
    FEB.T1_delay = 2000369;
    FEBs.insert({ 260, FEB });
    FEB.HW_mac = 245;
    FEB.SW_mac = 188;
    FEB.SW_modID = 261;
    FEB.T0_delay = 358;
    FEB.T1_delay = 2000384;
    FEBs.insert({ 261, FEB });
    FEB.HW_mac = 65;
    FEB.SW_mac = 189;
    FEB.SW_modID = 262;
    FEB.T0_delay = 373;
    FEB.T1_delay = 2000400;
    FEBs.insert({ 262, FEB });
    FEB.HW_mac = 57;
    FEB.SW_mac = 190;
    FEB.SW_modID = 263;
    FEB.T0_delay = 388;
    FEB.T1_delay = 2000415;
    FEBs.insert({ 263, FEB });
    FEB.HW_mac = 63;
    FEB.SW_mac = 176;
    FEB.SW_modID = 249;
    FEB.T0_delay = 404;
    FEB.T1_delay = 2000430;
    FEBs.insert({ 249, FEB });
    FEB.HW_mac = 251;
    FEB.SW_mac = 177;
    FEB.SW_modID = 250;
    FEB.T0_delay = 419;
    FEB.T1_delay = 2000445;
    FEBs.insert({ 250, FEB });
    FEB.HW_mac = 70;
    FEB.SW_mac = 163;
    FEB.SW_modID = 236;
    FEB.T0_delay = 434;
    FEB.T1_delay = 2000461;
    FEBs.insert({ 236, FEB });
    FEB.HW_mac = 155;
    FEB.SW_mac = 149;
    FEB.SW_modID = 222;
    FEB.T0_delay = 449;
    FEB.T1_delay = 2000476;
    FEBs.insert({ 222, FEB });
    FEB.HW_mac = 154;
    FEB.SW_mac = 135;
    FEB.SW_modID = 208;
    FEB.T0_delay = 465;
    FEB.T1_delay = 2000491;
    FEBs.insert({ 208, FEB });
    FEB.HW_mac = 85;
    FEB.SW_mac = 121;
    FEB.SW_modID = 194;
    FEB.T0_delay = 480;
    FEB.T1_delay = 2000507;
    FEBs.insert({ 194, FEB });
    FEB.HW_mac = 134;
    FEB.SW_mac = 219;
    FEB.SW_modID = 292;
    FEB.T0_delay = 495;
    FEB.T1_delay = 2000522;
    FEBs.insert({ 292, FEB });
    FEB.HW_mac = 129;
    FEB.SW_mac = 218;
    FEB.SW_modID = 291;
    FEB.T0_delay = 511;
    FEB.T1_delay = 2000537;
    FEBs.insert({ 291, FEB });
    FEB.HW_mac = 115;
    FEB.SW_mac = 120;
    FEB.SW_modID = 193;
    FEB.T0_delay = 526;
    FEB.T1_delay = 2000553;
    FEBs.insert({ 193, FEB });
    FEB.HW_mac = 204;
    FEB.SW_mac = 134;
    FEB.SW_modID = 207;
    FEB.T0_delay = 541;
    FEB.T1_delay = 2000568;
    FEBs.insert({ 207, FEB });
    FEB.HW_mac = 244;
    FEB.SW_mac = 148;
    FEB.SW_modID = 221;
    FEB.T0_delay = 557;
    FEB.T1_delay = 2000583;
    FEBs.insert({ 221, FEB });
    FEB.HW_mac = 82;
    FEB.SW_mac = 162;
    FEB.SW_modID = 235;
    FEB.T0_delay = 572;
    FEB.T1_delay = 2000598;
    FEBs.insert({ 235, FEB });
    FEB.HW_mac = 186;
    FEB.SW_mac = 199;
    FEB.SW_modID = 272;
    FEB.T0_delay = 284;
    FEB.T1_delay = 2000310;
    FEBs.insert({ 272, FEB });
    FEB.HW_mac = 83;
    FEB.SW_mac = 200;
    FEB.SW_modID = 273;
    FEB.T0_delay = 299;
    FEB.T1_delay = 2000326;
    FEBs.insert({ 273, FEB });
    FEB.HW_mac = 254;
    FEB.SW_mac = 201;
    FEB.SW_modID = 274;
    FEB.T0_delay = 314;
    FEB.T1_delay = 2000341;
    FEBs.insert({ 274, FEB });
    FEB.HW_mac = 166;
    FEB.SW_mac = 202;
    FEB.SW_modID = 275;
    FEB.T0_delay = 330;
    FEB.T1_delay = 2000356;
    FEBs.insert({ 275, FEB });
    FEB.HW_mac = 178;
    FEB.SW_mac = 203;
    FEB.SW_modID = 276;
    FEB.T0_delay = 345;
    FEB.T1_delay = 2000371;
    FEBs.insert({ 276, FEB });
    FEB.HW_mac = 136;
    FEB.SW_mac = 204;
    FEB.SW_modID = 277;
    FEB.T0_delay = 360;
    FEB.T1_delay = 2000387;
    FEBs.insert({ 277, FEB });
    FEB.HW_mac = 184;
    FEB.SW_mac = 205;
    FEB.SW_modID = 278;
    FEB.T0_delay = 375;
    FEB.T1_delay = 2000402;
    FEBs.insert({ 278, FEB });
    FEB.HW_mac = 187;
    FEB.SW_mac = 191;
    FEB.SW_modID = 264;
    FEB.T0_delay = 391;
    FEB.T1_delay = 2000417;
    FEBs.insert({ 264, FEB });
    FEB.HW_mac = 240;
    FEB.SW_mac = 231;
    FEB.SW_modID = 304;
    FEB.T0_delay = 406;
    FEB.T1_delay = 2000433;
    FEBs.insert({ 304, FEB });
    FEB.HW_mac = 242;
    FEB.SW_mac = 230;
    FEB.SW_modID = 303;
    FEB.T0_delay = 421;
    FEB.T1_delay = 2000448;
    FEBs.insert({ 303, FEB });
    FEB.HW_mac = 188;
    FEB.SW_mac = 229;
    FEB.SW_modID = 302;
    FEB.T0_delay = 437;
    FEB.T1_delay = 2000463;
    FEBs.insert({ 302, FEB });
    FEB.HW_mac = 58;
    FEB.SW_mac = 228;
    FEB.SW_modID = 301;
    FEB.T0_delay = 452;
    FEB.T1_delay = 2000479;
    FEBs.insert({ 301, FEB });
    FEB.HW_mac = 143;
    FEB.SW_mac = 227;
    FEB.SW_modID = 300;
    FEB.T0_delay = 467;
    FEB.T1_delay = 2000494;
    FEBs.insert({ 300, FEB });
    FEB.HW_mac = 235;
    FEB.SW_mac = 226;
    FEB.SW_modID = 299;
    FEB.T0_delay = 483;
    FEB.T1_delay = 2000509;
    FEBs.insert({ 299, FEB });

    return FEBs;
}

std::map<int, std::vector<CRTHit>>
    readCRTInfo( const std::string & ttreename,
                 const std::string & ttreenameDaq,  
                TFile *myTFile 
){


        std::map<int, std::vector<CRTHit>> myCRTMap;
        std::map<int, unsigned long long> GTfromFlag;


        // THe CRT information are divided for now on two ttrees:
        // 1) myReaderCRTDaq will read the infomration related to
        //    the reset flags in order to apply the appropriate 
        //    timing corrections 
        // 2) myReaderCRT will contain information of the CRT 
        //    hits, correspoding to the time of the physical 
        //    activity in the detector
        // for more info mailto: fpoppi@unibo.infn.it
        TTreeReader myReaderCRTDaq(ttreenameDaq.c_str(), myTFile);
        TTreeReaderValue<int> eventCRTDaq (myReaderCRTDaq, "event");
        TTreeReaderValue<unsigned long long> t0CRTDaq (myReaderCRTDaq, "t0");
        TTreeReaderValue<int> subSysDaq (myReaderCRTDaq, "subSys");
        TTreeReaderValue<int> febCRTDaq (myReaderCRTDaq, "mac5");
        TTreeReaderValue<int> flagsCRTDaq (myReaderCRTDaq, "flags");

        TTreeReader myReaderCRT(ttreename.c_str(), myTFile);
        TTreeReaderValue<int> event(myReaderCRT, "event");
        TTreeReaderValue<int> subSys(myReaderCRT, "subSys");
        TTreeReaderValue<float> crtHitX(myReaderCRT, "x");
        TTreeReaderValue<float> crtHitY(myReaderCRT, "y");
        TTreeReaderValue<float> crtHitZ(myReaderCRT, "z");
        TTreeReaderValue<unsigned long long> crtHitT0(myReaderCRT,"t0"); 
        TTreeReaderValue<unsigned long long> triggerTime(myReaderCRT,"trigger_timestamp");

        // Load the corrections per FEB
        std::map<int, FEB_delay> FEBs = loadFEBMap();

        // The loop begins on the CRT Daq TTree 
        int prevEvent=0;
        std::vector<unsigned long long> corrT0;
        myReaderCRTDaq.Restart();
        while (myReaderCRTDaq.Next()) {
            
            //consider only Top CRT
            if ((*subSysDaq)!= 0){ 
                continue; 
            }

            if ((*eventCRTDaq) == prevEvent) {
                // get the T1 reset event from flag 
                if ((*flagsCRTDaq) == 9) { 

                    unsigned long long globalTrigger = (*t0CRTDaq) + FEBs[(*febCRTDaq) + 73].T0_delay-FEBs[(*febCRTDaq) + 73].T1_delay;
                    corrT0.push_back(globalTrigger);
                }
            }
            else {
                
                GTfromFlag.insert({prevEvent, getMode(corrT0)});
                corrT0.clear();
            }
        
            prevEvent = (*eventCRTDaq);

        }

        myReaderCRT.Restart();
        while (myReaderCRT.Next()) {
            
            // Only CRT On top
            if((*subSys) == 0) {

                //Long64_t timeDiff = (*crtHitT0);

                //if( true ){
                    //Long64_t timeDiff = ((*crtHitT0)-(*triggerTime) % 1000000000)  ;
                    Long64_t timeDiff = ((*crtHitT0) - GTfromFlag[(*event)] % 1000000000);
                //}

                // Keep only the drift
                if ((Double_t)timeDiff>=-1500000 && (Double_t)timeDiff<= 1500000 )  {
                    
                    CRTHit thisCRT{
                        {(*crtHitX), (*crtHitY), (*crtHitZ)},
                        ((double)timeDiff/1000)
                    };

                    myCRTMap[(*event)].push_back( thisCRT );
    
                }
            }
        }

        return myCRTMap;

};



double getSimpleDCA( const Vector & crtHitPos, 
    const Vector & trackStart, const Vector & trackEnd ){

    // Use tvector for easy-peasy crossprodcut
    TVector3 start( trackStart.X, trackStart.Y, trackStart.Z );
    TVector3 end( trackEnd.X, trackEnd.Y, trackEnd.Z );
    TVector3 hit( crtHitPos.X, crtHitPos.Y, crtHitPos.Z );

    return ((hit-start).Cross( (hit-end) )).Mag() / (end-start).Mag();

};


// Dumb TOF- project the track on the crt hit exactly
double getTof( const Vector & crtHitPos, const Vector & trackEnd ){

    TVector3 hit( crtHitPos.X, crtHitPos.Y, crtHitPos.Z );
    TVector3 end( trackEnd.X, trackEnd.Y, trackEnd.Z );

    double c=30000; // cm/us

    return (hit-end).Mag() / c ;

}


CRTHit getClosestDCAHit( 
    const std::vector<CRTHit> &  hits,  const Track & track ){

    CRTHit thisHit; 
    double minDCA = 99999999; 
    for( auto const & hit : hits ){
        double thisDCA = 
            getSimpleDCA( hit.position, track.start, track.end );
        if( thisDCA < minDCA){
            minDCA = thisDCA;
            thisHit = hit;
        }
    }

    return thisHit;
}


bool hasCRTHit( const double & flashTime, 
    const std::vector<CRTHit> & crtHits,
    std::vector<CRTHit> & selCRTHits, 
    const double & interval ){

    bool hasACRTHit = false;

    // Consider only TOP CRT and negative TOF
    for( auto const crtHit : crtHits ){
        double tof = crtHit.time - flashTime;
        if( tof < 0 && tof > interval ){
            hasACRTHit=true; /// ok if only one is found! 
        }
        /// I return top CRT Hits in the interval +/- interval 
        if( abs(tof) < abs(interval) ){
            selCRTHits.push_back( crtHit );
        }
    }

    return hasACRTHit;

}   

