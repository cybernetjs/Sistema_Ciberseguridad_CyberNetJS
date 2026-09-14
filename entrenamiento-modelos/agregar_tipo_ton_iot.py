import argparse
import glob
import os

import pandas as pd


MAPEO_TIPO = {
    "normal": "benigno",
    "scanning": "escaneo",
    "dos": "dos",
    "ddos": "ddos",
    "injection": "inyeccion",
    "password": "fuerza_bruta",
    "xss": "xss",
    "backdoor": "backdoor",
    "ransomware": "ransomware",
    "mitm": "mitm",
}


def procesar_par(ruta_crudo, ruta_agregado, ruta_salida):
    crudo = pd.read_csv(ruta_crudo, low_memory=False, usecols=["type"])
    agregado = pd.read_csv(ruta_agregado, low_memory=False)

    if len(crudo) != len(agregado):
        print(f"ADVERTENCIA: {ruta_crudo} tiene {len(crudo)} filas pero {ruta_agregado} tiene {len(agregado)}, se omite")
        return 0

    tipo_crudo = crudo["type"].astype(str).str.strip().str.lower()
    agregado["tipo"] = tipo_crudo.map(MAPEO_TIPO).fillna("otro_ataque").values

    agregado.to_csv(ruta_salida, index=False)
    return len(agregado)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--crudo", required=True)
    parser.add_argument("--agregado", required=True)
    parser.add_argument("--salida", required=True)
    args = parser.parse_args()

    os.makedirs(args.salida, exist_ok=True)

    archivos_agregados = sorted(glob.glob(os.path.join(args.agregado, "*.csv")))
    if not archivos_agregados:
        print(f"No se encontraron archivos CSV en {args.agregado}")
        return

    total = 0
    for i, ruta_agregado in enumerate(archivos_agregados, start=1):
        nombre = os.path.basename(ruta_agregado)
        nombre_crudo = nombre.replace("ton_iot_", "")
        ruta_crudo = os.path.join(args.crudo, nombre_crudo)

        if not os.path.exists(ruta_crudo):
            print(f"[{i}/{len(archivos_agregados)}] {nombre}: no se encontro {ruta_crudo}, se omite")
            continue

        ruta_salida = os.path.join(args.salida, nombre)
        filas = procesar_par(ruta_crudo, ruta_agregado, ruta_salida)
        total += filas
        print(f"[{i}/{len(archivos_agregados)}] {nombre}: {filas} filas -> {ruta_salida}")

    print(f"\nListo. {total} filas totales procesadas en {args.salida}")


if __name__ == "__main__":
    main()