# Lesson 00: first collision

Run `make -C lessons/00-first-collision run` after sourcing the environment.
The program generates ten non-diffractive pp collisions at 14 TeV with seed 42.

Read [the source](../lessons/00-first-collision/main.cc) alongside the output:
`readString` chooses settings, `init` prepares the generator, and `next` generates
one collision. `event` holds both intermediate history and remaining particles.
The record size is not the number of particles a detector would observe.
Use `isFinal()` when you mean particles remaining at the end of simulation.

Identify the beams, partons and final particles. Change the beam energy to 900 GeV,
predict whether multiplicities will change, rebuild, then compare many events.
Restore the original settings before continuing. Do not combine SoftQCD and HardQCD
indiscriminately: the samples can overlap.
