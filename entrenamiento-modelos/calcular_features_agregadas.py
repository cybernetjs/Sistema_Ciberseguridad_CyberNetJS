import argparse
import glob
import os
from collections import deque

import pandas as pd


VENTANA_CORTA_SEGUNDOS = 5.0
VENTANA_MEDIA_SEGUNDOS = 60.0
VENTANA_LARGA_SEGUNDOS = 300.0


def calcular_para_grupo(grupo):
    grupo = grupo.sort_values("ts").reset_index(drop=True)
    historial = deque()

    conexiones_5s = []
    puertos_5s = []
    ips_60s = []
    conexiones_mismo_destino_300s = []

    for _, fila in grupo.iterrows():
        instante_actual = fila["ts"]
        historial.append((instante_actual, fila["id.resp_h"], fila["id.resp_p"], fila["proto"]))

        while historial and instante_actual - historial[0][0] > VENTANA_LARGA_SEGUNDOS:
            historial.popleft()

        c5 = 0
        puertos_vistos = set()
        ips_vistas = set()
        c300 = 0

        for instante, ip_destino, puerto_destino, protocolo in historial:
            antiguedad = instante_actual - instante

            if antiguedad <= VENTANA_CORTA_SEGUNDOS:
                c5 += 1
                puertos_vistos.add(puerto_destino)

            if antiguedad <= VENTANA_MEDIA_SEGUNDOS:
                ips_vistas.add(ip_destino)

            if (antiguedad <= VENTANA_LARGA_SEGUNDOS and
                    ip_destino == fila["id.resp_h"] and
                    puerto_destino == fila["id.resp_p"] and
                    protocolo == fila["proto"]):
                c300 += 1

        conexiones_5s.append(c5)
        puertos_5s.append(len(puertos_vistos))
        ips_60s.append(len(ips_vistas))
        conexiones_mismo_destino_300s.append(c300)

    grupo["conexiones_origen_5s"] = conexiones_5s
    grupo["puertos_distintos_origen_5s"] = puertos_5s
    grupo["ips_distintas_origen_60s"] = ips_60s
    grupo["conexiones_mismo_destino_300s"] = conexiones_mismo_destino_300s

    return grupo


def procesar_archivo(ruta_entrada, ruta_salida):
    df = pd.read_csv(ruta_entrada, low_memory=False)

    columnas_necesarias = {"ts", "id.orig_h", "id.resp_h", "id.resp_p", "proto"}
    faltantes = columnas_necesarias - set(df.columns)
    if faltantes:
        print(f"Saltando {ruta_entrada}, faltan columnas: {faltantes}")
        return 0

    df["ts"] = pd.to_numeric(df["ts"], errors="coerce")
    df = df.dropna(subset=["ts"])

    resultado = df.groupby("id.orig_h", group_keys=False).apply(calcular_para_grupo)
    resultado = resultado.sort_index()

    resultado.to_csv(ruta_salida, index=False)
    return len(resultado)


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

    print(f"Procesando {len(archivos)} archivo(s)")

    total_filas = 0
    for i, ruta in enumerate(archivos, start=1):
        nombre_salida = os.path.join(args.salida, os.path.basename(ruta))
        filas = procesar_archivo(ruta, nombre_salida)
        total_filas += filas
        print(f"[{i}/{len(archivos)}] {os.path.basename(ruta)}: {filas} filas -> {nombre_salida}")

    print(f"\nListo. {total_filas} filas totales procesadas en {args.salida}")


if __name__ == "__main__":
    main()