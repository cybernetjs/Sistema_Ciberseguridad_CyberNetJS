# servicio-inferencia (C++)

Corre en la laptop o servidor central. Escucha en el puerto 9999, recibe el
JSON que envia `agente-captura` desde la Raspberry Pi, y clasifica cada
evento con dos detectores en cadena: primero firmas (reglas conocidas),
despues el modelo de aprendizaje automatico.

## Arquitectura por capas

| Carpeta | Responsabilidad |
|---|---|
| `dominio/` | `IClasificadorEventos`, `VeredictoClasificacion`, `INotificadorAlertas`: contratos del servicio |
| `transporte/` | `ServidorTcp`: acepta la conexion y entrega lineas JSON completas. `decodificar_eventos()`: convierte cada linea en uno o varios `EventoRed` |
| `deteccion/` | `DetectorFirmas`: puertos de explotacion IoT conocidos y umbral de flooding por IP (DoS/DDoS). `DetectorAprendizajeAutomatico`: delega en un modelo de clasificacion entrenado (XGBoost/ONNX) |
| `notificacion/` | `NotificadorConsola`: emite la alerta (por ahora a consola) |
| `aplicacion/` | `CanalizadorEventos`: ejecuta los detectores en orden, mide el tiempo de respuesta (ms) y notifica al primero que marque amenaza |
| `punto_entrada/` | `principal.cpp`: composicion de las piezas |

`EventoRed`, `Bitacora` y `ControladorApagado` viven en `nucleo-compartido/`
porque tambien los usa `agente-captura`.

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
./build/servicio_inferencia [puerto=9999]
```

Debe correr antes de arrancar `agente_captura` en la Raspberry Pi (o en
cualquier momento, ya que `agente-captura` reintenta la conexion sola).
