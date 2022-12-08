import numpy as np
import pandas as pd
import subprocess

##########################################################################################################################################################
#
# The remote server to connect to is: icarusdcs@icarus-gateway01.fnal.gov 
# The current HV configuration file lives in: /home/nfs/icarusdcs/test_niccolo/iocs/SY1527/db/ 
# The other HV configuration files live in: /home/nfs/icarusdcs/test_niccolo/pmt_calibration_files/   
#
# getRemoteFile() brings a copy of the currently configuration file set in the folder $projectdir/hvfiles/ with suffix .current
# copyRemoteFile() is used to upload it to a new HV file to the HV configuration files area. The .current file will be remove from the local area. 
#
# Changing the new HV file has still to be done manually from the Expert PMT control panel
#
###########################################################################################################################################################

def getRemoteFile():
	"""
	Copy the configuration file with the voltages set to the local working directory 
	"""
	
	server = "icarusdcs@icarus-gateway01.fnal.gov"
	serverPath = "/home/nfs/icarusdcs/test_niccolo/iocs/SY1527/db/"
	destiationPath = "../hvfiles/"

	fileNames = ["Sy1527Wch.sub", "Sy1527Ech.sub"]

	for fileName in fileNames:
		destiationFilename = fileName.replace(".sub", ".current")
		subprocess.run(["scp", server+":"+serverPath+fileName, destiationPath+destiationFilename])

	return

def copyRemoteFile( fileName="", oldFilename="" ):

	"""
	Copy the configuration file from the local working directory to the dcs server
	"""

	server = "icarusdcs@icarus-gateway01.fnal.gov"
	serverPath = "/home/nfs/icarusdcs/test_niccolo/pmt_calibration_files/"
	originPath = "../hvfiles/"

	subprocess.run(["scp", originPath+fileName, server+":"+serverPath+fileName, ])
	subprocess.run(["rm", originPath+oldFilename])

	return



def parseHVFile( filenames=["../hvfiles/Sy1527Wch.current", "../hvfiles/Sy1527Ech.current"], copyFromSever=False ):

	"""
	Makes a dictionary with key the channelId and as value the voltage set
	"""

	if copyFromSever:
		getRemoteFile()


	voltages = {}

	for filename in filenames:

		for line in open(filename, "r"):

			buff = line.split(",")

			if buff[0] == "{icarus":

				try: 
					pmtID = int(buff[6])
					setVoltage = int( buff[7] )

					# Usual problem between channelID and pmtID
					channelID = 360-pmtID

					voltages[channelID] = setVoltage
				
				except ValueError:	
					continue 

	return voltages


def writeHVFile( timestring, correctVoltages, oldfilenames=["../hvfiles/Sy1527Wch.current", "../hvfiles/Sy1527Ech.current"], copyToSever=False ):

	for oldfilename in oldfilenames:

		newfilename = oldfilename.replace(".current", "_"+ timestring+".sub")

		nfp = open(newfilename, "w")

		for line in open(oldfilename , "r"):

			buff = line.split(",")

			if buff[0] == "{icarus":

				try: 
					pmtID = int(buff[6])
				except ValueError:	 
					line = ",".join(buff)
					nfp.write(line)
					continue


				value = int(buff[7])
				hwarning = 20
				hcaring = 5

                # Onlu if this pmt needs to be adjusted, the value on file will be changed to the new one 
				SEL_PMT = correctVoltages.pmt == 360-pmtID
				if np.any(SEL_PMT):
					value = int(correctVoltages[SEL_PMT]["New Voltage"].values)

					# Cap values over 2000 V
					if value > 2000:
						value=2000

					#Other sanity checks ??

					print( "Writing the new HV value {} for PMT {}".format(value, pmtID) )
          
				# here we write the new line to file
				newline = buff

				newline[7] = " " + str( value ) #Main nominal value
				newline[9] = " " + str( value+hwarning )
				newline[10] = " " + str( value+hcaring )
				newline[11] = " " + str( value-hcaring )
				newline[12] = " " + str( value-hwarning )+" }\n"

				line =  ",".join(newline)
				nfp.write(line)

			else:
				line = ",".join(buff)
				nfp.write(line)

		print( "Create new file {}".format(newfilename) )
		nfp.close()			

		# Copy to remote server and remove the .current file
		if copyToSever:
			copyRemoteFile( newfilename, oldFilename )			

	return


def main():

	#getRemoteFile()
	voltage = parseHVFile()

if __name__=="__main__":
	main()