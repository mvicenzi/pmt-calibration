export run=$1
export stream=$2
export odir="/exp/icarus/data/users/${USER}/pmt-calibration/input_caltuples"
export version="v09_78_06"

#loop through files and save in list
list="${odir}/files-caltuple-run${run}.list"
if test -f "$list"; then
    echo "$list exists. Removing old list"
    rm $list
fi
touch $list

## FOR RUN-2 reprocessing
#COND="run_number=${run} AND file_format=calib_ntuples AND (dataset.tag=Icaruspro_2024_Run2_production_Reproc_Run2_v09_89_01_01p03_offbeambnbmajority_calibtuples OR dataset.tag=Icaruspro_2024_Run2_production_Reproc_Run2_v09_89_01_01p03_bnbmajority_calibtuples)"

## FOR RUN-3 keepup processing
#COND="run_number=${run} AND data_stream=offbeambnbmajority AND file_format=calib_ntuples AND dataset.tag=keepup_production_Run3_v09_90_00_calibtuples"

## FOR run11816 processing
#AND dataset.tag=Icaruspro_2024_Run3_11816_offbeambnbmajority_production_Data_Run3_11816_allPMT_v09_90_00_offbeambnbmajority_calibtuples" 

COND="run_number=${run} and file_format=calib_ntuples AND version=${version} with limit 300"

prestage=0
errors=0
id=0
for file in $( samweb list-files ${COND} )
do
	#echo $id $file
        samwebLocFull=$(samweb locate-file $file | head -n 1 )
        fileLocPart=${samwebLocFull#*:}
        fileLoc=${fileLocPart%(*}
    	
	#echo $(samweb locate-file $file)
	#echo $fileLocPart
	#echo $fileLoc

	if [ -n "$fileLoc" ]; then
        	status=$(cat "${fileLoc}/.(get)($file)(locality)" 2>&1)
		errcode=$?

		if [ $errcode -ne 0 ]; then
    			echo "ERROR: $status" >> $errlist
                        ((errors++))
                        ((id++))
			continue
		fi

	       	checkOnline=$(echo $status | grep "ONLINE")
		if [ -z "$checkOnline" ]; then
			echo "... not on disk -> prestage needed!"
                	((prestage++))
        	fi
    	fi

	echo "$id: $( samweb get-file-access-url --schema=root $file | head -n 1 )"
	echo $( samweb get-file-access-url --schema=root $file | head -n 1 ) >> $list  
	((id++))
done 

export njobs=$( wc -l < $list )
echo "Project has ${njobs} files"
echo "${prestage} files need prestaging!"
echo "${errors} files showed errors and were skipped!"

thr=$(echo "$njobs" | awk '{printf "%d", 0.01*$1}')

if ((prestage > thr)); then
	echo "Prestaging files..." 
	echo "This can take a long time, but you can close this terminal & check status on webpage!"
	nohup samweb prestage-dataset --defname=${DEFNAME} --touch > ${log} 2>&1 &
fi

echo "ALL DONE!"
