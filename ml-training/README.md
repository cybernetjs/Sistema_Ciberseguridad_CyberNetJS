# ml-training (Python)

Entrenamiento offline del modelo, fuera del stack final en producción.

Responsabilidades:
- Cargar y explorar CICIoT2023 e IoT-23.
- Preprocesamiento: limpieza, codificación, normalización.
- Balanceo de clases con SMOTE.
- Selección de características (correlación de Pearson / K-Best).
- Entrenamiento y validación del modelo XGBoost.
- Exportar el modelo final a `../models/` en formato `.json` (nativo XGBoost)
  o `.onnx` (para consumir desde `inference-service` en C++).

## Estructura sugerida
```
ml-training/
├── notebooks/            # exploración de datos (EDA)
├── preprocessing/        # limpieza, SMOTE, selección de características
├── train_xgboost.py      # entrenamiento y validación
├── export_model.py       # exportar a .json / .onnx
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
