# servicio-inferencia (C++)

Corre en la laptop o servidor central. Escucha en el puerto 9999, recibe el
JSON que envia `sistema-captura` desde la Raspberry Pi, y clasifica cada
evento con dos detectores en cadena: primero firmas (reglas conocidas),
despues el modelo de aprendizaje automatico.

## Arquitectura por capas

| Carpeta | Responsabilidad |
|---|---|
| `dominio/` | `IClasificadorEventos`, `VeredictoClasificacion`, `INotificadorAlertas`: contratos del servicio |
| `transporte/` | `ServidorTcp`: acepta la conexion y entrega lineas JSON completas. `decodificar_eventos()`: convierte cada linea en uno o varios `EventoRed` |
| `deteccion/` | `DetectorFirmas`: puertos de explotacion IoT conocidos y umbral de flooding por IP (DoS/DDoS). `DetectorAprendizajeAutomatico`: delega en un modelo de clasificacion entrenado (XGBoost/ONNX) |
| `notificacion/` | `NotificadorConsola`: emite la alerta (por ahora a consola) |
| `persistencia/` | `RegistradorCsv`: guarda en CSV **todo** evento procesado (amenaza o benigno), con sus caracteristicas, el veredicto y el tiempo de respuesta |
| `aplicacion/` | `CanalizadorEventos`: ejecuta los detectores en orden, mide el tiempo de respuesta (ms), registra el evento en CSV y notifica al primero que marque amenaza |
| `punto_entrada/` | `principal.cpp`: composicion de las piezas |

`EventoRed`, `Bitacora` y `ControladorApagado` viven en `nucleo-compartido/`
porque tambien los usa `sistema-captura`.

## Registro en CSV (trafico de laboratorio)

`RegistradorCsv` escribe una fila por cada evento que pasa por el
`CanalizadorEventos`, no solo los que generan alerta. Es el archivo con el
que se arma el "trafico capturado directamente en el laboratorio" que pide
el punto 11.4 del plan de tesis, y la fuente para calcular Accuracy,
Precision, Recall, F1-Score, matriz de confusion y tiempos de respuesta en
la Fase 5 de evaluacion (Tabla 2, operacionalizacion de variables).

Columnas, en este orden — primero las caracteristicas de entrada del
evento (las mismas que usa el clasificador), despues el veredicto, y por
ultimo los indicadores de tiempo:

```
marca_tiempo_unix, ip_origen, ip_destino, puerto_destino, protocolo,
duracion, paquetes_origen, paquetes_destino, bytes_origen, bytes_destino,
tasa_transferencia, ttl_origen, ttl_destino, carga_origen, carga_destino,
intervalo_origen, intervalo_destino, fluctuacion_origen, fluctuacion_destino,
conteo_servicio_origen, conteo_destino_reciente,
clasificador, es_amenaza, etiqueta, confianza, tiempo_respuesta_ms
```

El archivo se abre en modo anexado: si ya existe, se sigue escribiendo al
final sin repetir el encabezado (util para no perder datos entre corridas
del laboratorio).

## Estado del detector de aprendizaje automatico

Todavia no hay un modelo entrenado (`entrenamiento-modelos/` esta
pendiente), asi que `DetectorAprendizajeAutomatico::clasificar()` siempre
devuelve "no es amenaza". El canalizador ya esta listo para conectarlo:
cuando exista el modelo exportado en `modelos-entrenados/`, se reemplaza el
cuerpo de `DetectorAprendizajeAutomatico::clasificar()` para cargarlo y
hacer la inferencia real, sin tocar `CanalizadorEventos`, `ServidorTcp` ni
nada mas.

## Compilar

```bash
cd servicio-inferencia
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## Ejecutar

```bash
./build/servicio_inferencia [puerto=9999] [ruta_csv=eventos_procesados.csv] [ruta_modelo] [ruta_log] [ruta_metricas]
```

Debe correr antes de arrancar `sistema_captura` en la Raspberry Pi (o en
cualquier momento, ya que `sistema-captura` reintenta la conexion sola).

## Interfaz grafica automatica (Windows)

Al arrancar, `servicio_inferencia.exe` intenta lanzar tambien
`panel-control` (`aplicacion/lanzador_panel_control.h/.cpp`), la interfaz
grafica de `panel-control/`, como proceso hijo. Asi, con un solo comando:

```
servicio_inferencia.exe 9999 eventos_procesados.csv C:\ruta\modelo_iot23_arboles.json servicio.log
```

ya aparece la ventana de `PanelControl.exe` mostrando el estado del
sistema, sin necesidad de abrirla por separado. El lanzador:

1. Busca `PanelControl.exe` junto al propio `servicio_inferencia.exe`
   (tambien revisa `PanelControl\` y `panel-control\` como subcarpetas, y
   la carpeta de compilacion de desarrollo `panel-control/PanelControl/bin/...`).
   Para desplegar, lo mas simple es copiar `PanelControl.exe` (con todos los
   archivos que Visual Studio genera al publicarlo) en la misma carpeta que
   `servicio_inferencia.exe`.
2. Si quieres forzar una ubicacion distinta, define la variable de entorno
   `SDI_RUTA_PANEL_CONTROL` con la ruta completa a `PanelControl.exe` antes
   de ejecutar el servicio.
3. Le pasa al panel, por variables de entorno (`PANEL_RUTA_CSV`,
   `PANEL_RUTA_METRICAS`, `PANEL_RUTA_LOG`), las mismas rutas que recibio
   `servicio_inferencia.exe`, para que no haga falta configurarlas a mano.
   La ruta de metricas es el quinto argumento (opcional) del ejecutable; si
   no se indica, se deduce a partir de la ruta del modelo (ver
   `derivar_ruta_metricas` en `lanzador_panel_control.cpp`).
4. Si no encuentra `PanelControl.exe`, el servicio sigue funcionando igual
   por consola y deja una linea `[ADVERTENCIA]` en el log explicando el
   motivo (no es un error fatal).

Esto solo aplica en Windows; en Linux/macOS el servicio funciona igual pero
no intenta lanzar ninguna interfaz grafica (PanelControl es WinUI 3, exclusivo
de Windows).
