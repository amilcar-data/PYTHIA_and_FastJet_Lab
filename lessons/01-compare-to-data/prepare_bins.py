#!/usr/bin/env python3
"""Extract exact boundaries from the downloaded HEPData tables (standard library only)."""
import csv
import math
from pathlib import Path

HERE = Path(__file__).resolve().parent

def extract(path):
    with path.open(newline="") as stream:
        rows = [r for r in csv.reader(stream) if r and not r[0].startswith("#")]
    header, rows = rows[0], rows[1:]
    if "LOW" not in header[1].upper() or "HIGH" not in header[2].upper():
        raise ValueError(f"Missing bin boundaries in {path}")
    edges = []
    for row in rows:
        lo, hi = map(float, row[1:3])
        if not math.isfinite(lo+hi) or hi <= lo:
            raise ValueError(f"Invalid interval in {path}")
        if edges and not math.isclose(lo, edges[-1], rel_tol=0, abs_tol=1e-9):
            raise ValueError(f"Noncontiguous bins in {path}")
        if not edges:
            edges.append(lo)
        edges.append(hi)
    if len(edges) < 2:
        raise ValueError(f"Empty table in {path}")
    return edges

if __name__ == "__main__":
    # Validate all inputs before replacing any output.
    tables = {name: extract(HERE / "data" / f"atlas_{name}.csv")
              for name in ("eta", "pt", "nch")}
    for name, edges in tables.items():
        (HERE / "data" / f"atlas_{name}.edges").write_text(
            "".join(f"{edge:.12g}\n" for edge in edges))
        print(f"{name}: {len(edges)-1} bins, {edges[0]} to {edges[-1]}")
