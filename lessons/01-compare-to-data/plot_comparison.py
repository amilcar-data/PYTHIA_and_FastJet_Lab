#!/usr/bin/env python3
"""Compare identical published bins; show MC and experimental errors separately."""
import csv
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
HERE = Path(__file__).resolve().parent

def read_hepdata_csv(path):
    with open(path, newline="") as stream:
        rows = [r for r in csv.reader(stream) if r and not r[0].startswith("#")]
    header, rows = rows[0], rows[1:]
    if "LOW" not in header[1].upper() or "HIGH" not in header[2].upper():
        raise ValueError(f"Missing boundaries in {path}")
    a = np.asarray(rows, dtype=float)
    up, down = np.zeros(len(a)), np.zeros(len(a))
    for i, name in enumerate(header[4:], start=4):
        if "+" in name: up += a[:, i]**2
        elif "-" in name: down += a[:, i]**2
    return a[:, :4], np.sqrt(down), np.sqrt(up)

def main():
    fig, axes = plt.subplots(2, 3, figsize=(16, 7), layout="constrained",
                             gridspec_kw={"height_ratios": [3, 1]})
    for col, (obs, xlabel, ylabel) in enumerate([
            ("eta", r"$\eta$", r"$(1/N_{ev})\,dN_{ch}/d\eta$"),
            ("pt", r"$p_T$ [GeV]", r"$(1/N_{ev})(1/2\pi p_T)\,d^2N_{ch}/d\eta dp_T$"),
            ("nch", r"$n_{ch}$", r"$(1/N_{ev})\,dN_{ev}/dn_{ch}$")]):
        sim = np.loadtxt(HERE / f"simulation_{obs}.dat")
        old = np.loadtxt(HERE / f"baseline_{obs}.dat")
        data, dlo, dhi = read_hepdata_csv(HERE / "data" / f"atlas_{obs}.csv")
        if (sim.shape != old.shape or sim.shape != (len(data), 5)
                or not np.allclose(sim[:, :2], data[:, 1:3], atol=1e-9, rtol=0)
                or not np.allclose(old[:, :2], sim[:, :2], atol=1e-9, rtol=0)):
            raise ValueError(f"{obs}: bin mismatch or stale output; rerun make run")
        ax, ratio = axes[:, col]
        edges = np.r_[sim[:, 0], sim[-1, 1]]
        ax.stairs(sim[:, 3], edges, label="PYTHIA: corrected primary", color="C0")
        ax.stairs(old[:, 3], edges, label="Old inclusive selection", color="C1", linestyle="--")
        occupied = sim[:, 3] > 0
        ax.errorbar(sim[occupied, 2], sim[occupied, 3], yerr=sim[occupied, 4],
                    fmt="none", color="C0", capsize=2, label="MC statistical error")
        ax.errorbar(data[:, 0], data[:, 3], yerr=[dlo, dhi], fmt=".", color="k",
                    label="ATLAS (stat + syst)", ms=4)
        valid = data[:, 3] > 0
        x, y = data[valid, 0], data[valid, 3]
        ratio.errorbar(x, sim[valid, 3]/y, yerr=sim[valid, 4]/y,
                       fmt=".", color="C0", ms=4)
        ratio.fill_between(x, 1-dlo[valid]/y, 1+dhi[valid]/y, color="gray", alpha=.3,
                           label="Data fractional error")
        ratio.axhline(1, color="gray", lw=1)
        ratio.set(xlabel=xlabel, ylabel="MC / data", ylim=(0, None))
        ax.set_ylabel(ylabel)
        if obs != "eta": ax.set_yscale("log")
        else: ax.set_ylim(bottom=0)
        if obs == "pt":
            ax.set_xscale("log"); ratio.set_xscale("log")
        ax.tick_params(labelbottom=False)
        ax.grid(alpha=.15); ratio.grid(alpha=.15)
    axes[0, 0].legend(fontsize=8)
    axes[1, 0].legend(fontsize=7)
    fig.suptitle("ATLAS primary charged particles: pp 13 TeV, pT ≥ 0.5 GeV, |η| < 2.5, nch ≥ 1\n"
                 "Published bin boundaries; MC errors from independent events (no model-systematic band)")
    fig.savefig(HERE / "comparison.png", dpi=150)
    plt.close(fig)
    print("Wrote comparison.png")

if __name__ == "__main__":
    main()
