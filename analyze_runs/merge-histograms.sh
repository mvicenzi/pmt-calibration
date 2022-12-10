export run=$1

list=""
out="histograms/pulseDistributionHist_run${run}.root" 

for n in {0..19};
do
	list+=" histograms_splitted/pulseDistributionHist_${n}_run${run}.root"
done

#echo $list
echo "Merging into ${out}"

if test -f "$out"; then
    echo "$out exists. Removing old file"
    rm $out
fi

hadd ${out} ${list}
