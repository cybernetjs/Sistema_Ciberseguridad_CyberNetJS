import numpy as np
import pandas as pd
from imblearn.over_sampling import SMOTE
from sklearn.feature_selection import SelectKBest, f_classif
from sklearn.preprocessing import LabelEncoder, StandardScaler


def limpiar(df, columna_etiqueta):
    df = df.drop_duplicates()
    for columna in df.columns:
        if columna == columna_etiqueta:
            continue
        if df[columna].dtype == object:
            convertida = pd.to_numeric(df[columna], errors="coerce")
            if convertida.notna().sum() > 0:
                df[columna] = convertida.fillna(0)
    df = df.replace([np.inf, -np.inf], np.nan)
    df = df.dropna()
    columnas_texto = df.select_dtypes(include="object").columns.tolist()
    if columna_etiqueta in columnas_texto:
        columnas_texto.remove(columna_etiqueta)
    df = df.drop(columns=columnas_texto)
    return df.reset_index(drop=True)


def separar_binario(etiquetas, etiqueta_benigna):
    normalizada = etiqueta_benigna.strip().lower()
    return np.array([0 if str(valor).strip().lower() == normalizada else 1 for valor in etiquetas])


def codificar_etiquetas(etiquetas):
    codificador = LabelEncoder()
    etiquetas_codificadas = codificador.fit_transform(etiquetas)
    return etiquetas_codificadas, codificador


def normalizar(X_entrenamiento, X_prueba):
    escalador = StandardScaler()
    X_entrenamiento_normalizado = escalador.fit_transform(X_entrenamiento)
    X_prueba_normalizado = escalador.transform(X_prueba)
    return X_entrenamiento_normalizado, X_prueba_normalizado, escalador


def balancear(X, y):
    smote = SMOTE(random_state=42)
    return smote.fit_resample(X, y)


def seleccionar_caracteristicas(X, y, columnas, k):
    k = max(1, min(k, X.shape[1]))
    selector = SelectKBest(score_func=f_classif, k=k)
    selector.fit(X, y)
    soporte = selector.get_support()
    columnas_seleccionadas = [columna for columna, mantenida in zip(columnas, soporte) if mantenida]
    return selector, columnas_seleccionadas