# panel-control (C# / WinUI 3)

Aplicacion de escritorio para Windows que monitorea el sistema de deteccion
de intrusiones en tiempo real.

Responsabilidades:
- Conectarse al `servicio-inferencia` (socket TCP, o idealmente un
  WebSocket/REST ligero expuesto sobre el mismo servicio) para recibir
  alertas y metricas.
- Mostrar en tiempo real: trafico analizado, alertas generadas, tipo de
  ataque detectado.
- Mostrar metricas de evaluacion: Accuracy, Precision, Recall, F1-Score.
- Mostrar tiempo de respuesta (ms) desde la deteccion hasta la alerta —
  variable dependiente clave de la tesis.

## Dependencias sugeridas
- .NET 8 / WinUI 3 (Windows App SDK)
- `System.Net.Sockets` para la conexion TCP
- `LiveCharts2` o `Microsoft.Toolkit.Uwp.UI.Controls` para graficos en tiempo real
- (opcional) `Microsoft.ML.OnnxRuntime` si se decide correr tambien
  inferencia local sobre el mismo panel, ademas de la Raspberry Pi

## Estructura sugerida
```
panel-control/
├── PanelControl.sln
├── PanelControl/
│   ├── App.xaml
│   ├── VentanaPrincipal.xaml
│   ├── Servicios/
│   │   └── ClienteAlertasTcp.cs
│   ├── ModelosVista/
│   └── Vistas/
└── README.md
```
