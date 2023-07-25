export run=$1
export limit=$2
export list="/icarus/data/users/${USER}/pmt-calibration/input/files-run${run}.list"
export histdir="/icarus/data/users/${USER}/pmt-calibration/histograms"
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

# This part does the fit
if test -f "${histdir}/pulseDistributionHist_run${run}.root"; then 
	
	echo "file ${histdir}/pulseDistributionHist_run${run}.root already exists!"
	echo "File will be replaced!"
	rm ${histdir}/pulseDistributionHist_run${run}.root
fi 

lar -c ${fcl} -S ${list} -T ${histdir}/pulseDistributionHist_run${run}.root

echo "ALL DONE!"
