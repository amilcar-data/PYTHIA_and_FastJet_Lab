#!/usr/bin/env bash
# Small end-to-end run. Larger studies can use each lesson's make run target.
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/env.sh"
cd "$PYTHIA_LAB_ROOT"
run_dir="$PYTHIA_LAB_ROOT/runs/$(date +%Y%m%d-%H%M%S)"
mkdir -p "$run_dir"
for lesson in setup-check 00-first-collision 01-compare-to-data 02-physics-processes 03-jets; do
    echo "Building and running $lesson ..."
    make -C "lessons/$lesson" run NEVENTS="${NEVENTS:-2000}" > "$run_dir/$lesson.log" 2>&1 || {
        tail -40 "$run_dir/$lesson.log"
        exit 1
    }
done
"$PYTHIA_LAB_PYTHON" lessons/03-jets/plot_jets.py
"$PYTHIA_LAB_PYTHON" lessons/02-physics-processes/plot_processes.py
"$PYTHIA_LAB_PYTHON" lessons/01-compare-to-data/plot_comparison.py
"$PYTHIA_LAB_PYTHON" scripts/validate-analysis.py > "$run_dir/validation.txt"
cp lessons/03-jets/jets_*.dat lessons/03-jets/jets.csv lessons/03-jets/jet_events.csv lessons/03-jets/jet_comparison.png "$run_dir/"
cp lessons/01-compare-to-data/baseline_*.dat "$run_dir/"
cp lessons/01-compare-to-data/simulation_*.dat lessons/01-compare-to-data/comparison.png "$run_dir/"
cp lessons/02-physics-processes/process_*.dat lessons/02-physics-processes/process_comparison.png "$run_dir/"
{
    date --iso-8601=seconds
    g++ --version | head -1
    fastjet-config --version
    grep '^#define PYTHIA_VERSION ' "$PYTHIA8/include/Pythia8/Pythia.h"
    echo "Seed: 42; requested events: ${NEVENTS:-2000} per sample (Lesson 00: 10, setup: 1)"
} > "$run_dir/environment.txt"
echo "Run logs, plots and histograms: $run_dir"
