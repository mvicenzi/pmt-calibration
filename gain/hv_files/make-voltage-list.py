import numpy as np
import sys, os
import sqlite3
import pandas as pd

def readSqlitedb(date, database="/cvmfs/icarus.opensciencegrid.org/products/icarus/icarus_data/v09_79_01/icarus_data/database/ChannelMapICARUS_20230829.db"):

    table = "pmt_placements"
    if int(date) >= 20230823:
        table = "pmt_placements_23aug2023"
    if int(date) >= 20230829:
        table = "pmt_placements_29aug2023"

    print(date, table)

    # Read sqlite query results into a pandas DataFrame
    con = sqlite3.connect(database)
    df = pd.read_sql_query("SELECT * from {}".format(table), con)
    con.close()

    return df

def PMTid_to_channel(date,pmt_ids):
    
    geo = readSqlitedb(date)
    
    if np.isscalar(pmt_ids):
        channel = geo[geo.pmt_id==pmt_ids].channel_id.values[0]
        return channel
    else:
        channels = [ geo[geo.pmt_id==pmt].channel_id.values[0] for pmt in pmt_ids ] 
        return channels
    
############################################################################################################

def load_hv(filename, voltages):

    """
    Makes a dictionary with key the channelId and as value the voltage set
    """
    date = filename.split("_")[1].rstrip(".sub")
    geo = readSqlitedb(date)

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

def main():
	
	args = sys.argv
	if (len(args) != 2):
		sys.exit()

	oldfilename = args[1]
	prefile = oldfilename.split("_")
	newfile = "pmt_voltage_data_" + prefile[1].rstrip(".sub") + ".csv"
	
	nfp = open(newfile, "w")

	voltages = {}
	load_hv(oldfilename,voltages)
	
	nfp.write("channel_id,voltage,status\n")
	for ch, v, in voltages.items():
		on = 1
		if (v < 1):
 			on = 0
		nfp.write("{},{},{}\n".format(ch,v,on))

	nfp.close()

if __name__=="__main__":
	main()
