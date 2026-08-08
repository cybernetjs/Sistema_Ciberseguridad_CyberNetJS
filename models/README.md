# models

Modelos entrenados, listos para producción.

- `model.json` — modelo XGBoost en formato nativo (cargable con la XGBoost C API)
- `model.onnx` — mismo modelo exportado a ONNX (cargable con ONNX Runtime en
  `inference-service` (C++) o incluso en `dashboard-winui` (C#) si se necesitara)

Si algún modelo pesa más de ~50MB, usar Git LFS o subirlo como *release* del
repositorio en vez de comitearlo directo.
