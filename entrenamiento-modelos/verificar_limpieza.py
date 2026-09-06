import sys
sys.path.append(".")
from cargar_datos import cargar_dataset
from preprocesamiento import limpiar

df = cargar_dataset("../datasets/crudo/IoT-23-muestra")
df = df.drop(columns=[c for c in ["ts", "missed_bytes"] if c in df.columns])
limpio = limpiar(df, "label")
print("total filas despues de excluir y limpiar:", len(limpio))
print(limpio["label"].value_counts())