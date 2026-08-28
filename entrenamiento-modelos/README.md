# entrenamiento-modelos (Python)

Entrenamiento sin conexion del modelo de clasificacion, fuera del stack
final en produccion.

Responsabilidades:
- Cargar y explorar CICIoT2023 e IoT-23.
- Preprocesamiento: limpieza, codificacion, normalizacion.
- Balanceo de clases con SMOTE.
- Seleccion de caracteristicas (correlacion de Pearson / K-Best).
- Entrenamiento y validacion del modelo de clasificacion (XGBoost).
- Exportar el modelo final a `../modelos-entrenados/` en formato `.json`
  (nativo XGBoost) o `.onnx` (para consumir desde `servicio-inferencia`).

## Estructura sugerida
```
entrenamiento-modelos/
├── exploracion/          # exploracion de datos (EDA)
├── preprocesamiento/     # limpieza, SMOTE, seleccion de caracteristicas
├── entrenar_xgboost.py   # entrenamiento y validacion
├── exportar_modelo.py    # exportar a .json / .onnx
└── requirements.txt
```

## Dependencias (requirements.txt sugerido)
```
pandas
scikit-learn
imbalanced-learn
xgboost
onnxmltools
```
