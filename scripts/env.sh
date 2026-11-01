#!/usr/bin/env bash
# Source from any directory: source /path/to/pythia-lab/scripts/env.sh
PYTHIA_LAB_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export PYTHIA_LAB_ROOT
export PYTHIA8="${PYTHIA8:-$PYTHIA_LAB_ROOT/.local/pythia}"
export FASTJET="${FASTJET:-$PYTHIA_LAB_ROOT/.local/fastjet}"
export PATH="$PYTHIA8/bin:$FASTJET/bin:$PATH"
export PYTHIA8DATA="$PYTHIA8/share/Pythia8/xmldoc"
export MPLCONFIGDIR="$PYTHIA_LAB_ROOT/.local/matplotlib"
export MPLBACKEND=Agg
# Select a Python installation with numpy and matplotlib.
export PYTHIA_LAB_PYTHON="${PYTHIA_LAB_PYTHON:-/usr/bin/python3}"
