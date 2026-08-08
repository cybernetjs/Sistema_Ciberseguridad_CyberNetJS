# Puesta en marcha del entorno

## 1. Requisitos previos

| Componente | Herramientas |
|---|---|
| `ml-training` | Python 3.10+, `pip install -r requirements.txt` |
| `capture-engine` / `inference-service` | GCC/Clang con C++17, CMake, `libpcap-dev` |
| `dashboard-winui` | Windows 11, Visual Studio 2022 con carga de trabajo "Windows App SDK" |
| Raspberry Pi 5 | Raspberry Pi OS (64-bit), configurada en modo bridge |

## 2. Clonar y explorar

```bash
git clone <url-del-repo>
cd ids-iot-hogar
```

## 3. Orden de trabajo recomendado (según CRISP-DM del plan de tesis)

1. `datasets/` → descargar y explorar CICIoT2023 e IoT-23
2. `ml-training/` → preprocesar, aplicar SMOTE, entrenar XGBoost, exportar a `models/`
3. `inference-service/` → prototipo del servidor TCP puerto 9999 + carga del modelo
4. `capture-engine/` → captura real de paquetes en la Raspberry Pi (modo bridge)
5. `dashboard-winui/` → cliente de monitoreo conectado al `inference-service`

## 4. Configuración de la Raspberry Pi 5 en modo bridge (referencia rápida)

```bash
sudo apt install bridge-utils
sudo brctl addbr br0
sudo brctl addif br0 eth0 eth1
sudo ip link set br0 up
```

(Ajustar interfaces según el hardware real: una hacia el router Archer AX12
y otra hacia la red de dispositivos IoT).
