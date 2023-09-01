export run=$1
export calibdb="/icarus/data/users/${USER}/pmt-calibration/amplitudedb/"
export min=$2
export max=$3

mkdir -p $calibdb

python python/fit-amplitude-histograms.py ${run} ${min} ${max}
