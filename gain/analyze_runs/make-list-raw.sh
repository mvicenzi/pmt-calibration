export run=$1
export limit=$2
export path="/exp/icarus/data/users/${USER}/pmt-calibration/input"
export list="${path}/files-run${run}.list"
export def="${path}/dataset-run${run}.txt"
export log="${path}/prestage-run${run}.txt"
export errlist="file-locality-errors.list"

mkdir -p $path

### FOR STANDARD RUNS
DEFNAME="${USER}_PMTgain_run${run}_offbeam_${limit}"
COND="run_number=${run} AND data_tier=raw AND icarus_project.stage=daq AND (Data_Stream=offbeamminbiascalib OR Data_Stream=offbeambnbminbias OR Data_Stream=offbeamnumiminbias) with limit ${limit}"

### FOR LASER RUNS
#DEFNAME="${USER}_PMTgain_run${run}_laser_${limit}"
#COND="run_number=${run} AND data_tier raw AND data_stream laser AND icarus_project.stage=daq with limit ${limit}"

echo "Creating samweb definition ${DEFNAME}" 

if test -f "$def"; then
    echo "$def exists. Removing old definition"
    samweb delete-definition ${DEFNAME}
    rm $def
fi

echo "Creating new samweb definition"
touch $def
samweb create-definition ${DEFNAME} ${COND}
echo $DEFNAME >> $def

echo "Creating file list ${list}" 
if test -f "$list"; then
    echo "$list exists. Removing old list"
    rm $list
fi
touch $list
touch $errlist

echo "Creating new file list"
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
