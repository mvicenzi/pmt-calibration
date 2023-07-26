export run=$1
export stream=$2
export path=$3
export odir="/icarus/app/users/${USER}/pmt-calibration/cosmics-timing/inputs"

#create directory (if not existing)
#mkdir -p $odir

#loop through files and save in list
list="${odir}/run${1}_tracks_${stream}_files.txt"
if test -f "$list"; then
    echo "$list exists. Removing old list"
    rm $list
fi
touch $list

for file in $( find ${path} -print | egrep '\.root$' );
do
	xroot=$( pnfsToXRootD ${file} )
	echo ${xroot} >> $list
done

export nfiles=$( wc -l < $list )
echo "Project has ${nfiles} output files"

echo "ALL DONE!"
