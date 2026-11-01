# Lesson 02: compare processes

Run `make -C lessons/02-physics-processes run NEVENTS=2000`, then run
`"$PYTHIA_LAB_PYTHON" lessons/02-physics-processes/plot_processes.py`.

The program creates a separate generator for minimum bias, Z to muons,
gluon-fusion Higgs, top pairs, and hard QCD (hard-scatter pT above 200 GeV).
All samples use pp at 13 TeV and seed 42. Read each settings group in
[main.cc](../lessons/02-physics-processes/main.cc).

It measures inclusive final charged particles with pT>0.5 GeV and |eta|<2.5,
in events with at least one such particle. This is a qualitative comparison,
not the primary-particle definition of Lesson 01. The sample called jets here
still measures particles; actual jet clustering is introduced in Lesson 03.

Inspect the hard-process record and compare it with the full event. A Z to two
muons event also includes radiation, remnants and additional parton interactions.
The plots are per selected event, not weighted by each process's rarity.
Cross sections are generator estimates dependent on cuts and model approximations.
These plots do not have MC uncertainty bars; use them to learn qualitative shapes.

Exercise: change the hard-QCD threshold and predict the effect on multiplicity
and cross section. Keep all other settings fixed and record the change.
