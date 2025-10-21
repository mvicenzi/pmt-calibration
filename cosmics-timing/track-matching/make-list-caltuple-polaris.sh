export run=$1
export odir="/exp/icarus/data/users/${USER}/pmt-calibration/input_caltuples"

#loop through files and save in list
list="${odir}/files-caltuple-all-run${run}.list"
if test -f "$list"; then
    echo "$list exists. Removing old list"
    rm $list
fi
touch $list

## FOR RUN-4 reprocessing
DEF1="Icaruspro_2025_wcdnn_production_Run4_SBN_v10_06_00_01p05_fstrmOffBeamBNBMAJORITY_calib_ntuples"
#DEF2="Icaruspro_2025_wcdnn_production_Run4_SBN_v10_06_00_01p05_fstrmBNBMAJORITY_calib_ntuples"

files=$( 
  samweb list-definition-files $DEF1 | grep ${run}
)
#  samweb list-definition-files $DEF2 | grep ${run}
#)

prestage=0
errors=0
id=0
for file in $files
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
	#nohup samweb prestage-dataset --defname=${DEFNAME} --touch > ${log} 2>&1 &
fi

echo "ALL DONE!"
