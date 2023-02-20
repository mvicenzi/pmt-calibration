import numpy as np
import pandas as pd
import os, re
from datetime import datetime

import subprocess
import sqlite3

from collections import defaultdict

def readSqlitedb(database="/cvmfs/icarus.opensciencegrid.org/products/icarus/icarus_data/v09_62_00/icarus_data/database/ChannelMapICARUS.db", table="pmt_placements"):

    # Read sqlite query results into a pandas DataFrame
    con = sqlite3.connect(database)
    df = pd.read_sql_query("SELECT * from {}".format(table), con)

    con.close()

    return df

def channel_to_PMTid(channels):
    
    geo = readSqlitedb()
    
    if np.isscalar(channels):
        pmt_id = geo[geo.channel_id==channels].pmt_id.values[0]
        return pmt_id
    else:
        pmt_ids = [ geo[geo.channel_id==ch].pmt_id.values[0] for ch in channels ] 
        return pmt_ids
    
def PMTid_to_channel(pmt_ids):
    
    geo = readSqlitedb()
    
    if np.isscalar(pmt_ids):
        channel = geo[geo.pmt_id==pmt_ids].channel_id.values[0]
        return channel
    else:
        channels = [ geo[geo.pmt_id==pmt].channel_id.values[0] for pmt in pmt_ids ] 
        return channels
    
def load_hv(filename, voltages):

    """
    Makes a dictionary with key the channelId and as value the voltage set
    """
    geo = readSqlitedb()

    for line in open(filename, "r"):

        buff = line.split(",")

        if "{icarus" in buff[0]:

            try: 
                pmtid = int(buff[6])
                channel_id = geo[geo.pmt_id==pmtid].channel_id.values[0]
                value = int( buff[7] )
                          
                voltages[channel_id]=value
            
            except ValueError:
                continue 
    return


def updateVal( pmts, df ):

    #NB! Assumes that the pmtid has not changed while updating the channel mapping 
    channels = [ df[df.pmt_id==(360-pp)].channel_id.values[0] for pp in pmts ] 
    
    return channels

def correctMapping( df, geo, timestamps ):

    #Timestamps to change because of mapping problems
    for t in timestamps:
        channels = updateVal( df.loc[t].pmt.values, geo )
        df.loc[t, 'pmt'] = channels

    return df

# Get the timestamp
def getTimestamp(file):
    
    buff=file.split('_')
    timestamp = int(buff[-1].split('.')[0])
    
    return timestamp
    

# Load a single file
def getDataFrame(file, offPMTs, timeseries=True):
   
    df=pd.read_csv(file, sep=',')
    
    # remove list of PMTs that are off
    channel_ids = PMTid_to_channel(offPMTs)
    df= df[~df['pmt'].isin(channel_ids)]
    
    if timeseries:
        df["timestamp"] = getTimestamp(file)
        df.set_index("timestamp", inplace=True)
    
    return df
    

# Load a dataframe dictionary having timestamp as key   
def dataLoaderDict( sourcedir = "../calibrationdb/" ):
    data = { getTimestamp(file) : getDataFrame(sourcedir+file, False) for file in  os.listdir(sourcedir) if "backgroundphotons" in file }
    return data

# Load just the most recent datafile produced
def getMostRecentCalibration( sourcedir = "../calibrationdb/", timeseries=False ):
    
    timestamps = [ getTimestamp(file) for file in  os.listdir(sourcedir) if "backgroundphotons" in file ]
    files = [ sourcedir+file for file in  os.listdir(sourcedir) if "backgroundphotons" in file ]
    
    mostRecentFile = files[ np.argmax(timestamps) ]
     
    return mostRecentFile






############################### DATA IMPORTER FUNCTION ###############################
offPMTs=[1, 111, 143, 166, 192, 230, 238, 254, 222, 302, 309, 340, 353, 290 ]

def dataLoader( sourcedir = "../calibrationdb/", 
                voltage_file="../../hv_files/Sy4527channels_Dec2022_nominal.sub", 
                interval=(1610067905, 1637788392), 
                adders=0.1 ):
        
        
    print("Import data in folder{} for interval ({}:{})".format(sourcedir, interval[0], interval[1]))
    
      
    # Load the data from the fit database
    data = pd.concat([ getDataFrame(sourcedir+file, offPMTs, True) for file in  os.listdir(sourcedir)[:-1] if "backgroundphotons" in file ])
    
          
    # keep data only for the selected interval
    data = data.loc[(data.index>=interval[0]) & (data.index<interval[1])]
    
    # Add the reference voltages
    voltage_map = {}
    load_hv( voltage_file, voltage_map )
    try:
        print("Import voltage info from {}".format( voltage_file)) 
        data["voltage"] = data.pmt.apply( lambda x : voltage_map.get(x) )
    except:
        raise RuntimeError( "INVALID VOLTAGE MAP!" )
                    
    # Do the correction for the adders ( they are only present at selected intervals )
    if adders>0:
        print( "Adder corrections are activated with value {}".format(adders) )
        t_sel=(data.index>=1627584480)
        data.loc[t_sel, 'q'] = data.loc[t_sel, 'q']+data.loc[t_sel, 'q']*adders
          
    # Sort the indeces by time
    data = data.sort_index()
    
    return data
