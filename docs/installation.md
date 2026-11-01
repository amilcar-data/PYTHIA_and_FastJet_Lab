# Install and run

## Linux Mint / Ubuntu

```bash
sudo apt update
sudo apt install build-essential curl rsync python3-numpy python3-matplotlib
```

Download or clone this repository, open a terminal in its root, then run:

```bash
bash scripts/install-local.sh
source scripts/env.sh
make -C lessons/setup-check run
bash lessons/01-compare-to-data/data/fetch_hepdata.sh
bash scripts/test-analysis.sh
bash scripts/check.sh
```

The installer downloads official source archives, checks recorded SHA-256 hashes,
then builds FastJet followed by PYTHIA under `.local/`. Allow several minutes and
at least about 1 GB of free space for sources, builds and initial outputs. Large
runs require additional space. Use `JOBS=2 bash scripts/install-local.sh` to reduce
build memory demand. Logs are in `.local/logs/`; no sudo is used for library builds.

Source `scripts/env.sh` in each new terminal. It selects local library/data paths,
uses `/usr/bin/python3` for plotting and saves plots without opening a GUI.
Set `PYTHIA_LAB_PYTHON` to another interpreter with numpy and matplotlib if needed.
For an existing library installation, export `PYTHIA8` and `FASTJET` to its prefixes
before sourcing the environment. This route requires compatible C++ libraries.

## Troubleshooting

| Problem | Check |
|---|---|
| Missing C++ header or config helper | Source scripts/env.sh; check installation logs |
| Missing numpy/matplotlib | Install system packages above or choose the correct Python |
| Missing ATLAS CSV or .edges | Run the HEPData download, then make run in Lesson 01 |
| Download is HTML or fails | Do not use it as numbers; retry or obtain Tables 3–5 v1 CSV from the linked HEPData record |
| Library cannot be loaded | Check installation prefix; rebuild after moving the directory |
| No space left | Check both the project filesystem and /tmp |
| Generator warnings | Read the final statistics; successful completion does not imply no internal retries |

Linux with Bash and GNU Make 4.3+ is the documented platform. macOS needs adaptation;
Windows users can use an appropriate Linux environment such as WSL. Other platforms
are not covered by the verification of these instructions.
