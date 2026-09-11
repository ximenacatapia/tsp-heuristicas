#!/usr/bin/env bash
#
# Runs the heuristic over a range of seeds on one instance, saves the results
# to a CSV, and prints a short summary. Also prints each line to the terminal
# as it goes.
#
# Usage:
#   experiments/run.sh <instance.tsp> [first_seed] [last_seed] [extra flags...]
#
# Example:
#   experiments/run.sh instances/input-150.tsp 1 50 -L 3000 -p 0.99
#
# The CSV is written to experiments/results/<instance>_<first>-<last>.csv
BIN=./build/tsp-analyzer
 
if [ -z "$1" ]; then
    echo "Usage: experiments/run.sh <instance.tsp> [first] [last] [flags...]"
    exit 1
fi
 
instance="$1"
first="${2:-1}"
last="${3:-30}"
 
# Drop the first three positional args; whatever is left are extra flags for
# the binary (e.g. -L 10000 -p 0.99). Guarded so it works even with fewer args.
[ $# -ge 1 ] && shift
[ $# -ge 1 ] && shift
[ $# -ge 1 ] && shift
extra_flags="$@"
 
name=$(basename "$instance" .tsp)
mkdir -p experiments/results
 
# Include the flags in the filename (spaces -> underscores) so different
# configurations do not overwrite each other.
tag=$(echo "$extra_flags" | tr ' ' '_' | tr -d '-')
out="experiments/results/${name}_${first}-${last}${tag:+_$tag}.csv"
 
"$BIN" --csv-header > "$out"
 
echo "Running $name, seeds $first..$last ${extra_flags:+with: $extra_flags}"
echo "------------------------------------------------------------"
 
start=$(date +%s)
for s in $(seq "$first" "$last"); do
    "$BIN" -r -s "$s" $extra_flags --csv "$instance" | tee -a "$out"
done
end=$(date +%s)
 
elapsed=$((end - start))
runs=$((last - first + 1))
 
echo "------------------------------------------------------------"
echo "Saved to: $out"
echo "Runs: $runs   Total time: ${elapsed}s"
 
feasible=$(tail -n +2 "$out" | awk -F, '$9==1' | wc -l | tr -d ' ')
best=$(tail -n +2 "$out" | awk -F, '{print $8}' | sort -g | head -1)
worst=$(tail -n +2 "$out" | awk -F, '{print $8}' | sort -g | tail -1)
avg=$(tail -n +2 "$out" | awk -F, '{sum+=$8; n++} END{printf "%.6f", sum/n}')
 
echo "Feasible: ${feasible}/${runs}"
echo "Cost  ->  best: ${best}   worst: ${worst}   avg: ${avg}"
 
