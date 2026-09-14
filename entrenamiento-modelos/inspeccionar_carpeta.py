import glob
import os
import sys

import pandas as pd


def inspeccionar(carpeta, lineas):
    archivos = sorted(glob.glob(os.path.join(carpeta, "*.csv")))
    if not archivos:
        mensaje = "No hay CSV en " + carpeta
        print(mensaje)
        lineas.append(mensaje)
        return

    encabezado = "=== " + carpeta + " ==="
    conteo_archivos = str(len(archivos)) + " archivo(s)"
    print(encabezado)
    print(conteo_archivos)
    lineas.append(encabezado)
    lineas.append(conteo_archivos)

    for ruta in archivos:
        df = pd.read_csv(ruta, low_memory=False, nrows=200000)
        nombre = os.path.basename(ruta)
        print("")
        print(nombre)
        lineas.append("")
        lineas.append(nombre)

        linea_columnas = "  columnas: " + str(df.columns.tolist())
        print(linea_columnas)
        lineas.append(linea_columnas)

        columna_etiqueta = None
        if "categoria" in df.columns:
            columna_etiqueta = "categoria"
        elif "label" in df.columns:
            columna_etiqueta = "label"

        if columna_etiqueta:
            conteos = df[columna_etiqueta].value_counts()
            linea_deteccion = "  columna de etiqueta detectada: " + columna_etiqueta
            print(linea_deteccion)
            lineas.append(linea_deteccion)
            for valor, cantidad in conteos.items():
                linea_valor = "    " + str(valor) + ": " + str(cantidad)
                print(linea_valor)
                lineas.append(linea_valor)
        else:
            linea_sin = "  no se encontro columna categoria ni label"
            print(linea_sin)
            lineas.append(linea_sin)


def main():
    if len(sys.argv) < 2:
        print("Uso: python inspeccionar_carpeta.py carpeta_padre")
        return

    carpeta_padre = sys.argv[1]

    if not os.path.isdir(carpeta_padre):
        print(carpeta_padre + " no existe")
        return

    subcarpetas = sorted(
        os.path.join(carpeta_padre, nombre)
        for nombre in os.listdir(carpeta_padre)
        if os.path.isdir(os.path.join(carpeta_padre, nombre))
    )

    lineas = []

    if not subcarpetas:
        inspeccionar(carpeta_padre, lineas)
    else:
        for subcarpeta in subcarpetas:
            inspeccionar(subcarpeta, lineas)
            print("")
            lineas.append("")

    ruta_reporte = os.path.join(carpeta_padre, "inspeccion_reporte.txt")
    with open(ruta_reporte, "w", encoding="utf-8") as salida:
        salida.write("\n".join(lineas))

    print("Reporte guardado en: " + ruta_reporte)


if __name__ == "__main__":
    main()
