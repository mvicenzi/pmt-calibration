export run=$1
export limit=$2
export path="/exp/icarus/data/users/${USER}/pmt-calibration/input"
export list="${path}/files-run${run}.list"
export def="${path}/dataset-run${run}.txt"
export log="${path}/prestage-run${run}.txt"

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

echo "Creating new file list"
prestage=0
for file in $( samweb list-files ${COND} )
do
	echo $file
        samwebLocFull=$(samweb locate-file $file | grep "enstore" | head -n 1)
        fileLocPart=${samwebLocFull#enstore:}
        fileLoc=${fileLocPart%(*}
 
	if [ -n "$fileLoc" ]; then
        	status=$(cat "${fileLoc}/.(get)($file)(locality)")
        	checkOnline=$(echo $status | grep "ONLINE")

		if [ -z "$checkOnline" ]; then
			echo "... not on disk -> prestage needed!"
                	((prestage++))
        	fi
    	fi

	echo $( samweb get-file-access-url --schema=root --location=enstore $file | head -n 1) >> $list  
done 

export njobs=$( wc -l < $list )
echo "Project has ${njobs} files"
echo "${prestage} files need prestaging!"

thr=$(echo "$njobs" | awk '{printf "%d", 0.01*$1}')

if ((prestage > thr)); then
	echo "Prestaging files..." 
	echo "This can take a long time, but you can close this terminal & check status on webpage!"
	nohup samweb prestage-dataset --defname=${DEFNAME} --touch > ${log} 2>&1 &
fi

echo "ALL DONE!"
