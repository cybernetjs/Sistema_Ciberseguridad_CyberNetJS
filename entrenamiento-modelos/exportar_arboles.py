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
    num_class = int(config["learner"]["learner_model_param"]["num_class"])
    base_score_crudo = config["learner"]["learner_model_param"]["base_score"]
    base_score_texto = str(base_score_crudo).strip("[]")

    volcado = booster.get_dump(dump_format="json")
    arboles = [json.loads(arbol) for arbol in volcado]

    datos = joblib.load(args.joblib)
    columnas = datos["columnas"]
    columnas_seleccionadas = datos["columnas_seleccionadas"]
    escalador = datos["escalador"]
    indices = [columnas.index(c) for c in columnas_seleccionadas]

    if num_class <= 1:
        base_score = float(base_score_texto)
        sesgo_inicial = [math.log(base_score / (1.0 - base_score))]
        clases = ["benigno", "ataque"]
        num_class_salida = 1
    else:
        sesgo_inicial = [float(v) for v in base_score_texto.split(",")]
        codificador = datos.get("codificador")
        if codificador is not None:
            clases = [str(c) for c in codificador.classes_]
        else:
            clases = [f"clase_{i}" for i in range(num_class)]
        num_class_salida = num_class

    salida = {
        "orden_caracteristicas": columnas_seleccionadas,
        "media": [float(escalador.mean_[i]) for i in indices],
        "desviacion": [float(escalador.scale_[i]) for i in indices],
        "sesgo_inicial": sesgo_inicial,
        "num_clases": num_class_salida,
        "clases": clases,
        "arboles": arboles,
    }

    with open(args.salida, "w", encoding="utf-8") as f:
        json.dump(salida, f)

    print(f"Exportado: {len(arboles)} arboles, {len(columnas_seleccionadas)} caracteristicas")
    print(f"num_clases: {num_class_salida}")
    print(f"clases: {clases}")
    print(f"sesgo_inicial: {sesgo_inicial}")


if __name__ == "__main__":
    main()