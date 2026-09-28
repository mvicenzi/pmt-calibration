RUNS=(
14797
14798,14799
14800,14801
14802
14803
14804,14805
)

DIR="/exp/icarus/data/users/${USER}/pmt-calibration/histograms_splitted"

for RUN in "${RUNS[@]}"; do
    source glob-job-output.sh "$RUN"
    source merge-histograms.sh "$RUN"
    rm ${DIR}/pulseDistributionHist_*_run${RUN}.root
done
