export run=$1

list=""
histdir="/exp/icarus/data/users/${USER}/pmt-calibration/histograms"
out="${histdir}/pulseDistributionHist_run${run}.root" 
dir="/exp/icarus/data/users/${USER}/pmt-calibration/test/splitted/pulseDistributionHist_*_run${run}.root"

for f in $(ls ${dir});
do
	#echo $f
	list+=" ${f}"
done

#echo $list
echo "Merging into ${out}"

mkdir -p $histdir

if test -f "$out"; then
    echo "$out exists. Removing old file"
    rm $out
fi

hadd ${out} ${list}
