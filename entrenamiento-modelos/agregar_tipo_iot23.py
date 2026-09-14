import argparse
import glob
import os

import pandas as pd


def derivar_tipo(label, detallado):
    if str(label).strip().lower() == "benign":
        return "benigno"

    texto = str(detallado).strip().lower()

    if "ddos" in texto:
        return "ddos"
    if "c&c" in texto:
        return "beaconing"
    if "portscan" in texto:
        return "escaneo"
    if "filedownload" in texto:
        return "backdoor"

    return "otro_ataque"


def procesar_archivo(ruta_entrada, ruta_salida):
    df = pd.read_csv(ruta_entrada, low_memory=False)

    if "label" not in df.columns:
        print(f"Saltando {ruta_entrada}, no tiene columna label")
        return 0

    detallado = df["detailed-label"] if "detailed-label" in df.columns else [""] * len(df)

    df["tipo"] = [derivar_tipo(l, d) for l, d in zip(df["label"], detallado)]
    df.to_csv(ruta_salida, index=False)
    return len(df)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", required=True)
    parser.add_argument("--salida", required=True)
    args = parser.parse_args()

    os.makedirs(args.salida, exist_ok=True)

    archivos = sorted(glob.glob(os.path.join(args.entrada, "*.csv")))
    if not archivos:
        print(f"No se encontraron archivos CSV en {args.entrada}")
        return

    total = 0
    for i, ruta in enumerate(archivos, start=1):
        nombre_salida = os.path.join(args.salida, os.path.basename(ruta))
        filas = procesar_archivo(ruta, nombre_salida)
        total += filas
        print(f"[{i}/{len(archivos)}] {os.path.basename(ruta)}: {filas} filas -> {nombre_salida}")

    print(f"\nListo. {total} filas totales procesadas en {args.salida}")


if __name__ == "__main__":
    main()