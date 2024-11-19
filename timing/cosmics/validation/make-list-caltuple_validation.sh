export run=$1
export stream=$2
export odir="/icarus/app/users/${USER}/pmt-calibration/cosmics-timing/validation/inputs"

#loop through files and save in list
list="${odir}/run${1}_tracks_${stream}_validation_files.txt"
if test -f "$list"; then
    echo "$list exists. Removing old list"
    rm $list
fi
touch $list

for file in $(samweb list-files run_number = ${run} and file_format = calib_ntuples);
do
	echo $file
        samwebLocFull=$(samweb locate-file $file | grep "enstore")
        fileLocPart=${samwebLocFull#enstore:}
        fileLoc=${fileLocPart%(*}
    	
	if [ -n "$fileLoc" ]; then
        	status=$(cat "${fileLoc}/.(get)($file)(locality)")
        	checkOnline=$(echo $status | grep "ONLINE")

		if [ -z "$checkOnline" ]; then
			echo "... not on disk -> prestage needed!"
        	fi
    	fi
	
	accesspath=$( samweb get-file-access-url --schema=root $file )
	echo $accesspath >> $list  

done 

export nfiles=$( wc -l < $list )
echo "Project has ${nfiles} output files"

echo "ALL DONE!"
