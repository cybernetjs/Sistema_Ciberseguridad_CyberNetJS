# inference-service (C++)

Servicio que corre en la Raspberry Pi 5 junto al `capture-engine`.

Responsabilidades:
- Levantar un servidor de sockets TCP en el **puerto 9999** que recibe los
  datos extraídos por `capture-engine`.
- Aplicar detección basada en firmas (patrones conocidos) antes del análisis ML.
- Cargar el modelo entrenado (`../models/model.onnx` o `.json`) y hacer inferencia
  con XGBoost C API o ONNX Runtime.
- Calcular métricas en tiempo real (tiempo de detección, tiempo de emisión de alerta).
- Emitir alertas (log, archivo, o hacia `dashboard-winui`).

## Dependencias sugeridas
- ONNX Runtime (C++ API) **o** libxgboost (C API)
- Boost.Asio o sockets POSIX nativos
- CMake >= 3.20

## Estructura sugerida
```
inference-service/
├── src/
│   ├── main.cpp
│   ├── tcp_server.cpp/.h
│   ├── signature_detector.cpp/.h
│   ├── model_inference.cpp/.h
│   └── alert_emitter.cpp/.h
├── CMakeLists.txt
└── README.md
```
