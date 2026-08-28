# modelos-entrenados

Modelos de clasificacion entrenados por `entrenamiento-modelos/`, listos
para que `servicio-inferencia` los cargue en produccion.

- `modelo.json` — modelo XGBoost en formato nativo
- `modelo.onnx` — mismo modelo exportado a ONNX (cargable con ONNX Runtime
  en `servicio-inferencia` o en `panel-control` si se necesitara)

Si algun modelo pesa mas de ~50MB, usar Git LFS o subirlo como *release*
del repositorio en vez de comitearlo directo.
