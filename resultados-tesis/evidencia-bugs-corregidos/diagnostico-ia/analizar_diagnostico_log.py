import argparse
import re

import pandas as pd


PATRON = re.compile(
    r"DIAGNOSTICO_IA probabilidad=([\d.]+) orig_pkts_flujo=(\d+) orig_ip_bytes_flujo=(\d+) "
    r"resp_pkts_flujo=(\d+) resp_ip_bytes_flujo=(\d+) duracion=([\d.]+) pps_flujo=([\d.]+)"
)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--log", required=True)
    parser.add_argument("--salida", default=None)
    argumentos = parser.parse_args()

    filas = []
    with open(argumentos.log, encoding="utf-8", errors="ignore") as archivo:
        for linea in archivo:
            coincidencia = PATRON.search(linea)
            if coincidencia:
                filas.append({
                    "probabilidad": float(coincidencia.group(1)),
                    "orig_pkts_flujo": int(coincidencia.group(2)),
                    "orig_ip_bytes_flujo": int(coincidencia.group(3)),
                    "resp_pkts_flujo": int(coincidencia.group(4)),
                    "resp_ip_bytes_flujo": int(coincidencia.group(5)),
                    "duracion": float(coincidencia.group(6)),
                    "pps_flujo": float(coincidencia.group(7)),
                })

    if not filas:
        print("No se encontraron lineas DIAGNOSTICO_IA en este log")
        print("Eso significa que la probabilidad nunca supero 0.3 en ningun momento")
        return

    df = pd.DataFrame(filas)

    print(f"Total de lineas de diagnostico encontradas: {len(df)}")
    print(f"Probabilidad minima: {df['probabilidad'].min():.6f}")
    print(f"Probabilidad maxima: {df['probabilidad'].max():.6f}")
    print(f"Probabilidad promedio: {df['probabilidad'].mean():.6f}")
    print()
    print("Distribucion de probabilidad (percentiles):")
    print(df["probabilidad"].describe())
    print()
    print("Las 10 filas con probabilidad mas alta:")
    print(df.sort_values("probabilidad", ascending=False).head(10).to_string(index=False))

    if argumentos.salida:
        df.to_csv(argumentos.salida, index=False)
        print()
        print(f"Guardado detalle completo en: {argumentos.salida}")


if __name__ == "__main__":
    main()