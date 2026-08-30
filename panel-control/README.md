# panel-control (C# / WinUI 3)

Aplicacion de escritorio para Windows que muestra en tiempo real lo que
esta pasando en `servicio-inferencia`: no se conecta por socket, lee
directamente el CSV que genera `RegistradorCsv` (`eventos_procesados.csv`)
y el `metricas.json` que genera `entrenamiento-modelos`.

## Requisitos

- Windows 10 (1809+) o Windows 11
- Visual Studio 2022 con la carga de trabajo ".NET desktop development"
- El componente individual "Windows App SDK C# Templates" (Visual Studio
  Installer -> Componentes individuales -> buscar "Windows App SDK")

## Abrir y compilar

1. Abrir `PanelControl.sln` en Visual Studio.
2. Restaurar paquetes NuGet (se hace solo al abrir, o clic derecho en la
   solucion -> "Restore NuGet Packages").
3. Compilar y ejecutar (F5), plataforma `x64`.

## Como se conecta con el resto del sistema

El panel busca dos archivos (por defecto en la carpeta desde la que se
ejecuta):

- `eventos_procesados.csv` — el mismo archivo que genera
  `servicio_inferencia` (parametro `ruta_csv`).
- `modelo_iot23_metricas.json` — el que genera
  `entrenamiento-modelos/entrenar_xgboost.py`.
- `servicio.log` — el mismo texto que ves en la consola de
  `servicio_inferencia.exe` (INFO/ADVERTENCIA/ERROR), ahora tambien
  guardado en archivo (parametro `ruta_log`, cuarto argumento del exe).

Se pueden usar otras rutas sin tocar el codigo, definiendo variables de
entorno antes de abrir Visual Studio (o en las propiedades de depuracion
del proyecto, pestaña "Debug" -> "Environment variables"):

```
PANEL_RUTA_CSV=C:\ruta\a\eventos_procesados.csv
PANEL_RUTA_METRICAS=C:\ruta\a\modelo_iot23_metricas.json
PANEL_RUTA_LOG=C:\ruta\a\servicio.log
```

Para probarlo sin tener la Raspberry Pi conectada: corre
`servicio_inferencia.exe` en la misma PC (con o sin `sistema-captura`
real detras) y abre el panel al mismo tiempo apuntando al mismo CSV; cada
evento que se procese aparece en la lista en menos de un segundo.

## Estructura

```
panel-control/
├── PanelControl.sln
└── PanelControl/
    ├── App.xaml(.cs)
    ├── MainWindow.xaml(.cs)
    ├── app.manifest
    ├── PanelControl.csproj
    ├── Modelos/
    │   ├── RegistroEvento.cs        (una fila del CSV)
    │   └── MetricasModelo.cs        (el metricas.json)
    ├── Servicios/
    │   ├── LectorEventosCsv.cs      (sigue el CSV en tiempo real)
    │   ├── LectorArchivoTexto.cs    (sigue el servicio.log en tiempo real)
    │   └── LectorMetricas.cs        (relee el metricas.json cada 10s)
    └── ModelosVista/
        └── ModeloVistaPrincipal.cs  (estado que se ve en pantalla)
```

## Que muestra

- Punto verde/rojo + texto: si esta leyendo el CSV o no lo encuentra.
- Tarjetas: eventos procesados, alertas generadas, tiempo de respuesta
  promedio (ms), Accuracy, Precision, Recall, F1-Score (estos 4 ultimos
  vienen del `metricas.json`, no cambian evento a evento).
- Lista de los ultimos 200 eventos procesados (mas reciente arriba), con
  la fila resaltada en rojo cuando `es_amenaza = 1`.
- Un panel de consola/bitacora abajo, con las mismas lineas `[INFO]` /
  `[ADVERTENCIA]` / `[ERROR]` que se ven en la ventana de
  `servicio_inferencia.exe` (mas reciente arriba, hasta 500 lineas).

## Ejecutar servicio_inferencia.exe con log a archivo

El log a archivo es el cuarto argumento del ejecutable (el tercero es la
ruta al modelo):

```
servicio_inferencia.exe 9999 eventos_procesados.csv C:\ruta\modelo_iot23_arboles.json servicio.log
```

## Nota

Esto no se pudo compilar ni ejecutar en este entorno (no hay Windows ni
Visual Studio disponibles aqui): la sintaxis de C# y el XAML se
verificaron a mano, pero el primer build real hay que hacerlo en tu
maquina. Si sale algun error de compilacion, pega el mensaje completo y
lo corregimos.
