import pandas as pd
import os, sys

def find_and_replace(file_path, calib_path):
	
	ifile = open(file_path, 'r')
	pos = 0	

	for line in ifile:
		
		pos += len(line)
		line = line.rstrip("\n")
		ready_to_replace = False

		if line.isdigit():  
			run = int(line)
			search = True
			corrections = []
			
			while search:
				next_line = next(ifile, None)
				
				if next_line is None or next_line=="\n":
					search = False
					ready_to_replace = True
					break

				next_line = next_line.rstrip("\n")
	
				if next_line.isdigit():
					search = False
					ready_to_replace = True
					ifile.seek(pos)
					break
                	
				corrections.append( next_line )
			#print(run, corrections)
           
		if ready_to_replace:
			#print(run)
			replace_lines( calib_path, run, corrections ) 
			ready_to_replace = False

              
def replace_lines( calib_path, run, replacements):

	csvfile = ""
	for f in os.listdir(calib_path):

		match = "backgroundphotons_run{:.0f}".format(run) 
		if match in f:
			csvfile = calib_path + "/" + f

	print(csvfile)

	# Read the CSV file into a pandas DataFrame
	df = pd.read_csv(csvfile)
    
	for replacement_line in replacements:

        	# Find the rows where the key column matches the key value
		channel = int(replacement_line.split(",")[0])
		row_to_replace = df[df["pmt"] == channel].index
        
		print(channel, row_to_replace)

    		# Replace the lines with the provided replacement values
		df.loc[row_to_replace] = replacement_line.split(",")

	# Write the modified DataFrame back to the CSV file
	df.to_csv(csvfile, index=False)




def main():

	user = os.environ.get("USER")
	args = sys.argv
    
	if (len(args) != 3):
		sys.exit()

	print("Using {} as corrections...\nfor files in {}".format(args[2],args[1]))

	path = args[1]
	fixes = args[2]
	find_and_replace(args[2], path)
      
if __name__ == "__main__":
	main()


