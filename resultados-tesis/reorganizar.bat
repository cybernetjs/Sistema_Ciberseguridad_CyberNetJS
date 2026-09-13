@echo off
mkdir evidencia-bugs-corregidos
mkdir evidencia-bugs-corregidos\diagnostico-ia

del prueba01_ddos_saliente.csv
del prueba01_ddos_saliente_v2.csv
del prueba01_ddos_saliente_v3.csv
del prueba03_fuerza_bruta_rapido.csv
del prueba04_dos_dispositivo.csv
del prueba04_modelo_nuevo.csv
del prueba06_dga_v2.csv
del diagnostico-ia\prueba01_diagnostico_ia.csv
del diagnostico-ia\prueba04_diagnostico_ia.csv

move prueba03_fuerza_bruta_lento.csv evidencia-bugs-corregidos\
move prueba05_beaconing.csv evidencia-bugs-corregidos\

move diagnostico-ia\analizar_diagnostico_log.py evidencia-bugs-corregidos\diagnostico-ia\
move diagnostico-ia\benigno_video_v2.csv evidencia-bugs-corregidos\diagnostico-ia\
move diagnostico-ia\benigno_video_v3.csv evidencia-bugs-corregidos\diagnostico-ia\
move diagnostico-ia\benigno_video.csv evidencia-bugs-corregidos\diagnostico-ia\
move diagnostico-ia\diagnostico_finaldelulti.csv evidencia-bugs-corregidos\diagnostico-ia\
move diagnostico-ia\diagnostico_video.csv evidencia-bugs-corregidos\diagnostico-ia\
move diagnostico-ia\prueba01_umbral090.csv evidencia-bugs-corregidos\diagnostico-ia\
move diagnostico-ia\prueba01_umbral095.csv evidencia-bugs-corregidos\diagnostico-ia\
move diagnostico-ia\verificar_diagnostico_ia.py evidencia-bugs-corregidos\diagnostico-ia\

rmdir diagnostico-ia

echo LISTO
pause