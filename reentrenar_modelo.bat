@echo off
setlocal enabledelayedexpansion
set RAIZ=%~dp0
set EVENTOS=%1
set TIPO=%2
set VERSION=%3
if "%EVENTOS%"=="" goto uso
if "%TIPO%"=="" goto uso
if "%VERSION%"=="" goto uso
cd /d "%RAIZ%entrenamiento-modelos"
python etiquetar_capturas_propias.py --eventos "%EVENTOS%" --salida ..\datasets\crudo\entrenamiento_multiclase\propias_%TIPO%_%VERSION%.csv --tipo %TIPO%
if errorlevel 1 goto fin
python entrenar_xgboost.py --datos ../datasets/crudo/entrenamiento_multiclase --etiqueta tipo --k 15 --excluir "ts,orig_bytes,resp_bytes" --salida ../modelos-entrenados/modelo_%VERSION%.json
if errorlevel 1 goto fin
python exportar_arboles.py --modelo ../modelos-entrenados/modelo_%VERSION%.json --joblib ../modelos-entrenados/modelo_%VERSION%_preprocesamiento.joblib --salida ../modelos-arboles/modelo_%VERSION%_arboles.json
goto fin
:uso
echo uso: reentrenar_modelo.bat ruta_csv_eventos tipo_ataque version_modelo
:fin
endlocal