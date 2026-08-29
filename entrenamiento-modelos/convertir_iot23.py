import argparse
import glob
import os
import re
import csv

SPLIT_RE = re.compile(r"\s{2,}")


def dividir_cola(cola, n=3):
    partes = SPLIT_RE.split(cola.strip())
    if len(partes) < n:
        partes += [""] * (n - len(partes))
    elif len(partes) > n:
        partes = partes[:n - 1] + [" ".join(partes[n - 1:])]
    return partes


def procesar_archivo(ruta_entrada, ruta_salida):
    columnas = None
    filas_escritas = 0

    with open(ruta_entrada, "r", encoding="utf-8", errors="ignore") as f_in, \
         open(ruta_salida, "w", newline="", encoding="utf-8") as f_out:

        escritor = csv.writer(f_out)

        for linea in f_in:
            linea = linea.rstrip("\n")

            if linea.startswith("#fields"):
                crudo = linea.split("\t")[1:]
                columnas = crudo[:-1] + dividir_cola(crudo[-1])
                escritor.writerow(columnas)
                continue

            if linea.startswith("#"):
                continue

            if columnas is None:
                continue

            crudo = linea.split("\t")
            campos = crudo[:-1] + dividir_cola(crudo[-1])

            if len(campos) < len(columnas):
                campos += [""] * (len(columnas) - len(campos))
            elif len(campos) > len(columnas):
                extra = campos[len(columnas) - 1:]
                campos = campos[:len(columnas) - 1] + [" ".join(extra)]

            escritor.writerow(campos)
            filas_escritas += 1

    return filas_escritas


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", required=True)
    parser.add_argument("--salida", required=True)
    args = parser.parse_args()

    os.makedirs(args.salida, exist_ok=True)

    patron = os.path.join(args.entrada, "**", "conn.log.labeled")
    archivos = sorted(glob.glob(patron, recursive=True))

    if not archivos:
        print(f"No se encontraron archivos conn.log.labeled dentro de {args.entrada}")
        return

    print(f"Encontrados {len(archivos)} archivos conn.log.labeled")

    total_filas = 0
    for i, ruta in enumerate(archivos, start=1):
        nombre_captura = os.path.basename(os.path.dirname(os.path.dirname(ruta)))
        nombre_salida = f"{nombre_captura}.csv"
        ruta_salida = os.path.join(args.salida, nombre_salida)

        filas = procesar_archivo(ruta, ruta_salida)
        total_filas += filas
        print(f"[{i}/{len(archivos)}] {nombre_captura}: {filas} filas -> {ruta_salida}")

    print(f"\nListo. {total_filas} filas totales convertidas en {args.salida}")


if __name__ == "__main__":
    main()