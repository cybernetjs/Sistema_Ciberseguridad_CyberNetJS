import argparse

import pandas as pd


def construir_pares_ataque(df):
    alertas = df[df["es_amenaza"] == 1]
    pares = set()
    for _, fila in alertas.iterrows():
        pares.add((fila["ip_origen"], fila["ip_destino"]))
    return pares


def etiquetar_fila(fila, pares):
    clave = (fila["ip_origen"], fila["ip_destino"])
    if clave in pares:
        return "Malicious"
    return "benign"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--eventos", required=True)
    parser.add_argument("--salida", required=True)
    argumentos = parser.parse_args()

    df = pd.read_csv(argumentos.eventos, low_memory=False)

    pares = construir_pares_ataque(df)
    if not pares:
        print("No se encontraron alertas en este CSV, todo el archivo se etiquetara como benigno")

    df["label"] = df.apply(lambda fila: etiquetar_fila(fila, pares), axis=1)

    columnas_salida = {
        "marca_tiempo_unix": "ts",
        "ip_origen": "id.orig_h",
        "ip_destino": "id.resp_h",
        "puerto_origen": "id.orig_p",
        "puerto_destino": "id.resp_p",
        "protocolo": "proto",
        "duracion": "duration",
        "bytes_origen": "orig_bytes",
        "bytes_destino": "resp_bytes",
        "missed_bytes": "missed_bytes",
        "orig_pkts_flujo": "orig_pkts",
        "orig_ip_bytes_flujo": "orig_ip_bytes",
        "resp_pkts_flujo": "resp_pkts",
        "resp_ip_bytes_flujo": "resp_ip_bytes",
    }

    columnas_presentes = {origen: destino for origen, destino in columnas_salida.items() if origen in df.columns}

    df_salida = df.rename(columns=columnas_presentes)

    if "conexiones_origen_5s" in df.columns:
        columnas_finales = list(columnas_presentes.values()) + [
            "conexiones_origen_5s",
            "puertos_distintos_origen_5s",
            "ips_distintas_origen_60s",
            "conexiones_mismo_destino_300s",
            "label",
        ]
    else:
        columnas_finales = list(columnas_presentes.values()) + ["label"]

    df_salida = df_salida[columnas_finales]
    df_salida.to_csv(argumentos.salida, index=False)

    total_malicioso = (df["label"] == "Malicious").sum()
    total_benigno = (df["label"] == "benign").sum()
    print(f"Filas etiquetadas como Malicious: {total_malicioso}")
    print(f"Filas etiquetadas como benign: {total_benigno}")


if __name__ == "__main__":
    main()