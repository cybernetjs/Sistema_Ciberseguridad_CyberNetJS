# datasets

Los datasets no se versionan (son muy pesados: CICIoT2023 tiene decenas de GB).

CICIoT2023 e IoT-23 no tienen un link de descarga directo: hay que
descargarlos a mano (el sitio del CIC pide llenar un formulario corto
antes de dar acceso). Ver `../entrenamiento-modelos/README.md`, seccion
"Conseguir los datasets", para los links y el detalle.

Una vez descargados, se descomprimen dentro de `crudo/` (por ejemplo
`crudo/CICIoT2023/`, `crudo/IoT-23/`). Esa carpeta y los `.csv` sueltos
estan excluidos en `.gitignore`.

`entrenamiento-modelos/entrenar_xgboost.py --datos crudo/CICIoT2023` lee
directo todos los CSV de esa carpeta.
