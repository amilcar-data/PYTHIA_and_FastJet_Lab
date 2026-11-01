# Learning path

For every lesson: **read → predict → run → inspect → change one setting → explain**.
Do not judge a model from one collision. Keep the seed fixed while learning what a
setting changes, then repeat with different seeds to study statistical variation.

| Step | Run from the repository root | Check before moving on |
|---|---|---|
| Setup | `make -C lessons/setup-check run` | Jets and successful setup message |
| 00 | `make -C lessons/00-first-collision run` | Distinguish intermediate from final particles |
| 01 | `make -C lessons/01-compare-to-data run NEVENTS=10000` | Primary definition, selected-event denominator, errors |
| 02 | `make -C lessons/02-physics-processes run NEVENTS=2000` | Process settings and relative shapes |
| 03 | `make -C lessons/03-jets run NEVENTS=2000` | Jets versus constituents; zero-jet events |

First source `scripts/env.sh`. For plots:

```bash
"$PYTHIA_LAB_PYTHON" lessons/01-compare-to-data/plot_comparison.py
"$PYTHIA_LAB_PYTHON" lessons/02-physics-processes/plot_processes.py
"$PYTHIA_LAB_PYTHON" lessons/03-jets/plot_jets.py
"$PYTHIA_LAB_PYTHON" scripts/validate-analysis.py
```

The NEVENTS argument is attempts per process. Lesson 02 has five processes and
Lesson 03 has two. Failed events are distinct from attempted events.

## A useful study notebook

For each run record: question, predicted effect, software versions, seed, process,
energy, particle definition, cuts, radius, event counts, diagnostics and conclusion.
Keep results locally; share the commands/settings needed to reproduce them.

## Questions to test yourself

1. Why can three quarks produce fewer than three resolved jets?
2. Why can a larger radius increase the number of jets above a threshold?
3. Why do more events not repair a wrong selection?
4. Why should unfolded particle-level data be compared with particle-level simulation?

Answers: sprays can overlap; extra collected momentum can move jets across cuts;
statistics do not correct definitions; smearing again would compare different levels.
