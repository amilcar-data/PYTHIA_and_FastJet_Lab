# Lesson 01: compare primary charged particles with ATLAS

Code: [main.cc](../lessons/01-compare-to-data/main.cc).
Selection: [atlas_selection.h](../lessons/01-compare-to-data/atlas_selection.h).

## 1. The question

Does the simulated charged-particle distribution resemble the measured one when
we count the same particles, use the same bins, and include sampling uncertainty?

The data come from [ATLAS, arXiv:1602.01633](https://arxiv.org/html/1602.01633v2),
[HEPData record ins1419652](https://www.hepdata.net/record/ins1419652), Tables 3–5,
version 1. The downloaded CSV files retain the table DOIs and selection metadata.
The measurement is at particle level; detector effects have been corrected.

We generate pp at 13 TeV using `SoftQCD:inelastic`, including diffraction, seed 42.
Particles must be charged, have pT >= 0.5 GeV and |eta| < 2.5. Events require at
least one accepted particle. The current code additionally enforces the lifetime
and decay-history definition below.

## 2. Why final is not the same as primary

The old code used only final-state status, charge, pT and eta. “Final” means a
particle remains at the end of this simulation. It does not mean it was produced
promptly enough to count in the published measurement.

For example, a K0S can decay into two charged pions. The pions are final and charged,
but the measurement excludes this secondary contribution. Counting those daughters does not match this measurement. Increasing the event count cannot repair that definition error.

The correction uses the paper's two lifetime thresholds: accepted charged species
must live longer than 300 ps; decay products of parents living 30 ps or longer
are excluded. Prompt particles and descendants of short-lived parents can count.
The intermediate 30–300 ps charged strange baryons do not count themselves.
This definition is specific to this measurement, not a universal rule for analyses.

| Example | Current selection |
|---|---|
| Direct charged pion, kaon, proton or electron within the kinematic cuts | Keep |
| Pion from a short-lived rho decay | Keep |
| Pion from K0S decay | Exclude |
| Proton from Lambda decay | Exclude |
| Charged strange baryon with lifetime between the two thresholds | Exclude |
| Descendant reached through a secondary decay followed by a short-lived decay | Exclude |

## 3. How the correction works

`AtlasSelection` examines each particle and walks its mother links. It caches
whether each branch contains a secondary decay. It checks **decay edges**, identified
by PYTHIA's decay-product statuses 91–97, so a muon copied during showering is not
mistaken for a daughter of a muon decay. It also follows intermediate copies and
short-lived decays, so an indirect secondary descendant cannot slip through.

PYTHIA's nominal `tau0()` is c*tau in millimetres, not picoseconds and not the
randomly sampled flight distance in that particular event:

```
30 ps  ->  8.99377374 mm
300 ps -> 89.9377374 mm
```

Stable species such as protons and electrons are represented by zero lifetime
and no decay channels; they are accepted as stable rather than rejected as
short-lived. The selection also requires final state, charge and acceptance.

The generator retains its standard decay setup, with long-lived muons, charged
pions and kaons stable. That lets us compare the old and corrected selections
on the **same generated events**. This implementation targets the standard PYTHIA
sample used here. Changes to forced decays, external decayers, exotic stable
species or event-record conventions require revalidation.

Tests cover prompt particles, short-lived parents, K0S/Lambda daughters, indirect
secondary chains, stable species, shower copies and the pT boundary. This is a
measurement-aligned implementation with regression tests; it is not a claim of
independent certification against a full experimental analysis framework.

## 4. Matching experimental bins

A histogram bin is an interval, not just a point. Suppose the experiment reports
a value for 0.5–0.6 GeV, but the simulation groups 0.5–0.75 GeV together. Those
averages need not agree even for identical underlying physics: the momentum
spectrum falls rapidly within that wider interval. Interpolating bin centres
cannot recover the missing information exactly.

`prepare_bins.py` reads the actual LOW/HIGH columns of the published CSV files.
The C++ code fills those intervals directly:

- eta: 50 bins;
- pT: 37 variable-width bins;
- charged multiplicity: 81 variable-width bins.

The plot verifies identical boundaries before dividing. It never interpolates the
simulation to the data points. HEPData's representative x coordinate may differ
from the arithmetic bin centre; that does not change the interval being compared.

For density plots, divide each bin by its own width. A 10 GeV-wide bin can contain
more entries than a 1 GeV-wide bin simply because it covers more space.
Underflow/overflow are counted in the headers instead of silently treated as zero.

## 5. Normalization and simulation uncertainty

The denominator is the number of events that pass **that selection**, not the
number attempted. The old and corrected selections can accept different events.

| Distribution | Event contribution and normalization |
|---|---|
| eta | Number of accepted particles in the bin / (selected events × bin width) |
| pT | Sum of 1/(2*pi*pT) weights / (selected events × bin width × eta range 5) |
| nch | One entry at the event's multiplicity / (selected events × bin width) |

A finite simulation fluctuates. If you repeat it with another seed, the bins will
change. The new fifth column, `mc_stat_error`, estimates the standard error of each
bin's mean over independent selected events.

The code first adds **all contributions within one event**, then accumulates
that event total and its square. For event totals Xe and N selected events:

```
SE(mean) = sqrt([sum(Xe²) - sum(Xe)²/N] / [N(N-1)])
```

For example, totals 2 and 0 in two events give mean 1 and standard error 1. Treating
the two particles as two independent observations would instead give about 0.707,
missing the within-event correlation. Zero-contribution events are included.
At least two selected events are required to produce the estimates.

In well-populated bins, four times as many events usually halves statistical
errors. More events do not reduce model uncertainty, incorrect selections or
detector bias. A zero-content bin has a zero estimated error in this estimator;
it is not evidence that the underlying probability is exactly zero.

The main plot shows corrected PYTHIA, the old inclusive selection, and ATLAS.
The ratio uses the exact same bins. Its error bars show MC statistics; the gray
band around one shows the fractional experimental error. Statistical and systematic
HEPData components are combined in quadrature for display. We do not have a full
experimental or MC bin-to-bin covariance here, so do not extract a precision
chi-squared from these bars. The two simulation curves are correlated because
they share events; their difference needs a paired uncertainty calculation.

## 6. Run it

```bash
source scripts/env.sh
make -C lessons/01-compare-to-data run NEVENTS=100000
"$PYTHIA_LAB_PYTHON" lessons/01-compare-to-data/plot_comparison.py
```

Run from the repository root. The Makefile extracts bin edges from the downloaded
CSV files automatically. If the files are missing, first run:

```bash
bash lessons/01-compare-to-data/data/fetch_hepdata.sh
```

Outputs in the lesson folder:

- `simulation_eta.dat`, `simulation_pt.dat`, `simulation_nch.dat`: corrected selection;
- `baseline_*.dat`: the old inclusive selection on the same events and current bins;
- `comparison.png`: overlay with MC statistical errors and matched-bin ratios.

Every data row contains `low high centre value mc_stat_error`. Headers record
versions/settings, event count and overflow. The log compares selected event and
particle counts and retains PYTHIA diagnostics, including any internal retries.

## 7. What to investigate next

Residual differences after the correction can come from the tune, process model,
remaining implementation details and statistics. The correction does not force
agreement. Before quantitative fits, add full covariance or event bootstrap
replicas, independent selection cross-checks, and controlled tune/scale variations.
Lesson 02 still uses inclusive charged particles for its qualitative process
comparison; its output is not labelled as this exact ATLAS primary selection.
