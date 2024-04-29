export run=${1}
export channel=${2}
export low=${3}

s="single_fit.C(${run},${channel},${low})"

root -l $s
