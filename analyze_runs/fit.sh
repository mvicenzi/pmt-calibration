export run=${1}
export channel=${2}

s="single_fit.C(${run},${channel})"

root -l $s
