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
./build/servicio_inferencia [puerto=9999] [ruta_csv=eventos_procesados.csv]
```

Debe correr antes de arrancar `sistema_captura` en la Raspberry Pi (o en
cualquier momento, ya que `sistema-captura` reintenta la conexion sola).
