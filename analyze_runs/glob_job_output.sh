export run=$1
export path="/pnfs/icarus/scratch/users/${USER}/pmt-gains/${run}/out/"
export odir="/icarus/data/users/${USER}/pmt-calibration/histograms_splitted/"

#create directory (if not existing)
mkdir -p $odir

#loop through list of output files
ID=0
for file in $( find ${path} -print | egrep '\.root$' );
do
	ifdh cp ${file} ${odir}/pulseDistributionHist_${ID}_run${run}.root
  	((ID++))
done

echo "Project has ${ID} output files"

echo "ALL DONE!"
