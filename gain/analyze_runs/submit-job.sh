export run=$1
export path="/exp/icarus/data/users/${USER}/pmt-calibration/input/"
export list="${path}/files-run${run}.list"

njobs=$( wc -l < $list )
echo "${run} has ${njobs} files"

source make-job-submission.sh ${run} ${njobs}
