import argparse
import glob
import os

import pandas as pd


MAPEO_COLUMNAS = {
    "ts": "ts",
    "src_ip": "id.orig_h",
    "src_port": "id.orig_p",
    "dst_ip": "id.resp_h",
    "dst_port": "id.resp_p",
    "proto": "proto",
    "service": "service",
    "duration": "duration",
    "src_bytes": "orig_bytes",
    "dst_bytes": "resp_bytes",
    "conn_state": "conn_state",
    "missed_bytes": "missed_bytes",
    "src_pkts": "orig_pkts",
    "src_ip_bytes": "orig_ip_bytes",
    "dst_pkts": "resp_pkts",
    "dst_ip_bytes": "resp_ip_bytes",
}


def procesar_archivo(ruta_entrada, ruta_salida):
    df = pd.read_csv(ruta_entrada, low_memory=False)

    columnas_presentes = {origen: destino for origen, destino in MAPEO_COLUMNAS.items() if origen in df.columns}
    df = df.rename(columns=columnas_presentes)

    if "label" in df.columns:
        df["label"] = df["label"].apply(lambda valor: "benign" if str(valor).strip() in ("0", "0.0") else "malicious")
    elif "type" in df.columns:
        df["label"] = df["type"].apply(lambda valor: "benign" if str(valor).strip().lower() == "normal" else "malicious")
    else:
        df["label"] = "malicious"

    columnas_finales = [c for c in MAPEO_COLUMNAS.values() if c in df.columns] + ["label"]
    df = df[columnas_finales]

    df.to_csv(ruta_salida, index=False)
    return len(df)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", required=True)
    parser.add_argument("--salida", required=True)
    args = parser.parse_args()

    os.makedirs(args.salida, exist_ok=True)

    if os.path.isdir(args.entrada):
        archivos = sorted(glob.glob(os.path.join(args.entrada, "*.csv")))
    else:
        archivos = [args.entrada]

    if not archivos:
        print(f"No se encontraron archivos CSV en {args.entrada}")
        return

    print(f"Procesando {len(archivos)} archivo(s) de TON_IoT")

    total_filas = 0
    for i, ruta in enumerate(archivos, start=1):
        nombre_salida = os.path.join(args.salida, f"ton_iot_{os.path.basename(ruta)}")
        filas = procesar_archivo(ruta, nombre_salida)
        total_filas += filas
        print(f"[{i}/{len(archivos)}] {os.path.basename(ruta)}: {filas} filas -> {nombre_salida}")

    print(f"\nListo. {total_filas} filas totales convertidas en {args.salida}")


if __name__ == "__main__":
    main()