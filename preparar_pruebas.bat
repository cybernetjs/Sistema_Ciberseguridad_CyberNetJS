@echo off
set RUTA_BASE=C:\Users\JOSE\Documents\Sistema_Ciberseguridad_CyberNetJS
set RUTA_BUILD=%RUTA_BASE%\servicio-inferencia\out\build\x64-Debug
set RUTA_RESULTADOS=%RUTA_BASE%\resultados-tesis
set RUTA_ENTRENAMIENTO=%RUTA_BASE%\datos-entrenamiento

if not exist "%RUTA_RESULTADOS%" mkdir "%RUTA_RESULTADOS%"
if not exist "%RUTA_ENTRENAMIENTO%" mkdir "%RUTA_ENTRENAMIENTO%"

if exist "%RUTA_BUILD%\eventos_procesados.csv" (
    del "%RUTA_BUILD%\eventos_procesados.csv"
    echo se elimino
) else (
    echo No se encontro eventos_procesados.csv, no hay nada que mover
)

if not exist "%RUTA_ENTRENAMIENTO%\ventanas.csv" (
    echo inicio,fin,etiqueta> "%RUTA_ENTRENAMIENTO%\ventanas.csv"
    echo Se creo la plantilla ventanas.csv en %RUTA_ENTRENAMIENTO%
)

echo.
echo Carpeta para CSVs de resultados de tesis:
echo %RUTA_RESULTADOS%
echo.
echo Carpeta para CSVs de entrenamiento y su plantilla de ventanas:
echo %RUTA_ENTRENAMIENTO%
echo.
pause