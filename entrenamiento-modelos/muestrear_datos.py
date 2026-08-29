import argparse
import glob
import os

import numpy as np
import pandas as pd


def muestrear_archivo(ruta_entrada, ruta_salida, fraccion, tamano_bloque, semilla):
    rng = np.random.default_rng(semilla)
    primer_bloque = True
    filas_leidas = 0
    filas_guardadas = 0

    lector = pd.read_csv(ruta_entrada, low_memory=False, chunksize=tamano_bloque)

    for bloque in lector:
        filas_leidas += len(bloque)
        mascara = rng.random(len(bloque)) < fraccion
        bloque_muestra = bloque[mascara]
        filas_guardadas += len(bloque_muestra)

        bloque_muestra.to_csv(
            ruta_salida,
            mode="w" if primer_bloque else "a",
            header=primer_bloque,
            index=False,
        )
        primer_bloque = False

    return filas_leidas, filas_guardadas


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", required=True)
    parser.add_argument("--salida", required=True)
    parser.add_argument("--fraccion", type=float, default=0.01)
    parser.add_argument("--bloque", type=int, default=200_000)
    parser.add_argument("--semilla", type=int, default=42)
    args = parser.parse_args()

    os.makedirs(args.salida, exist_ok=True)

    archivos = sorted(glob.glob(os.path.join(args.entrada, "*.csv")))
    if not archivos:
        print(f"No se encontraron .csv en {args.entrada}")
        return

    total_leidas = 0
    total_guardadas = 0

    for i, ruta in enumerate(archivos, start=1):
        nombre = os.path.basename(ruta)
        ruta_salida = os.path.join(args.salida, nombre)
        leidas, guardadas = muestrear_archivo(ruta, ruta_salida, args.fraccion, args.bloque, args.semilla + i)
        total_leidas += leidas
        total_guardadas += guardadas
        print(f"[{i}/{len(archivos)}] {nombre}: {leidas} filas -> {guardadas} filas muestreadas")

    print(f"\nListo. {total_leidas} filas leidas -> {total_guardadas} filas guardadas en {args.salida}")


if __name__ == "__main__":
    main()