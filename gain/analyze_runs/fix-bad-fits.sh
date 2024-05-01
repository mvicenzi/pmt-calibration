export calibdb="/exp/icarus/data/users/${USER}/pmt-calibration/calibrationdb/"
export fixes=$(readlink -f $1)

python python/fix-bad-fits.py ${calibdb} ${fixes}
