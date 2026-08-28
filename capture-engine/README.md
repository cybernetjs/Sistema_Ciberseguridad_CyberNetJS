# capture-engine (C++)

Puerto en C++ de `TraficoDeRed_JSON.py`. Corre en la Raspberry Pi 5, captura trafico real con `libpcap`, extrae las mismas caracteristicas estilo UNSW-NB15 y las envia por TCP en JSON al NIDS central, con el mismo batching y logica de reconexion que la version en Python.

## Arquitectura (SOLID)

| Clase | Responsabilidad | Principio aplicado |
|---|---|---|
| `PacketCapture` | Captura de paquetes con libpcap | SRP: solo sabe de captura, nada de negocio |
| `FlowTracker` / `extract_event()` | Extraer un `NetworkEvent` de un paquete crudo | SRP: solo extraccion de caracteristicas |
| `ISender` | Interfaz abstracta: `send(batch)` / `close()` | DIP + LSP: cualquier implementacion es intercambiable |
| `JsonLineSender` | Implementa `ISender` sobre TCP + JSON (nlohmann/json) | SRP: solo serializacion y transporte |
| `EventBatcher` | Agrupa eventos, decide cuando hacer flush, hilo de flush automatico | SRP + OCP: tamano de batch e intervalo configurables por constructor, sin tocar la clase |
| `Logger` | Salida de consola thread-safe | SRP |
| `ShutdownSignal` | Manejo de SIGINT/SIGTERM | SRP |
| `main.cpp` | *Composition root*: crea las piezas, las conecta, arranca hilos | Sin logica de negocio propia |

`EventBatcher` depende de `ISender` (abstraccion), no de `JsonLineSender`
(concreto). Si mas adelante se necesita otro transporte (por ejemplo un
socket UDP, o escribir a un archivo para pruebas), se crea otra clase que
implemente `ISender` y no hay que tocar `EventBatcher` ni `main.cpp`.

## Equivalencia con el script Python original

| Python | C++ |
|---|---|
| `scapy.sniff(iface=None, filter=BPF, ...)` | `PacketCapture` sobre el dispositivo `"any"` |
| `pkt_to_event()` | `extract_event()` en `feature_extractor.cpp` |
| `last_seen = defaultdict(...)` | `FlowTracker` |
| `JsonLineSender` (clase Python) | `JsonLineSender` (implementa `ISender`) |
| `flush_batch()` / batching en `main()` | clase `EventBatcher` |
| `BATCH_SIZE = 25`, `FLUSH_SEC = 1.0` | parametros del constructor de `EventBatcher` en `main.cpp` |
| `PORT = 4444` | `kDefaultPort = 9999` en `main.cpp` (configurable por CLI) |

## Dependencias

```bash
sudo apt install libpcap-dev cmake g++
```

`nlohmann/json` **no requiere instalacion**: viene vendorizado como un solo
header en `third_party/nlohmann/json.hpp` (version 3.11.3), asi que no hace
falta `apt install` nada adicional para el JSON.

## Compilar

```bash
cd capture-engine
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## Ejecutar (requiere privilegios para captura raw)

```bash
sudo ./build/nids_capture <ip_de_la_laptop> [puerto=9999]
```

Ejemplo:
```bash
sudo ./build/nids_capture 192.168.1.50
```

Detener con `Ctrl+C` (SIGINT) o `SIGTERM`: hace flush final del batch
pendiente y cierra la conexion de forma ordenada.

## Estructura

```
capture-engine/
├── CMakeLists.txt
├── third_party/
│   └── nlohmann/json.hpp        # libreria JSON header-only, vendorizada
├── src/
│   ├── main.cpp                 # composition root
│   ├── feature_extractor.h/.cpp # NetworkEvent, FlowTracker, extract_event()
│   ├── packet_capture.h/.cpp    # wrapper de libpcap
│   ├── i_sender.h               # interfaz ISender
│   ├── json_sender.h/.cpp       # implementa ISender: JSON + TCP
│   ├── event_batcher.h/.cpp     # batching + flush automatico
│   ├── logger.h/.cpp            # logging thread-safe
│   └── shutdown_signal.h/.cpp   # manejo de SIGINT/SIGTERM
└── README.md
```
