# agente-captura (C++)

Corre en la Raspberry Pi 5, captura trafico real con `libpcap`, extrae
caracteristicas de red por paquete y las envia por TCP en JSON hacia el
`servicio-inferencia`, agrupando eventos en lotes con reconexion automatica.

## Arquitectura por capas

| Carpeta | Responsabilidad |
|---|---|
| `dominio/` | `RastreadorFlujos`: mide la duracion de cada flujo de red observado |
| `captura/` | `CapturadorPaquetes`: envoltorio sobre libpcap, sin logica de negocio |
| `extraccion/` | `extraer_caracteristicas()`: convierte un paquete crudo en un `EventoRed` |
| `transporte/` | `ITransmisorEventos`, `serializador_json`, `TransmisorJson`: serializacion y envio por TCP |
| `aplicacion/` | `LoteadorEventos`: agrupa eventos, decide cuando enviarlos, hilo de envio automatico |
| `punto_entrada/` | `principal.cpp`: composicion de las piezas, arranque de hilos |

`LoteadorEventos` depende de `ITransmisorEventos` (abstraccion), no de
`TransmisorJson` (implementacion concreta). Si mas adelante se necesita otro
transporte, se crea otra clase que implemente `ITransmisorEventos` sin tocar
`LoteadorEventos` ni `principal.cpp`.

`EventoRed`, `Bitacora` y `ControladorApagado` viven en `nucleo-compartido/`
porque tambien los usa `servicio-inferencia`.

## Dependencias

```bash
sudo apt install libpcap-dev cmake g++
```

`nlohmann/json` no requiere instalacion: viene vendorizado como un solo
header en `dependencias-externas/nlohmann/json.hpp`.

## Compilar

```bash
cd agente-captura
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## Ejecutar (requiere privilegios para captura raw)

```bash
sudo ./build/agente_captura <ip_destino> [puerto=9999]
```

Ejemplo:
```bash
sudo ./build/agente_captura 192.168.1.50
```

Detener con `Ctrl+C` (SIGINT) o `SIGTERM`: hace el envio final del lote
pendiente y cierra la conexion de forma ordenada.
