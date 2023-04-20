export run=$1
export limit=$2
export path="/icarus/data/users/${USER}/pmt-calibration/input"
export list="${path}/files-run${run}.list"

mkdir -p $path

echo "creating file list $list"

if test -f "$list"; then
    echo "$list exists. Removing old list"
    rm $list
fi

echo "Creating new file list"
touch $list

for file in $( samweb list-files "run_number=${run} AND data_tier raw  AND (Data_Stream=offbeambnbminbias OR Data_Stream=offbeamnumiminbias) with limit ${limit}" )
#for file in $( samweb list-files "run_number=${run} AND data_tier raw with limit ${limit}" )
do
	echo $file
	echo $( samweb get-file-access-url --schema=root --location=enstore $file ) >> $list  
done 

export njobs=$( wc -l < $list )
echo "Project has ${njobs} files"

echo "ALL DONE!"
