export run=$1
export file="/icarus/data/users/${USER}/pmt-calibration/histograms/pulseDistributionHist_run${run}.root"
export calibdb="/icarus/data/users/${USER}/pmt-calibration/calibrationdb/"

mkdir -p $calibdb

fitPulseDistribution -i $file -d ${calibdb} -v 1 -l 0.3 -h 2.0
