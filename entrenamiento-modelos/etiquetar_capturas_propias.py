import argparse
import csv

import pandas as pd


def cargar_ventanas(ruta_ventanas):
    ventanas = []
    with open(ruta_ventanas, newline="") as archivo:
        lector = csv.DictReader(archivo)
        for fila in lector:
            ventanas.append((float(fila["inicio"]), float(fila["fin"]), fila["etiqueta"]))
    return ventanas


def etiquetar(marca_tiempo, ventanas, etiqueta_por_defecto):
    for inicio, fin, etiqueta in ventanas:
        if inicio <= marca_tiempo <= fin:
            return etiqueta
    return etiqueta_por_defecto


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--eventos", required=True)
    parser.add_argument("--ventanas", required=True)
    parser.add_argument("--salida", required=True)
    parser.add_argument("--etiqueta-por-defecto", default="benign")
    argumentos = parser.parse_args()

    df = pd.read_csv(argumentos.eventos, low_memory=False)
    ventanas = cargar_ventanas(argumentos.ventanas)

    df["label"] = df["marca_tiempo_unix"].apply(lambda t: etiquetar(t, ventanas, argumentos.etiqueta_por_defecto))

    columnas_salida = {
        "puerto_origen": "id.orig_p",
        "puerto_destino": "id.resp_p",
        "duracion": "duration",
        "bytes_origen": "orig_bytes",
        "bytes_destino": "resp_bytes",
        "missed_bytes": "missed_bytes",
        "orig_pkts_flujo": "orig_pkts",
        "orig_ip_bytes_flujo": "orig_ip_bytes",
        "resp_pkts_flujo": "resp_pkts",
        "resp_ip_bytes_flujo": "resp_ip_bytes",
    }

    df_salida = df.rename(columns=columnas_salida)
    columnas_finales = list(columnas_salida.values()) + ["label"]
    df_salida = df_salida[columnas_finales]
    df_salida.to_csv(argumentos.salida, index=False)


if __name__ == "__main__":
    main()