import argparse
import json
import math

import joblib
import xgboost as xgb


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--modelo", required=True)
    parser.add_argument("--joblib", required=True)
    parser.add_argument("--salida", required=True)
    args = parser.parse_args()

    booster = xgb.Booster()
    booster.load_model(args.modelo)

    config = json.loads(booster.save_config())
    base_score_crudo = config["learner"]["learner_model_param"]["base_score"]
    base_score_texto = str(base_score_crudo).strip("[]")
    base_score = float(base_score_texto)
    sesgo_inicial = math.log(base_score / (1.0 - base_score))

    volcado = booster.get_dump(dump_format="json")
    arboles = [json.loads(arbol) for arbol in volcado]

    datos = joblib.load(args.joblib)
    columnas = datos["columnas"]
    columnas_seleccionadas = datos["columnas_seleccionadas"]
    escalador = datos["escalador"]
    indices = [columnas.index(c) for c in columnas_seleccionadas]

    salida = {
        "orden_caracteristicas": columnas_seleccionadas,
        "media": [float(escalador.mean_[i]) for i in indices],
        "desviacion": [float(escalador.scale_[i]) for i in indices],
        "sesgo_inicial": sesgo_inicial,
        "arboles": arboles,
    }

    with open(args.salida, "w", encoding="utf-8") as f:
        json.dump(salida, f)

    print(f"Exportado: {len(arboles)} arboles, {len(columnas_seleccionadas)} caracteristicas")
    print(f"sesgo_inicial: {sesgo_inicial}")


if __name__ == "__main__":
    main()