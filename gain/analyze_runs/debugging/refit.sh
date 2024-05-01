export run=$1

for c in 10 42 51 74 79 91 117 214 225 263 267 279 305;
do
last=$(($c+1))
source fit-histograms.sh $run -w 0 -s $c -e $last -qmin 0.1 -qmax 2.0 -ah 1.9 | grep "$c," >> temp_${run}.txt
done

