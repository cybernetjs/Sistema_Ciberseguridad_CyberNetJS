# capture-engine (C / C++)

Módulo que corre en la Raspberry Pi 5 configurada como bridge transparente.

Responsabilidades:
- Poner la Raspberry Pi en modo bridge entre el router y los dispositivos IoT.
- Capturar el tráfico con `libpcap`.
- Extraer las características de cada paquete/flujo necesarias para el modelo
  (protocolo, tamaño, IPs, puertos, flags, tiempos entre paquetes, etc.).
- Enviar los datos extraídos al `inference-service` vía socket TCP al puerto 9999.

## Dependencias sugeridas
- `libpcap-dev`
- CMake >= 3.20
- Compilador con soporte C++17

## Estructura sugerida
```
capture-engine/
├── src/
│   ├── main.cpp
│   ├── packet_capture.cpp/.h
│   └── feature_extractor.cpp/.h
├── CMakeLists.txt
└── README.md
```
