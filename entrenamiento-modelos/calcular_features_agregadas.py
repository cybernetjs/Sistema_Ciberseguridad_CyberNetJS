import argparse
import glob
import os
import time
from collections import Counter, deque

import pandas as pd


VENTANA_CORTA_SEGUNDOS = 5.0
VENTANA_MEDIA_SEGUNDOS = 60.0
VENTANA_LARGA_SEGUNDOS = 300.0


def calcular_para_grupo(tiempos, ips_destino, puertos_destino, protocolos):
    n = len(tiempos)

    conexiones_5s = [0] * n
    puertos_5s = [0] * n
    ips_60s = [0] * n
    conexiones_mismo_destino_300s = [0] * n

    ventana_5s = deque()
    ventana_60s = deque()
    ventana_300s = deque()

    contador_puertos_5s = Counter()
    contador_ips_60s = Counter()
    contador_destino_300s = Counter()

    for i in range(n):
        instante_actual = tiempos[i]
        clave_destino = (ips_destino[i], puertos_destino[i], protocolos[i])

        ventana_5s.append((instante_actual, puertos_destino[i]))
        while ventana_5s and instante_actual - ventana_5s[0][0] > VENTANA_CORTA_SEGUNDOS:
            _, puerto_viejo = ventana_5s.popleft()
            contador_puertos_5s[puerto_viejo] -= 1
            if contador_puertos_5s[puerto_viejo] <= 0:
                del contador_puertos_5s[puerto_viejo]
        contador_puertos_5s[puertos_destino[i]] += 1

        ventana_60s.append((instante_actual, ips_destino[i]))
        while ventana_60s and instante_actual - ventana_60s[0][0] > VENTANA_MEDIA_SEGUNDOS:
            _, ip_vieja = ventana_60s.popleft()
            contador_ips_60s[ip_vieja] -= 1
            if contador_ips_60s[ip_vieja] <= 0:
                del contador_ips_60s[ip_vieja]
        contador_ips_60s[ips_destino[i]] += 1

        ventana_300s.append((instante_actual, clave_destino))
        while ventana_300s and instante_actual - ventana_300s[0][0] > VENTANA_LARGA_SEGUNDOS:
            _, clave_vieja = ventana_300s.popleft()
            contador_destino_300s[clave_vieja] -= 1
            if contador_destino_300s[clave_vieja] <= 0:
                del contador_destino_300s[clave_vieja]
        contador_destino_300s[clave_destino] += 1

        conexiones_5s[i] = len(ventana_5s)
        puertos_5s[i] = len(contador_puertos_5s)
        ips_60s[i] = len(contador_ips_60s)
        conexiones_mismo_destino_300s[i] = contador_destino_300s[clave_destino]

    return conexiones_5s, puertos_5s, ips_60s, conexiones_mismo_destino_300s


def procesar_archivo(ruta_entrada, ruta_salida):
    df = pd.read_csv(ruta_entrada, low_memory=False)

    columnas_necesarias = {"ts", "id.orig_h", "id.resp_h", "id.resp_p", "proto"}
    faltantes = columnas_necesarias - set(df.columns)
    if faltantes:
        print(f"Saltando {ruta_entrada}, faltan columnas: {faltantes}")
        return 0

    df["ts"] = pd.to_numeric(df["ts"], errors="coerce")
    df = df.dropna(subset=["ts"])
    df = df.reset_index(drop=True)

    total_filas = len(df)

    columna_c5 = [0] * total_filas
    columna_p5 = [0] * total_filas
    columna_i60 = [0] * total_filas
    columna_c300 = [0] * total_filas

    grupos = df.groupby("id.orig_h").indices
    total_grupos = len(grupos)

    print(f"  {total_filas} filas, {total_grupos} IPs origen distintas")

    inicio = time.time()
    filas_procesadas = 0

    for i, (_, indices) in enumerate(grupos.items(), start=1):
        indices_ordenados = sorted(indices, key=lambda idx: df.at[idx, "ts"])

        tiempos = [df.at[idx, "ts"] for idx in indices_ordenados]
        ips_destino = [df.at[idx, "id.resp_h"] for idx in indices_ordenados]
        puertos_destino = [df.at[idx, "id.resp_p"] for idx in indices_ordenados]
        protocolos = [df.at[idx, "proto"] for idx in indices_ordenados]

        c5, p5, i60, c300 = calcular_para_grupo(tiempos, ips_destino, puertos_destino, protocolos)

        for pos, idx in enumerate(indices_ordenados):
            columna_c5[idx] = c5[pos]
            columna_p5[idx] = p5[pos]
            columna_i60[idx] = i60[pos]
            columna_c300[idx] = c300[pos]

        filas_procesadas += len(indices_ordenados)

        if i % 25 == 0 or i == total_grupos:
            transcurrido = time.time() - inicio
            porcentaje = 100.0 * filas_procesadas / total_filas
            print(f"    IP {i}/{total_grupos} ({porcentaje:.1f}% de filas, {transcurrido:.0f}s transcurridos)")

    df["conexiones_origen_5s"] = columna_c5
    df["puertos_distintos_origen_5s"] = columna_p5
    df["ips_distintas_origen_60s"] = columna_i60
    df["conexiones_mismo_destino_300s"] = columna_c300

    df.to_csv(ruta_salida, index=False)
    return total_filas


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
        print(f"[{i}/{len(archivos)}] {os.path.basename(ruta)}")
        filas = procesar_archivo(ruta, nombre_salida)
        total_filas += filas
        print(f"[{i}/{len(archivos)}] {os.path.basename(ruta)}: {filas} filas -> {nombre_salida}")

    print(f"\nListo. {total_filas} filas totales procesadas en {args.salida}")


if __name__ == "__main__":
    main()