export run=$1
export limit=200
export list="/icarus/data/users/${USER}/pmt-calibration/input/files-run${run}.list"
export histdir="/icarus/data/users/${USER}/pmt-calibrations/histograms_splitted"
export fcl="bkgphotons-calibration_v3.fcl"


# This part creates a files list
echo "creating file list $list"

if test -f "$list"; then
    echo "$list exists. Removing old list"
    rm $list
fi

echo "Creating new file list"
touch $list

for file in $( samweb list-files "run_number=${run} AND data_tier raw  AND (Data_Stream=offbeambnbminbias OR Data_Stream=offbeamnumiminbias) with limit ${limit}" )
#for file in $( samweb list-files "run_number=${run} AND data_tier raw  with limit ${limit}" )
do
	echo $( samweb get-file-access-url --schema=root --location=enstore $file ) >> $list
done 

export njobs=$( wc -l < $list )
echo "Project has ${njobs} files"

ID=0
cat $list | while read line || [[ -n $line ]];
do

name=${histdir}/pulseDistributionHist_${ID}_run${run}.root
if test -f "$name"; then
  ((ID++))
  continue;
fi

echo $line
echo "$ID / ${njobs}" 
lar -c ${fcl} -s $line -T ${name}

((ID++))
done

echo "ALL DONE!"
