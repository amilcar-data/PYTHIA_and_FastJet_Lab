#!/usr/bin/env python3
"""Per-event jet spectra and multiplicity; errors from independent events."""
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import re
HERE = Path(__file__).resolve().parent

def main():
    headers = [(HERE / f"jets_{sample}_pt.dat").read_text().splitlines()[0]
               for sample in ("qcd", "top")]
    radii = [re.search(r"anti-kt R=([0-9.]+)", header).group(1) for header in headers]
    if float(radii[0]) != float(radii[1]):
        raise ValueError("Samples use different radii; rerun both before plotting")
    radius = float(radii[0])
    fig, axes = plt.subplots(1, 3, figsize=(15, 4.8), layout="constrained")
    for ax, obs, xlabel, ylabel in zip(axes,
            ["pt", "mass", "multiplicity"],
            [r"Jet $p_T$ [GeV]", "Jet mass [GeV]", "Accepted jets per event"],
            [r"$(1/N_{ev})\,dN_{jet}/dp_T$ [GeV$^{-1}$]",
             r"$(1/N_{ev})\,dN_{jet}/dm$ [GeV$^{-1}$]", "Event probability"]):
        for sample, label, color in [("qcd", "QCD (hard-scatter pT > 200 GeV)", "C0"),
                                     ("top", "Top pairs (all decays)", "C3")]:
            a = np.loadtxt(HERE / f"jets_{sample}_{obs}.dat")
            edges = np.r_[a[:, 0], a[-1, 1]]
            ax.stairs(a[:, 3], edges, label=label, color=color)
            valid = a[:, 3] > 0
            ax.errorbar(a[valid, 2], a[valid, 3], yerr=a[valid, 4],
                        fmt=".", ms=3, color=color, capsize=2)
        ax.set(xlabel=xlabel, ylabel=ylabel)
        ax.grid(alpha=0.2)
        if obs == "multiplicity":
            ax.set_xlim(-0.5, 12.5)
            ax.set_ylim(bottom=0)
        else:
            ax.set_yscale("log")
        if obs == "pt":
            ax.set_xscale("log")
    axes[0].legend(fontsize=8)
    fig.suptitle(f"Particle-level jets at 13 TeV: anti-kt R={radius:g}, pT > 30 GeV, |y| < 2.5\n"
                 "Per generated event; error bars = MC statistical uncertainty; no detector simulation")
    fig.savefig(HERE / "jet_comparison.png", dpi=150)
    plt.close(fig)
    print("Wrote jet_comparison.png")

if __name__ == "__main__":
    main()
