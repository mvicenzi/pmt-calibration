export run=$1
export calibdb="/exp/icarus/data/users/${USER}/pmt-calibration/test"

for t in noT T0.5; do

  source merge-histograms.sh ${run} ${t}
  file="/exp/icarus/data/users/${USER}/pmt-calibration/test/pulseDistributionHist_${t}_run${run}.root"
  path="${calibdb}/${t}"

  rm -r $path
  mkdir -p $path

  fname="${path}/tstart0.2_n1"
  mkdir -p $fname
  fitPulseDistribution -i $file -d ${fname} -v 1 -l 0.2 -h 3.0 -c 1 -n 1
  mv plots.pdf $fname
  
  fname="${path}/tstart0.3_n1"
  mkdir -p $fname
  fitPulseDistribution -i $file -d ${fname} -v 1 -l 0.3 -h 3.0 -c 1 -n 1
  mv plots.pdf $fname
  
  fname="${path}/noC_tstart0.2_n1"
  mkdir -p $fname
  fitPulseDistribution -i $file -d ${fname} -v 1 -l 0.2 -h 3.0 -c 0 -n 1
  mv plots.pdf $fname

  fname="${path}/noC_tstart0.3_n1"
  mkdir -p $fname
  fitPulseDistribution -i $file -d ${fname} -v 1 -l 0.3 -h 3.0 -c 0 -n 1
  mv plots.pdf $fname
  
  fname="${path}/tstart0.3_n2"
  mkdir -p $fname
  fitPulseDistribution -i $file -d ${fname} -v 1 -l 0.3 -h 3.0 -c 1 -n 2
  mv plots.pdf $fname
  
  fname="${path}/tstart0.2_n2"
  mkdir -p $fname
  fitPulseDistribution -i $file -d ${fname} -v 1 -l 0.2 -h 3.0 -c 1 -n 2
  mv plots.pdf $fname
  
  fname="${path}/tstart0.2_n2_p1"
  mkdir -p $fname
  fitPulseDistribution -i $file -d ${fname} -v 1 -l 0.2 -h 3.0 -c 1 -n 2 -p 1
  mv plots.pdf $fname
  
  fname="${path}/tstart0_n2_p1"
  mkdir -p $fname
  fitPulseDistribution -i $file -d ${fname} -v 1 -l 0 -h 3.0 -c 1 -n 2 -p 1
  mv plots.pdf $fname
  
  fname="${path}/tstart0_n2"
  mkdir -p $fname
  fitPulseDistribution -i $file -d ${fname} -v 1 -l 0 -h 3.0 -c 1 -n 2
  mv plots.pdf $fname
  
  fname="${path}/tstart0.1_f2_n2"
  mkdir -p $fname
  fitPulseDistribution -i $file -d ${fname} -v 1 -l 0.1 -h 2.0 -c 1 -n 2
  mv plots.pdf $fname

done
