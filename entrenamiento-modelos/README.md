# entrenamiento-modelos (Python)

Entrenamiento sin conexion del modelo de clasificacion (Fases 3 y 4 del
CRISP-DM: Preparacion de los datos y Modelado), fuera del stack en
produccion.

## Instalar dependencias

```bash
cd entrenamiento-modelos
pip install -r requirements.txt
```

## Archivos

- `cargar_datos.py` — `cargar_dataset(ruta)`: si `ruta` es un archivo CSV lo
  carga directo; si es una carpeta, junta todos los `.csv` que encuentre
  dentro (asi se puede apuntar directo a la carpeta donde quedo
  descomprimido CICIoT2023, que trae muchos archivos `part-*.csv`).
- `preprocesamiento.py` — limpieza (duplicados, nulos, infinitos, columnas
  de texto que no sean la etiqueta), codificacion de la etiqueta, division
  binaria (benigno vs ataque), normalizacion con `StandardScaler`, balanceo
  con `SMOTE` y seleccion de caracteristicas con `SelectKBest`.
- `entrenar_xgboost.py` — arma el flujo completo: carga, limpia, separa
  entrenamiento/prueba, normaliza, balancea, selecciona caracteristicas,
  entrena `XGBClassifier`, calcula Accuracy/Precision/Recall/F1-Score/
  matriz de confusion/tiempo de prediccion por registro, y exporta todo.

## Ejecutar

Clasificacion binaria (benigno / ataque, es lo que necesita la hipotesis
general de la tesis):

```bash
python3 entrenar_xgboost.py --datos ../datasets/crudo/CICIoT2023 --binario --salida ../modelos-entrenados/modelo.json
```

Clasificacion multiclase (identificar el tipo de ataque, para la parte de
"Precision por tipo de ataque" de la Tabla 2):

```bash
python3 entrenar_xgboost.py --datos ../datasets/crudo/CICIoT2023 --salida ../modelos-entrenados/modelo.json
```

Parametros disponibles:

| Parametro | Default | Que hace |
|---|---|---|
| `--datos` | (obligatorio) | Ruta a un CSV o a una carpeta con varios CSV |
| `--etiqueta` | `label` | Nombre de la columna con la clase (`label` en CICIoT2023) |
| `--benigna` | `BenignTraffic` | Valor de la columna de etiqueta que representa trafico normal |
| `--k` | `15` | Cantidad de caracteristicas a conservar (SelectKBest) |
| `--salida` | `../modelos-entrenados/modelo.json` | Donde se guarda el modelo entrenado |
| `--binario` | apagado | Si se pasa, entrena benigno-vs-ataque en vez de multiclase |

Para IoT-23 la columna de etiqueta suele llamarse distinto (revisar el CSV
antes de correr el script y ajustar `--etiqueta`/`--benigna` segun
corresponda).

## Que queda en `modelos-entrenados/` despues de correrlo

- `modelo.json` — el modelo XGBoost en formato nativo.
- `preprocesamiento.joblib` — el `StandardScaler`, el `SelectKBest`, la
  lista de columnas originales, las columnas seleccionadas y el
  `LabelEncoder` (si es multiclase). Se necesita para transformar
  cualquier evento nuevo exactamente igual que en el entrenamiento antes
  de pasarselo al modelo.
- `metricas.json` — accuracy, precision, recall, f1_score, matriz de
  confusion, tiempo de prediccion por registro (ms) y las columnas que
  selecciono el modelo. Estos son los numeros que van directo a la Fase 5
  de evaluacion de la tesis.

## Conseguir los datasets

CICIoT2023 e IoT-23 no se pueden descargar con un script automatico: el
sitio del CIC (unb.ca) pide llenar un formulario corto antes de dar el
link de descarga, y el mirror de IoT-23 tambien requiere navegar el sitio
de Stratosphere Lab manualmente. Hay que descargarlos a mano desde:

- CICIoT2023: https://www.unb.ca/cic/datasets/iotdataset-2023.html
- IoT-23: https://www.stratosphereips.org/datasets-iot23

y dejarlos en `../datasets/crudo/` (esa carpeta esta excluida de git por
el peso). Una vez ahi, `--datos ../datasets/crudo/<carpeta_del_dataset>`
apunta directo a los CSV.
