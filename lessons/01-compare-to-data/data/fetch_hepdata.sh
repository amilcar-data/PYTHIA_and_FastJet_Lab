#!/usr/bin/env bash
# Download the ATLAS 13 TeV charged-particle tables from HEPData as CSV.
# Record: https://www.hepdata.net/record/ins1419652  (paper: arXiv:1602.01633)
#
# The script prints each table's own description so YOU can verify that it is
# the selection we simulate: n_ch >= 1, pT > 500 MeV, |eta| < 2.5.
set -euo pipefail
cd "$(dirname "$0")"

BASE="https://www.hepdata.net/record/ins1419652"

# local name -> HEPData table name (see data/README.md for how to double-check)
declare -A TABLE=( [eta]="Table 3" [pt]="Table 4" [nch]="Table 5" )

for key in eta pt nch; do
    echo "=============================================================="
    echo "Downloading ${TABLE[$key]} -> atlas_${key}.csv"
    temp_file=$(mktemp "atlas_${key}.csv.XXXXXX")
    trap 'rm -f "$temp_file"' EXIT
    curl -fsSL --connect-timeout 20 --max-time 120 -G "$BASE" \
         --data-urlencode "version=1" \
         --data-urlencode "format=csv" \
         --data-urlencode "table=${TABLE[$key]}" \
         -o "$temp_file"
    if ! grep -q '^#: table_doi:' "$temp_file"; then
        echo "Download is not a HEPData CSV table; keeping the previous file." >&2
        exit 1
    fi
    mv "$temp_file" "atlas_${key}.csv"
    trap - EXIT
    # Lines starting with '#:' hold the table's name, description and qualifiers.
    grep -E '^#:' "atlas_${key}.csv" | cut -c1-300 || true
done
echo "=============================================================="
echo "Done. Check the descriptions above before trusting the comparison."
