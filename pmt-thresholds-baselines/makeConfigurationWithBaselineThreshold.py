import pandas as pd
import sys, os
import sqlite3

#--------------------------------------------------------------
# helper functions to read .csv baseline file

def read_baselines(file="./baseline.csv"):
    
    df = pd.read_csv(file, sep=',')
    
    return df

def get_baseline(df, ch):
    
    baseline = df.loc[df['channel_id']==ch, ['baseline']]
    
    return baseline.values[0][0]

#----------------------------------------------------------------
# helper functions to map board/digitizer channel to channel id

def readSqlitedb(database="/cvmfs/icarus.opensciencegrid.org/products/icarus/icarus_data/v09_62_00/icarus_data/database/ChannelMapICARUS.db", table="pmt_placements"):

    # Read sqlite query results into a pandas DataFrame
    con = sqlite3.connect(database)
    df = pd.read_sql_query("SELECT * from {}".format(table), con)

    con.close()

    return df

def get_channel_id(db, board, digitizer_channel):
    
    ch = db.loc[(db['digitizer_label']==board) & (db['digitizer_ch_number']==digitizer_channel), ['channel_id']]
    # print(board, ",", digitizer_channel, "-->", ch.values[0][0])

    return ch.values[0][0]

#------------------------------------------------------------------
# help function to parse the board name from the file

def get_board_label(filename):
    
    head, tail = os.path.split(filename)
    buf = tail.removesuffix(".fcl").removeprefix("icaruspmt")
    geo = buf[0:2]
    pos = buf[2:-2]
    slot = buf[-2:]
  
    if slot == "01":
        slot = "A"
    elif slot == "02":
        slot = "B"
    elif slot == "03":
        slot = "C"

    board_label = geo.upper() + "-" + pos.upper() + "-" + slot
    
    return board_label

#-------------------------------------------------------------------
# editing the file --> change baselines and thresholds

def editFile( filename, baseline_file, set_threshold ):
    
	readfile = open( filename, 'r' )
	board_label = get_board_label(filename)
	set_baselines = read_baselines(baseline_file)
	map = readSqlitedb()
    
	new_baselines = {}
	new_file_content = ""

	for line in readfile:

		line = line.strip()

		if "BaselineCh" in line:
        
			argline = line.split(".")[-1]
			buff = argline.split(":")		
			digitizer_channel = int(buff[0].split("Ch")[-1]) #these go from 1 to 16
			old_bl = int(buff[1])
			new_bl = 0
            
			if (digitizer_channel != 16): #last digitizer channel has no pmt
                
				channel_id = get_channel_id(map, board_label, digitizer_channel-1) #in db, expected 0 to 15
				new_bl = get_baseline(set_baselines,channel_id)

			else:
				new_bl = 14500

			new_baselines[digitizer_channel] = new_bl  #save for later threshold computation
            
            # replace old baseline with new baseline
			# print( new_bl)
			line = line.replace( ("BaselineCh%d: %d" % (digitizer_channel, old_bl) ), ("BaselineCh%d: %d" % (digitizer_channel, new_bl) ))
            
		if "triggerThreshold" in line:
			                                
			argline = line.split(".")[-1]
			buff = argline.split(":")		
			digitizer_channel = int(buff[0].split("triggerThreshold")[-1]) #these go from 0 to 15
			old_threshold = int(buff[1])
			new_threshold = 0
            
			if (digitizer_channel != 15): #last digitizer channel has no pmt   
                
				baseline = new_baselines[digitizer_channel+1] #need to convert since baselines are stored 1 to 16
				new_threshold = baseline - set_threshold
                
			else: 
				new_threshold = 100  #make sure it never triggers --> huge threshold
                                
			line = line.replace( ("triggerThreshold%d: %d" % (digitizer_channel, old_threshold) ), ("triggerThreshold%d: %d" % (digitizer_channel, new_threshold) )  )
            
		new_file_content += line+"\n"
                                
	readfile.close()

	writing_file = open(filename, "w")

	writing_file.write(new_file_content)

	writing_file.close()

	return


foldername = sys.argv[1] 
set_threshold = int(sys.argv[2])
baselines = sys.argv[3]

pmt_components = [ filename for filename in os.listdir(foldername)  if "icaruspmt" in filename]

for pmt_component in pmt_components:
	
	editFile( foldername+pmt_component, baselines, set_threshold )

# Change name to the folder
newfoldername = foldername.replace( "thr380", ("thr%d"%set_threshold) )

print("Creating a configuration {} with threshold {}".format(newfoldername, set_threshold))

os.system( "mv %s %s" % (foldername, newfoldername) )

# Publish command
#confname = newfoldername.split("/")[1]
#print("cd workdir ; conftool.py importConfiguration %s- " % confname )
