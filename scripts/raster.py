#!/usr/bin/env python3
"""Render a spike raster plot from a CSV log.

Expected CSV columns:
- time_ms
- neuron_id
- step (optional for plotting)
"""

from __future__ import annotations

import argparse
import csv
import os
from pathlib import Path

# Keep matplotlib cache local to the repo so plotting works in restricted environments.
os.environ.setdefault("MPLCONFIGDIR", str((Path.cwd() / ".mpl-cache").resolve()))

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a spike raster plot from spikes.csv")
    parser.add_argument("--input", required=True, help="Path to spikes CSV file")
    parser.add_argument("--output", required=True, help="Path to output image (e.g. data/raster.png)")
    parser.add_argument("--title", default="Spike Raster Plot", help="Plot title")
    parser.add_argument("--marker-size", type=float, default=1.0, help="Scatter marker size")
    parser.add_argument("--alpha", type=float, default=0.8, help="Point opacity")
    parser.add_argument("--dpi", type=int, default=160, help="Output image DPI")
    parser.add_argument("--max-events", type=int, default=0, help="Optional cap on plotted events (0 = all)")
    parser.add_argument("--start-ms", type=float, default=None, help="Optional lower time bound")
    parser.add_argument("--end-ms", type=float, default=None, help="Optional upper time bound")
    return parser.parse_args()


def load_spikes(
    input_path: Path,
    start_ms: float | None,
    end_ms: float | None,
    max_events: int,
) -> tuple[list[float], list[int]]:
    times: list[float] = []
    neuron_ids: list[int] = []

    with input_path.open("r", newline="") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames is None or "time_ms" not in reader.fieldnames or "neuron_id" not in reader.fieldnames:
            raise ValueError("CSV must contain at least: time_ms, neuron_id")

        for row in reader:
            t = float(row["time_ms"])
            if start_ms is not None and t < start_ms:
                continue
            if end_ms is not None and t > end_ms:
                continue

            times.append(t)
            neuron_ids.append(int(row["neuron_id"]))

            if max_events > 0 and len(times) >= max_events:
                break

    return times, neuron_ids


def render_raster(
    times: list[float],
    neuron_ids: list[int],
    output_path: Path,
    title: str,
    marker_size: float,
    alpha: float,
    dpi: int,
) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    fig, ax = plt.subplots(figsize=(12, 6))
    if times:
        ax.scatter(
            times,
            neuron_ids,
            s=marker_size,
            c="#111111",
            alpha=alpha,
            marker=".",
            linewidths=0.0,
            rasterized=True,
        )
    else:
        ax.text(0.5, 0.5, "No spikes in selected window", ha="center", va="center", transform=ax.transAxes)

    ax.set_xlabel("Time (ms)")
    ax.set_ylabel("Neuron ID")
    ax.set_title(title)
    ax.grid(alpha=0.15, linewidth=0.5)
    fig.tight_layout()
    fig.savefig(output_path, dpi=dpi)
    plt.close(fig)


def main() -> None:
    args = parse_args()
    input_path = Path(args.input)
    output_path = Path(args.output)

    times, neuron_ids = load_spikes(
        input_path=input_path,
        start_ms=args.start_ms,
        end_ms=args.end_ms,
        max_events=args.max_events,
    )

    render_raster(
        times=times,
        neuron_ids=neuron_ids,
        output_path=output_path,
        title=args.title,
        marker_size=args.marker_size,
        alpha=args.alpha,
        dpi=args.dpi,
    )

    print(f"Loaded events: {len(times)}")
    print(f"Wrote raster: {output_path}")


if __name__ == "__main__":
    main()
