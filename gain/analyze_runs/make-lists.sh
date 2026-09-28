LIMIT=800

RUNS=(
14064 
14082
14099
14117,14122,14124
14159
14163
14165
14190,14192,14193
14196
14208,14210,14211
14246
14259
14267,14269,14270
14278,14279,14280
14290,14291,14292
14300,14301,14302
)

REPORT="$(dirname "${BASH_SOURCE[0]}")/summary_$(date '+%Y%m%d_%H%M%S').txt"
printf "%-12s  %-10s  %-10s  %s\n" "RUN" "REQUESTED" "FOUND" "STATUS" | tee "$REPORT"
echo "--------------------------------------------" | tee -a "$REPORT"

for RUN in "${RUNS[@]}"; do
    source make-list-raw.sh "$RUN" "$LIMIT"
    LIST="/exp/icarus/data/users/${USER}/pmt-calibration/input/files-run${RUN}.list"
    FOUND=$(wc -l < "$LIST" 2>/dev/null || echo 0)
    if [[ $FOUND -ge $LIMIT ]]; then
        STATUS="OK"
    else
        STATUS="SHORT (missing $(( LIMIT - FOUND )))"
    fi
    printf "%-12s  %-10s  %-10s  %s\n" "$RUN" "$LIMIT" "$FOUND" "$STATUS" | tee -a "$REPORT"
done

echo "--------------------------------------------" | tee -a "$REPORT"
echo "Summary written to $REPORT"
