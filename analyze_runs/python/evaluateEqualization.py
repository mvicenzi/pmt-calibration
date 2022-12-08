import pandas as pd
import numpy as np
from datetime import datetime 

from helpers import getMostRecentCalibration, getDataFrame, getTimestamp
from gaussfit import fitGainsDistribution, gaus, getEqualization, makeplot
from hvfiles import parseHVFile, writeHVFile


def loadPowerLaws(dbfilename="../calibrationdb/gain_5pt_cold.csv"):
	
	laserCalibratiodb = pd.read_csv(dbfilename, sep=',')
	laserCalibratiodb = laserCalibratiodb.set_index("pmt")
	
	return laserCalibratiodb


def _getVoltageCorrection(V, a, expa, k, q, avgq):

	# If one of them has value ==0 it means the power law is not good, so 
	# I don't change the voltage value based on that! 
	if 0.0 in [a, k, expa, avgq]:
		print('Voltage correction warning: division by 0! No correction done')
		return V

	else:

		dg = (avgq-q)/avgq 
		g=1.0
		a *= np.power(10, -expa)

		dV = 1./(a*k)*np.power((g/a), (1-k)/k)*dg

		correctedVoltage = V + dV

	return int(correctedVoltage)



def main():

	powerLaws  = loadPowerLaws()
	voltagesdb = parseHVFile( copyFromSever=True ) #Always copy the most recent version 

	saveimage = False
	
	file  = getMostRecentCalibration()

	print( "Loading data from: "+ file )

	data = getDataFrame(file, False)

	# Lazy join 
	data["Set Voltage"] = [ voltagesdb[pmt] for pmt in data.pmt ]
	data["a"] = [ powerLaws.loc[pmt].a for pmt in data.pmt ]
	data["expa"] = [ powerLaws.loc[pmt].expa for pmt in data.pmt ]
	data["k"] = [ powerLaws.loc[pmt].k for pmt in data.pmt ]

	fitOutput  = fitGainsDistribution( data )
	params = fitOutput[2] ; errors = fitOutput[3]
	equalization, eqerror = getEqualization( params[1], params[2], errors[1], errors[2] )
	
	if saveimage:
    	# Save fit image
		figname = "fit_"+file.split("/")[-1].replace("csv", "pdf")
		print( "Save a picture of the fit to : "+ figname )
		makeplot( getTimestamp(file), figname, *fitOutput )


	print( "\nData fits to a gaussian:") 
	print( " Mean: %.2e +/- %.2e" % (params[1], errors[1]) )
	print( " Sigma: %.2e +/- %.2e" % (params[2], errors[2]) )
	print( " Equalization: %.2f +/- %.3f %%" % (equalization*100, eqerror*100) )


	if (params[1] > 0.6) and (equalization < 15):
		print( "These values are OK!\n" )
	elif params[1] < 0.6:
		print( "Please check the distribution mean!" )
		return
	elif equalization > 15:
		print( "Please check global equalization!" )
		return

	# now we identify the PMTs in data that are below mean-3.5*sigma or over mean+3.5*sigma
	qmin = params[1] - 3.5*params[2]
	qmax = params[1] + 3.5*params[2]

	SEL_QUALITY_BAD = (data.q < qmin) | (data.q > qmax)
	selData = data.loc[SEL_QUALITY_BAD].copy()
	
	allPMTs = len(data)
	badPMTs = len(selData)

	print( "Found %d / %d PMTs that are not equalized: " % ( badPMTs, allPMTs ) )

	print( selData[ ["pmt", "mu", "q", "eq"]] )

	print( "NB: Please also visually inspect the quality of the fit for those PMTs" )

	print( "New suggested voltages: " )

	selData["New Voltage"] = selData.apply(lambda x :  _getVoltageCorrection(x["Set Voltage"], x["a"], x["expa"] ,x["k"], x["q"], params[1])  ,axis=1).values
	selData["dV"] = selData["Set Voltage"] - selData["New Voltage"]
	
	print( selData[ ["pmt","q" ,"Set Voltage", "New Voltage", "dV"]] )	

	# produce the new equalization file
	print("\nSave new HV values to file")

	#Save the day in a human readable format
	date = datetime.fromtimestamp( getTimestamp(file) ).strftime("%m-%d-%Y")
	writeHVFile( date, selData )

	print("\nALL DONE!")


if __name__=="__main__":
	main()