export run=$1
export stream=$2
export odir="/exp/icarus/app/users/${USER}/pmt-calibration/cosmics-timing/inputs"
export sdir="/pnfs/icarus/scratch/users/${USER}/CNAF_runs/${run}/stage0"

#create directory (if not existing)
mkdir -p ${sdir}

#loop through files and save in list
list="${odir}/run${1}_stage0_${stream}_files.txt"
if test -f "$list"; then
    echo "$list exists. Removing old list"
    rm $list
fi
touch $list

CNAFPATH=srm://storm-fe-archive.cr.cnaf.infn.it:8444/srm/managerv2?SFN=/icarusdata/plain/user/run1-reprocess
stage=stage0_run1_icarus

for dir in $(gfal-ls ${CNAFPATH}/${run}/${stream});
do
	#echo $dir
	for file in $(gfal-ls ${CNAFPATH}/${run}/${stream}/${dir}/${stage} | grep -E "^data_(.*)-stage0.root");
	do
		fullpath=${CNAFPATH}/${run}/${stream}/${dir}/${stage}/${file}
		#echo $fullpath
		gfal-copy $fullpath ${sdir}/${file}
		echo $(pnfsToXRootD ${sdir}/${file}) >> $list
	done
done

export nfiles=$( wc -l < $list )
echo "Project has ${nfiles} output files"

echo "ALL DONE!"
