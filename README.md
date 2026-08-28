# IDS/IPS basado en Aprendizaje Automatico para redes domesticas con IoT

Tesis: *Sistema basado en Aprendizaje Automatico para la deteccion de intrusiones en
redes domesticas que integran dispositivos IoT en hogares del Peru - 2026*
Universidad Nacional del Centro del Peru — Facultad de Ingenieria de Sistemas

## Arquitectura general

```
[Dispositivos IoT] --- [Router Archer AX12] --- [Raspberry Pi 5 - modo bridge]
                                                        |
                                          agente-captura (libpcap)
                                                        |
                                        socket TCP :9999 (servicio-inferencia)
                                                        |
                                 deteccion por firmas + modelo de clasificacion entrenado
                                                        |
                                                    alerta -> panel-control
```

## Estructura del repositorio

| Carpeta | Lenguaje | Contenido |
|---|---|---|
| `agente-captura/` | C / C++ | Captura de trafico real en la Raspberry Pi (libpcap), modo bridge |
| `servicio-inferencia/` | C++ | Servidor TCP puerto 9999, deteccion por firmas + inferencia del modelo |
| `nucleo-compartido/` | C / C++ | Entidad de dominio, bitacora, control de apagado y compatibilidad de sockets usados por ambos ejecutables |
| `dependencias-externas/` | C++ | Libreria JSON de terceros (nlohmann/json), vendorizada como header-only |
| `entrenamiento-modelos/` | Python | Preprocesamiento, SMOTE, seleccion de caracteristicas, entrenamiento del modelo de clasificacion |
| `servicio-inferencia/` → `modelos-entrenados/` | - | Modelos de clasificacion entrenados y exportados (formato XGBoost nativo u ONNX) |
| `datasets/` | Python/scripts | Descarga y preparacion de CICIoT2023 e IoT-23 (los datasets NO se versionan) |
| `panel-control/` | C# / WinUI | Aplicacion de escritorio: monitoreo en tiempo real, metricas, alertas |
| `documentacion/` | - | Guia de instalacion, plan de tesis, resultados experimentales |

## Metodologia (CRISP-DM)

1. Comprension del problema — ver `documentacion/plan-tesis.pdf`
2. Comprension de los datos — `datasets/`
3. Preparacion de datos — `entrenamiento-modelos/preprocesamiento/`
4. Modelado — `entrenamiento-modelos/entrenar_xgboost.py`
5. Evaluacion — `documentacion/resultados/`
6. Despliegue — `agente-captura/` + `servicio-inferencia/` sobre Raspberry Pi 5

## Como empezar

Ver `documentacion/guia-instalacion.md` para la puesta en marcha del entorno de desarrollo.

## Autores

- Huaynate Achachau, Jose Luis
- Araujo Champi, Jose Eduardo

Asesor: Dr. Maquera Quispe, Henry George
