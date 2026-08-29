import glob
import os

import pandas as pd


def cargar_dataset(ruta):
    if os.path.isdir(ruta):
        archivos = sorted(glob.glob(os.path.join(ruta, "*.csv")))
        if not archivos:
            raise FileNotFoundError(ruta)
        marcos = [pd.read_csv(archivo, low_memory=False) for archivo in archivos]
        return pd.concat(marcos, ignore_index=True)
    return pd.read_csv(ruta, low_memory=False)
