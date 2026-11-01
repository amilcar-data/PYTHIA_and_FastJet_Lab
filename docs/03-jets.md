# Lesson 03: from one collision to jet distributions

Code: [main.cc](../lessons/03-jets/main.cc). Run guide: [installation.md](installation.md).

## 1. What “multi-event” means

The setup check asked: can one collision pass from PYTHIA into FastJet?
An analysis asks a different question: what happens repeatedly across many
independent collisions? One event might contain two accepted jets, the next four,
and another none. We need many events to estimate how often each outcome occurs.

For every successful collision, this lesson:

1. Selects the particle four-momenta to give FastJet.
2. Clusters them with anti-kt, radius R=0.4 and four-vector addition (E-scheme).
3. Keeps jets with pT > 30 GeV and |rapidity| < 2.5.
4. Fills the pT and mass histograms once per accepted jet.
5. Fills the multiplicity histogram once per event, including zero-jet events.
6. Saves the event's bin totals before starting the next collision.

No particles from different collisions are clustered together.

## 2. The three measurements

| Quantity | Meaning | What one event contributes |
|---|---|---|
| Jet transverse momentum pT | Momentum perpendicular to the beam | One entry for each accepted jet |
| Jet invariant mass m | Mass of the combined four-vector of its constituents | One entry for each accepted jet |
| Jet multiplicity Njet | Number of accepted jets in the event | Exactly one entry, including Njet=0 |

For E-scheme recombination, add the constituent energies and momentum components:

```
Ejet = sum(Ei), pxjet = sum(pxi), pyjet = sum(pyi), pzjet = sum(pzi)
pTjet = sqrt(pxjet² + pyjet²)
mjet² = Ejet² - pxjet² - pyjet² - pzjet²
```

We use units with c=1: energies, masses and momenta are quoted in GeV.
The jet mass is NOT the sum of the individual particle masses. Two massless
particles with an opening angle already have a nonzero combined mass. The
regression test demonstrates this with two 40 GeV massless particles.

For example, an event with accepted jet pT values 240, 190 and 45 GeV contributes
three entries to the pT histogram but only one entry at Njet=3 in the multiplicity
histogram. “Constituents” means particles inside one jet; it is not jet multiplicity.
The “leading” jet is the accepted jet with the largest pT.

## 3. Our precise particle and jet definition

- pp at 13 TeV, default tune recorded in the output, seed 42.
- Only final particles marked visible by PYTHIA; neutrinos are excluded.
- Muons are explicitly excluded; electrons and photons are included.
- `ParticleDecays:limitTau0=on`, `tau0Max=10 mm`: longer-lived hadrons remain
  undecayed. This is a particle-level jet convention, distinct from Lesson 01.
- No input pseudorapidity cut. Apply the jet acceptance after clustering, so a
  hard input boundary does not artificially remove constituents near a jet edge.
- Anti-kt R=0.4, E-scheme; accepted jets have pT > 30 GeV, |y| < 2.5.

Rapidity y and pseudorapidity eta are close for very energetic light objects,
but differ for massive jets. FastJet clusters in rapidity–azimuth space. This
lesson records both and explicitly uses y for the acceptance. R controls the
angular grouping scale; changing it changes constituent content and mass.

These settings do not simulate a calorimeter. There is no detector inefficiency,
energy smearing, pile-up, trigger or b-tagging.

## 4. What is being compared

We generate two separate samples:

- **qcd:** `HardQCD:all=on`, hard-scatter pTHat > 200 GeV.
- **top:** gg and qqbar production of top–antitop pairs, with all default decays.

The hard-scatter threshold is NOT a guaranteed reconstructed-jet threshold.
Radiation and acceptance change the final jets. The samples also have different
hard-process selections, so a difference is not solely a “top versus quark” effect.

Top events can produce several jets through the b quarks and hadronic W decays.
Leptonic W decays reduce the jet count. QCD events can also have more than two jets
because of radiation and additional interactions. At R=0.4, a top decay is often
spread across several jets: do not expect every top-event jet to peak at 173 GeV.
These histograms alone are not a top tagger.

## 5. Normalization and errors

The pT and mass panels show jets **per generated event per GeV**. Their integrals
are the mean accepted jet count (less any recorded out-of-range entries), not one.
The multiplicity histogram shows the fraction of events in each Njet bin, and
integrates to one if no events overflow its range.

We count only successfully generated events in these denominators; failed attempts
are reported. We retain successful events with no accepted jet.

For a particular bin, let Xe be the total contribution from event e. With N
independent events, the mean and standard error are:

```
mean = sum(Xe) / N
SE(mean) = sqrt([sum(Xe²) - sum(Xe)²/N] / [N(N-1)])
```

For a density, divide both by the bin width. All particles/jets in the same event
are accumulated BEFORE squaring. Treating them as independent would miss their
correlations. The reusable implementation is `lessons/common/event_histogram.h`.

These are per-bin MC statistical errors. They are not uncertainty from the tune,
PDF, scale choice, hadronization, detector, or cross-section calculation. No full
bin-to-bin covariance matrix is saved yet. Empty bins have estimated error zero
in this estimator, which does not establish that their true rate is zero. Generate
more events or use suitable interval estimates for sparse tails.

## 6. Run and inspect

From the repository root:

```bash
source scripts/env.sh
make -C lessons/03-jets run NEVENTS=5000
"$PYTHIA_LAB_PYTHON" lessons/03-jets/plot_jets.py
```

NEVENTS is attempts **per process**, so 5,000 means 10,000 attempts in total.

| File | Contents |
|---|---|
| `jets.csv` | One row per accepted jet: sample, event, rank, pT, mass, y, eta, constituent count |
| `jet_events.csv` | One row per successful event: jet count and leading-jet pT; 0 denotes no accepted jet |
| `jets_qcd_*.dat`, `jets_top_*.dat` | Bin boundaries, centre, density/probability, MC standard error |
| `jet_comparison.png` | Three-panel plot with statistical errors |

Each histogram header records versions, selection, attempted/failed counts and
underflow/overflow. The terminal summary also records cross sections. Cross
sections are leading-order generator estimates; the plotted distributions are
not weighted by the relative rarity of the processes.

Test the selection/statistics/jet recombination logic with:

```bash
bash scripts/test-analysis.sh
```

## 7. Exercises

1. Raise the accepted jet pT threshold from 30 to 50 GeV. Predict the change in
   multiplicity and zero-jet fraction before running.
2. Increase R from 0.4 to 0.6. Compare jet mass and multiplicity, keeping every
   other setting fixed. This needs fresh simulation output, not just a new plot.
3. Increase the number of events by four. In well-populated bins, the statistical
   errors should be roughly half as large.
4. Restrict top events to a chosen decay channel, then document the new sample.
   Do not mistake a selection change for a generator disagreement.

Reference: [FastJet user manual](https://arxiv.org/abs/1111.6097).

Radius study: set JET_RADIUS=0.6 before running main to change R; default is 0.4.
