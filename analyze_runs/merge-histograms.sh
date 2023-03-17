export run=$1

list=""
out="histograms/pulseDistributionHist_run${run}.root" 
dir="histograms_splitted/*run${run}.root"

for f in $(ls ${dir});
do
	#echo $f
	list+=" ${f}"
done

#echo $list
echo "Merging into ${out}"

if test -f "$out"; then
    echo "$out exists. Removing old file"
    rm $out
fi

hadd ${out} ${list}
