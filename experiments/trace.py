#!/usr/bin/env python3
"""
Grafica la traza de una corrida: el costo de cada solucion aceptada a lo largo
de las evaluaciones aceptadas.

Uso:
    experiments/trace.py <trace.csv> [salida.png]

El archivo de traza se produce con:
    ./build/tsp-analyzer -r -s SEED --trace trace.csv INSTANCE.tsp

Columnas: accepted, cost.
"""

import csv
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def main():
    if len(sys.argv) < 2:
        print("Uso: trace.py <trace.csv> [salida.png]")
        sys.exit(1)

    path = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else path.replace(".csv", ".png")

    accepted, cost = [], []
    with open(path) as f:
        for row in csv.DictReader(f):
            accepted.append(int(row["accepted"]))
            cost.append(float(row["cost"]))

    if not accepted:
        print("Sin datos en", path)
        sys.exit(1)

    final_cost = cost[-1]

    fig, ax = plt.subplots(figsize=(10, 6))

    # Una sola curva
    ax.plot(accepted, cost, linewidth=0.8)

    ax.set_xlabel("Evaluaciones aceptadas")
    ax.set_ylabel("Costo")
    ax.set_title("Threshold Accepting")

    # Texto con el costo final, se calcula del propio archivo, asi que siempre muestra el valor real de
    # esta corrida
    ax.text(0.97, 0.95, f"Costo final: {final_cost:.6f}",
            transform=ax.transAxes, ha="right", va="top", fontsize=12,
            bbox=dict(boxstyle="round", facecolor="white", edgecolor="gray"))

    fig.tight_layout()
    fig.savefig(out, dpi=120)
    print("Grafica guardada en", out)
    print(f"puntos: {len(accepted)}   costo final: {final_cost:.6f}")


if __name__ == "__main__":
    main()