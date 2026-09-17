BIN=./build/tsp-analyzer

instance="$1"
first="$2"
last="$3"
jobs="${4:-0}"

if [ "$jobs" = "0" ] || [ -z "$jobs" ]; then
    jobs=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
fi

shift 4 2>/dev/null || shift $#
extra_flags="$@"

name=$(basename "$instance" .tsp)
mkdir -p experiments/results
tag=$(echo "$extra_flags" | tr ' ' '_' | tr -d '-')
out="experiments/results/${name}_${first}-${last}${tag:+_$tag}_par.csv"

echo "Running $name, seeds $first..$last on $jobs cores ${extra_flags:+with: $extra_flags}"
start=$(date +%s)

"$BIN" --csv-header > "$out"

seq "$first" "$last" | xargs -P "$jobs" -I {} sh -c \
    "$BIN -r -s {} $extra_flags --csv \"$instance\" >> \"$out\""

end=$(date +%s)
elapsed=$((end - start))
runs=$((last - first + 1))

echo "------------------------------------------------------------"
echo "Saved to: $out"
echo "Runs: $runs   Total time: ${elapsed}s   on $jobs cores"

feasible=$(tail -n +2 "$out" | awk -F, '$9==1' | wc -l | tr -d ' ')
best=$(tail -n +2 "$out" | awk -F, '$9==1 {print $8}' | sort -g | head -1)
avg=$(tail -n +2 "$out" | awk -F, '$9==1 {sum+=$8; n++} END{if(n>0)printf "%.6f", sum/n; else print "n/a"}')

echo "Feasible: ${feasible}/${runs}"
echo "Best feasible cost: ${best}   avg feasible: ${avg}"