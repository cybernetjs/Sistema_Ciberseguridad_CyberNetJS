# Puesta en marcha del entorno

## 1. Requisitos previos

| Componente | Herramientas |
|---|---|
| `entrenamiento-modelos` | Python 3.10+, `pip install -r requirements.txt` |
| `agente-captura` / `servicio-inferencia` | GCC/Clang con C++17, CMake, `libpcap-dev` |
| `panel-control` | Windows 11, Visual Studio 2022 con carga de trabajo "Windows App SDK" |
| Raspberry Pi 5 | Raspberry Pi OS (64-bit), configurada en modo bridge |

## 2. Clonar y explorar

```bash
git clone <url-del-repositorio>
cd sistema-deteccion-intrusiones-iot
```

## 3. Orden de trabajo recomendado (segun CRISP-DM del plan de tesis)

1. `datasets/` → descargar y explorar CICIoT2023 e IoT-23
2. `entrenamiento-modelos/` → preprocesar, aplicar SMOTE, entrenar el modelo de clasificacion, exportar a `modelos-entrenados/`
3. `servicio-inferencia/` → servidor TCP puerto 9999 + carga del modelo entrenado
4. `agente-captura/` → captura real de paquetes en la Raspberry Pi (modo bridge)
5. `panel-control/` → cliente de monitoreo conectado al `servicio-inferencia`

## 4. Configuracion de la Raspberry Pi 5 en modo bridge (referencia rapida)

```bash
sudo apt install bridge-utils
sudo brctl addbr br0
sudo brctl addif br0 eth0 eth1
sudo ip link set br0 up
```

Ajustar interfaces segun el hardware real: una hacia el router Archer AX12
y otra hacia la red de dispositivos IoT.
