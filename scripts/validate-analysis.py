#!/usr/bin/env python3
"""Check current outputs, including reconstructing jet errors from individual events."""
import csv
import importlib.util
import re
from pathlib import Path
import numpy as np
ROOT = Path(__file__).resolve().parent.parent

def load(path):
    a = np.loadtxt(path)
    assert a.ndim == 2 and a.shape[1] == 5, path
    assert np.isfinite(a).all() and (a[:, 3:] >= 0).all(), path
    assert (a[:, 1] > a[:, 0]).all(), path
    return a

def main():
    atlas = ROOT / "lessons/01-compare-to-data"
    spec = importlib.util.spec_from_file_location("prepare_bins", atlas / "prepare_bins.py")
    bins = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(bins)
    for obs in ["eta", "pt", "nch"]:
        edges = np.array(bins.extract(atlas / "data" / f"atlas_{obs}.csv"))
        for prefix in ["simulation", "baseline"]:
            path = atlas / f"{prefix}_{obs}.dat"
            a = load(path)
            assert np.allclose(a[:, :2], np.column_stack([edges[:-1],edges[1:]]), atol=1e-9, rtol=0)
            n = int(re.search(r"# events=(\d+)", path.read_text())[1])
            if obs == "nch":
                widths = np.diff(edges)
                p = a[:, 3]*widths
                assert p.sum() <= 1.+1e-8
                expected = np.sqrt(np.maximum(0, p*(1-p))/(n-1))/widths
                assert np.allclose(a[:, 4], expected, atol=2e-10, rtol=2e-7)
    print("ATLAS outputs: published boundaries, finite values and multiplicity errors pass")

    jetdir = ROOT / "lessons/03-jets"
    with (jetdir / "jet_events.csv").open() as f: events = list(csv.DictReader(f))
    with (jetdir / "jets.csv").open() as f: jets = list(csv.DictReader(f))
    for sample in ["qcd", "top"]:
        selected = [e for e in events if e["sample"] == sample]
        positions = {int(e["event"]): i for i,e in enumerate(selected)}
        assert len(positions) == len(selected) >= 2
        grouped = [[] for _ in selected]
        for jet in jets:
            if jet["sample"] != sample: continue
            grouped[positions[int(jet["event"])]].append(jet)
            assert float(jet["pt_GeV"]) > 30 and abs(float(jet["rapidity"])) < 2.5
            assert float(jet["mass_GeV"]) >= 0 and int(jet["constituents"]) > 0
        for event, group in zip(selected, grouped):
            assert len(group) == int(event["njets"])
            pt = [float(j["pt_GeV"]) for j in group]
            assert pt == sorted(pt, reverse=True)
            assert np.isclose(float(event["leading_pt_GeV"]), pt[0] if pt else 0.)
            assert [int(j["rank"]) for j in group] == list(range(len(group)))
        for obs, key in [("pt","pt_GeV"),("mass","mass_GeV"),("multiplicity",None)]:
            a = load(jetdir / f"jets_{sample}_{obs}.dat")
            edges = np.r_[a[:,0],a[-1,1]]
            # Explicitly include events with zero jets in the independent calculation.
            contributions = np.array([np.histogram(
                [float(j[key]) for j in group] if key else [len(group)], bins=edges)[0]
                for group in grouped])
            widths = np.diff(edges)
            expected = contributions.mean(axis=0)/widths
            errors = contributions.std(axis=0, ddof=1)/np.sqrt(len(selected))/widths
            assert np.allclose(a[:,3],expected,rtol=1e-8,atol=1e-10), (sample,obs)
            assert np.allclose(a[:,4],errors,rtol=1e-8,atol=1e-10), (sample,obs)
        print(f"{sample}: {len(selected)} events; CSV counts, cuts, ordering, all spectra and event errors pass")

if __name__ == "__main__": main()
