#ifdef  UTILS_H
#define UTILS_H

#include "DataStructures.h"

#include "TFile.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <map>

//namespace utils{

    std::map<int, Trigger> readTriggerInfo(const std::string & ttreename, TFile *myTFile);

    std::vector<std::string> makeFilesVector( std::string const & filename );

    std::vector<Track> readTPCInfo( const std::vector<std::string> & ttreename, TFile * myTFile );

    std::map<int, std::vector<Flash>> readPMTInfo( const std::vector<std::string> & ttreename, TFile *myTFile, const float & peCut, std::map<int, double> pmtCorrectionsMap  );

    std::map<int, std::vector<Flash>> readPMTInfo_oldFormat( const std::vector<std::string> & ttreename, TFile *myTFile, const float & peCut); //old format files

    unsigned long long getMode(std::vector<unsigned long long > vector);

    std::map<int, FEB_delay> loadFEBMap();

    std::map<int, std::vector<CRTHit>> readCRTInfo( const std::string & ttreename, const std::string & ttreenameDaq, TFile *myTFile );

    double getSimpleDCA( const Vector & crtHitPos, const Vector & trackStart, const Vector & trackEnd );

    double getTof( const Vector & crtHitPos, const Vector & trackEnd );

    CRTHit getClosestDCAHit( const std::vector<CRTHit> &  hits,  const Track & track );

    bool flashInTime( double const & flashTime, Trigger const & trigger );

    void loadPMTTimeCorrections( std::string const & channelmap ="",
                        std::string const & laserCorrections="",
                        std::string const & muonsCosmicCorrections="", 
                        std::map<int, double> & timeMap );

    bool hasCRTHit( const OpFlash & flash, 
                    const std::vector<CRTHit> & crtHits,
                    std::vector<CRTHit> & selCRTHits, 
                    const double & interval );

//}

#endif // UTILS_H
