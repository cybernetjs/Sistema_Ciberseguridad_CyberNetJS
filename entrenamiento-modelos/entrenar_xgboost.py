import argparse
import json
import os
import time

import joblib
from sklearn.metrics import (
    accuracy_score,
    classification_report,
    confusion_matrix,
    f1_score,
    precision_score,
    recall_score,
)
from sklearn.model_selection import train_test_split
from xgboost import XGBClassifier

from cargar_datos import cargar_dataset
from preprocesamiento import (
    balancear,
    codificar_etiquetas,
    limpiar,
    normalizar,
    seleccionar_caracteristicas,
    separar_binario,
)


def entrenar(ruta_datos, columna_etiqueta, etiqueta_benigna, k_caracteristicas, ruta_salida, binario, columnas_excluir):
    df = cargar_dataset(ruta_datos)
    df = limpiar(df, columna_etiqueta)

    if columnas_excluir:
        columnas_a_quitar = [c.strip() for c in columnas_excluir.split(",") if c.strip() in df.columns]
        df = df.drop(columns=columnas_a_quitar)

    if binario:
        etiquetas = separar_binario(df[columna_etiqueta], etiqueta_benigna)
        nombres_clases = ["benigno", "ataque"]
        codificador = None
    else:
        etiquetas, codificador = codificar_etiquetas(df[columna_etiqueta])
        nombres_clases = [str(clase) for clase in codificador.classes_]

    X = df.drop(columns=[columna_etiqueta])
    columnas = X.columns.tolist()

    X_entrenamiento, X_prueba, y_entrenamiento, y_prueba = train_test_split(
        X.values, etiquetas, test_size=0.2, random_state=42, stratify=etiquetas
    )

    X_entrenamiento, X_prueba, escalador = normalizar(X_entrenamiento, X_prueba)
    X_entrenamiento, y_entrenamiento = balancear(X_entrenamiento, y_entrenamiento)

    selector, columnas_seleccionadas = seleccionar_caracteristicas(
        X_entrenamiento, y_entrenamiento, columnas, k_caracteristicas
    )
    X_entrenamiento = selector.transform(X_entrenamiento)
    X_prueba = selector.transform(X_prueba)

    modelo = XGBClassifier(
        n_estimators=300,
        max_depth=6,
        learning_rate=0.1,
        subsample=0.9,
        colsample_bytree=0.9,
        random_state=42,
        n_jobs=-1,
    )
    modelo.fit(X_entrenamiento, y_entrenamiento)

    inicio = time.perf_counter()
    predicciones = modelo.predict(X_prueba)
    fin = time.perf_counter()
    tiempo_prediccion_ms_por_registro = ((fin - inicio) / len(X_prueba)) * 1000

    metricas = {
        "accuracy": accuracy_score(y_prueba, predicciones),
        "precision": precision_score(y_prueba, predicciones, average="weighted", zero_division=0),
        "recall": recall_score(y_prueba, predicciones, average="weighted", zero_division=0),
        "f1_score": f1_score(y_prueba, predicciones, average="weighted", zero_division=0),
        "matriz_confusion": confusion_matrix(y_prueba, predicciones).tolist(),
        "tiempo_prediccion_ms_por_registro": tiempo_prediccion_ms_por_registro,
        "clases": nombres_clases,
        "caracteristicas_seleccionadas": columnas_seleccionadas,
        "total_registros_entrenamiento": int(len(X_entrenamiento)),
        "total_registros_prueba": int(len(X_prueba)),
    }

    reporte = classification_report(
        y_prueba, predicciones, target_names=nombres_clases, zero_division=0
    )
    print(reporte)
    print(json.dumps({clave: metricas[clave] for clave in ("accuracy", "precision", "recall", "f1_score")}, indent=2))

    carpeta_salida = os.path.dirname(ruta_salida) or "."
    os.makedirs(carpeta_salida, exist_ok=True)
    prefijo = os.path.splitext(os.path.basename(ruta_salida))[0]

    modelo.save_model(ruta_salida)

    joblib.dump(
        {
            "escalador": escalador,
            "selector": selector,
            "columnas": columnas,
            "columnas_seleccionadas": columnas_seleccionadas,
            "codificador": codificador,
            "binario": binario,
        },
        os.path.join(carpeta_salida, f"{prefijo}_preprocesamiento.joblib"),
    )

    with open(os.path.join(carpeta_salida, f"{prefijo}_metricas.json"), "w", encoding="utf-8") as archivo:
        json.dump(metricas, archivo, indent=2, ensure_ascii=False)

    return modelo, metricas


def construir_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument("--datos", required=True)
    parser.add_argument("--etiqueta", default="label")
    parser.add_argument("--excluir", default="")
    parser.add_argument("--benigna", default="BenignTraffic")
    parser.add_argument("--k", type=int, default=15)
    parser.add_argument("--salida", default="../modelos-entrenados/modelo.json")
    parser.add_argument("--binario", action="store_true")
    return parser


if __name__ == "__main__":
    argumentos = construir_parser().parse_args()
    entrenar(
        argumentos.datos,
        argumentos.etiqueta,
        argumentos.benigna,
        argumentos.k,
        argumentos.salida,
        argumentos.binario,
        argumentos.excluir,
    )