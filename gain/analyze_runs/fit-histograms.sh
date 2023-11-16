export run=$1
export file="/exp/icarus/data/users/${USER}/pmt-calibration/histograms/pulseDistributionHist_run${run}.root"
export calibdb="/exp/icarus/data/users/${USER}/pmt-calibration/calibrationdb/"

mkdir -p $calibdb

fitPulseDistribution -i $file -d ${calibdb} -v 1 -l 0.3 -h 2.0
