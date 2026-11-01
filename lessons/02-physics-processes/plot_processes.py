#!/usr/bin/env python3
"""Overlay the observables of all processes.  (Plotting only: no physics here.)

Inputs  (same folder):  process_eta.dat, process_pt.dat, process_nch.dat
Output:                 process_comparison.png

Run:  python3 plot_processes.py
"""
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

HERE = Path(__file__).parent


def read_columns(path):
    """Read a file written by main.cc.

    The last '#' line holds the column names: x_low x_high x_centre <process names...>
    Returns (edges, {process_name: values}).
    """
    with open(path) as f:
        comment_lines = [line for line in f if line.startswith("#")]
    names = comment_lines[-1].lstrip("# ").split()[3:]       # skip the 3 x columns
    table = np.loadtxt(path, comments="#")
    edges = np.append(table[:, 0], table[-1, 1])             # lower edges + last upper edge
    return edges, {name: table[:, 3 + i] for i, name in enumerate(names)}


def draw(ax, path, xlabel, ylabel, logx=False, logy=False, xlim=None):
    edges, columns = read_columns(path)
    for name, values in columns.items():
        values = np.where(values > 0, values, np.nan)        # empty bins cannot be drawn on a log axis
        ax.stairs(values, edges, label=name, lw=1.5)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    if logx:
        ax.set_xscale("log")
    if logy:
        ax.set_yscale("log")
    else:
        ax.set_ylim(bottom=0)                                # linear plots start at zero
    if xlim:
        ax.set_xlim(*xlim)
    ax.grid(alpha=0.3)


def main():
    fig, axes = plt.subplots(1, 3, figsize=(16, 4.8))
    draw(axes[0], HERE / "process_eta.dat", r"$\eta$",
         r"$(1/N_{ev})\,dN_{ch}/d\eta$")
    draw(axes[1], HERE / "process_pt.dat", r"$p_T$ [GeV]",
         r"$(1/N_{ev})\,(1/2\pi p_T)\,d^2N_{ch}/d\eta\,dp_T$", logx=True, logy=True)
    draw(axes[2], HERE / "process_nch.dat", r"$n_{ch}$",
         r"$(1/N_{ev})\,dN_{ev}/dn_{ch}$", logy=True, xlim=(0, 200))
    axes[0].legend(title="process")
    fig.suptitle(r"pp, $\sqrt{s}$ = 13 TeV, charged particles with $p_T>500$ MeV, $|\eta|<2.5$, $n_{ch}\geq 1$")
    fig.tight_layout()
    fig.savefig(HERE / "process_comparison.png", dpi=150)
    print("Wrote process_comparison.png")


if __name__ == "__main__":
    main()
