#!/usr/bin/env bash
# Build the pinned C++ dependencies inside this checkout. No sudo required.
set -euo pipefail
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root_dir"
for tool in g++ make curl tar rsync sha256sum; do
    command -v "$tool" >/dev/null || { echo "Missing prerequisite: $tool" >&2; exit 1; }
done
mkdir -p .local/src .local/logs
cd .local/src
if [[ ! -f fastjet-3.5.2.tar.gz ]]; then
    curl -fL --connect-timeout 20 --max-time 300 https://fastjet.fr/repo/fastjet-3.5.2.tar.gz -o fastjet-3.5.2.tar.gz.part
    mv fastjet-3.5.2.tar.gz.part fastjet-3.5.2.tar.gz
fi
if [[ ! -f pythia8318.tgz ]]; then
    curl -fL --connect-timeout 20 --max-time 300 https://pythia.org/releases/pythia83/pythia8318.tgz -o pythia8318.tgz.part
    mv pythia8318.tgz.part pythia8318.tgz
fi
# Hashes of the official archives retrieved for these pinned releases.
sha256sum -c <<'SUMS'
0b8884a394186a39bd27cef0645a65dd0398d695ae20d29e0559294ee2a44de7  fastjet-3.5.2.tar.gz
85dce1e623f91499b2973e3f939bf760b0f745ad4f4eb1bd0fbce2074e2e8f5c  pythia8318.tgz
SUMS
[[ -d fastjet-3.5.2 ]] || tar xzf fastjet-3.5.2.tar.gz
[[ -d pythia8318 ]] || tar xzf pythia8318.tgz
echo "Building FastJet; logs are in .local/logs/ (this takes a few minutes)."
cd fastjet-3.5.2
./configure --prefix="$root_dir/.local/fastjet" > ../../logs/fastjet-configure.log 2>&1
make -j"${JOBS:-4}" > ../../logs/fastjet-build.log 2>&1
make install > ../../logs/fastjet-install.log 2>&1
echo "Building PYTHIA; logs are in .local/logs/."
cd ../pythia8318
./configure --prefix="$root_dir/.local/pythia" --with-fastjet3="$root_dir/.local/fastjet" > ../../logs/pythia-configure.log 2>&1
make -j"${JOBS:-4}" > ../../logs/pythia-build.log 2>&1
make install > ../../logs/pythia-install.log 2>&1
echo "Installed. From the repository root: source scripts/env.sh"
