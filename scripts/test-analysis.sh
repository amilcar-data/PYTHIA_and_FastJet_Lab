#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/env.sh"
cd "$PYTHIA_LAB_ROOT"
mkdir -p .local/tests
# Config tools intentionally return space-separated compiler/linker flags.
g++ -std=c++11 -O2 -Wall -Wextra tests/analysis_checks.cc -o .local/tests/analysis_checks \
    $(pythia8-config --cxxflags) $(fastjet-config --cxxflags) \
    $(pythia8-config --libs) $(fastjet-config --libs)
.local/tests/analysis_checks
