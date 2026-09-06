import argparse

import pandas as pd


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--eventos", required=True)
    argumentos = parser.parse_args()

    df = pd.read_csv(argumentos.eventos, low_memory=False)

    total_filas = len(df)
    total_ia_amenaza = (df["veredicto_ia_es_amenaza"] == 1).sum()
    total_regla_amenaza = (df["es_amenaza"] == 1).sum()

    print(f"Total de filas: {total_filas}")
    print(f"Filas con veredicto_ia_es_amenaza = 1: {total_ia_amenaza}")
    print(f"Filas con es_amenaza (reglas) = 1: {total_regla_amenaza}")

    if total_ia_amenaza > 0:
        print()
        print("Filas donde la IA marco amenaza:")
        columnas_interes = ["marca_tiempo_unix", "ip_origen", "ip_destino", "puerto_destino",
                             "clasificador", "veredicto_ia_confianza"]
        print(df.loc[df["veredicto_ia_es_amenaza"] == 1, columnas_interes].to_string(index=False))
    else:
        print()
        print("La IA no marco ninguna fila como amenaza en este archivo.")


if __name__ == "__main__":
    main()