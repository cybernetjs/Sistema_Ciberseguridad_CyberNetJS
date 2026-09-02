import glob
import pandas as pd

archivos = sorted(glob.glob("../datasets/crudo/IoT-23-muestra/*.csv"))
marcos = [pd.read_csv(archivo, low_memory=False) for archivo in archivos]
df = pd.concat(marcos, ignore_index=True)

print(df.groupby("label")["ts"].agg(["min", "max"]))