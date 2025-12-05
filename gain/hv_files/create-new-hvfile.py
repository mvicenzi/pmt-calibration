import numpy as np
import sys, os

def writeHVFile( offset, oldfilename, newfilename ):

		nfp = open(newfilename, "w")
		offPMTs = []
	
		for line in open(oldfilename , "r"):

			buff = line.split(",")

			if '{icarus' in buff[0]:

				try: 
					pmtID = int(buff[6])
				except ValueError:	 
					print("Warning: invalid PMT id for {}".format(buff[6]))
					line = ",".join(buff)
					nfp.write(line)
					continue

				oldvalue = int(buff[7])

				value = oldvalue+offset
				hwarning = 20
				hcaring = 5

				# Skip switched-off PMTs
				if oldvalue == 0:
					print( "WARNING: PMT {} is off, stays off!".format(pmtID) )
					line = ",".join(buff)
					nfp.write(line)
					offPMTs.append(pmtID)	
					continue
				
				# Cap values over 2100 V
				if value > 2100:
					print( "WARNING: Capped voltage to 2100V for PMT {}".format(pmtID) )
					value=2100
				else:
					print( "Replacing {}V with {}V for PMT {}".format(oldvalue, value, pmtID) )
          

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

		with open("offPMTs.txt","w") as f:
			for item in offPMTs:
				f.write("%s\n" % item)
		
		return


def main():

	oldfile = sys.argv[1]

	newfile = oldfile.replace("nominal", "p50")
	os.system( "touch {}".format(newfile) )
	print( oldfile, newfile, +50 )
	writeHVFile( +50, oldfile, newfile )

	newfile = oldfile.replace("nominal", "p30")
	os.system( "touch {}".format(newfile) )
	print( oldfile, newfile, +30 )
	writeHVFile( +30, oldfile, newfile )

	newfile = oldfile.replace("nominal", "m30")
	os.system( "touch {}".format(newfile) )
	print( oldfile, newfile, -30 )
	writeHVFile( -30, oldfile, newfile )

	newfile = oldfile.replace("nominal", "p20")
	os.system( "touch {}".format(newfile) )
	print( oldfile, newfile, +20 )
	writeHVFile( +20, oldfile, newfile )
	
	print("ALL DONE!")



if __name__=="__main__":
	main()
