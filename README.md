# IDS/IPS basado en Aprendizaje Automático para redes domésticas con IoT

Tesis: *Sistema basado en Aprendizaje Automático para la detección de intrusiones en
redes domésticas que integran dispositivos IoT en hogares del Perú - 2026*
Universidad Nacional del Centro del Perú — Facultad de Ingeniería de Sistemas

## Arquitectura

```
[Dispositivos IoT] --- [Router Archer AX12] --- [Raspberry Pi 5 - modo bridge]
                                                        |
                                          captura de paquetes (libpcap)
                                                        |
                                        socket TCP :9999 (inference-service)
                                                        |
                                 detección por firmas + modelo XGBoost/ONNX
                                                        |
                                                    alerta -> dashboard-winui
```

## Estructura del repositorio

| Carpeta | Lenguaje | Contenido |
|---|---|---|
| `capture-engine/` | C / C++ | Captura de paquetes en la Raspberry Pi (libpcap), modo bridge |
| `ml-training/` | Python | Preprocesamiento, SMOTE, selección de características, entrenamiento XGBoost |
| `inference-service/` | C++ | Servidor TCP puerto 9999, detección por firmas + inferencia del modelo |
| `dashboard-winui/` | C# / WinUI | Aplicación de escritorio: monitoreo en tiempo real, métricas, alertas |
| `models/` | - | Modelos entrenados exportados (`.json` de XGBoost o `.onnx`) |
| `datasets/` | Python/scripts | Descarga y preparación de CICIoT2023 e IoT-23 (los datasets NO se versionan) |
| `docs/` | - | Plan de tesis, diagramas, resultados experimentales |

## Metodología (CRISP-DM)

1. Comprensión del problema — ver `docs/plan-tesis.pdf`
2. Comprensión de los datos — `datasets/`
3. Preparación de datos — `ml-training/preprocessing/`
4. Modelado — `ml-training/train_xgboost.py`
5. Evaluación — `docs/resultados/`
6. Despliegue — `capture-engine/` + `inference-service/` sobre Raspberry Pi 5

## Cómo empezar

Ver `docs/setup.md` para la puesta en marcha del entorno de desarrollo.

## Autores

- Huaynate Achachau, José Luis
- Araujo Champi, José Eduardo

Asesor: Dr. Maquera Quispe, Henry George
