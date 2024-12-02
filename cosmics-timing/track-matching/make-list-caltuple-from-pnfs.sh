export run=$1
export path=$2
export odir="/exp/icarus/data/users/${USER}/pmt-calibration/input_caltuples"

#create directory (if not existing)
mkdir -p $odir

#loop through files and save in list
list="${odir}/files-caltuple-run${run}.list"
if test -f "$list"; then
    echo "$list exists. Removing old list"
    rm $list
fi
touch $list

id=0

for file in $( find ${path} | egrep '\.root$' );
do
	echo $file
        xroot=$( pnfsToXRootD ${file} )
	echo ${xroot} >> $list
        echo "$id : ${xroot}"
        ((id++))
done

export nfiles=$( wc -l < $list )
echo "Project has ${nfiles} output files"

echo "ALL DONE!"
