import argparse
import json

import joblib


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--joblib", required=True)
    parser.add_argument("--salida", required=True)
    args = parser.parse_args()

    datos = joblib.load(args.joblib)
    escalador = datos["escalador"]
    columnas = datos["columnas"]
    columnas_seleccionadas = datos["columnas_seleccionadas"]

    indices = [columnas.index(c) for c in columnas_seleccionadas]

    parametros = {
        "orden_caracteristicas": columnas_seleccionadas,
        "media": [float(escalador.mean_[i]) for i in indices],
        "desviacion": [float(escalador.scale_[i]) for i in indices],
    }

    with open(args.salida, "w", encoding="utf-8") as f:
        json.dump(parametros, f, indent=2, ensure_ascii=False)

    print(json.dumps(parametros, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
