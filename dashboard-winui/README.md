# dashboard-winui (C# / WinUI 3)

Aplicación de escritorio para Windows que monitorea el sistema IDS en tiempo real.

Responsabilidades:
- Conectarse al `inference-service` (socket TCP, o idealmente un WebSocket/REST
  ligero expuesto sobre el mismo servicio) para recibir alertas y métricas.
- Mostrar en tiempo real: tráfico analizado, alertas generadas, tipo de ataque detectado.
- Mostrar métricas de evaluación: Accuracy, Precision, Recall, F1-Score.
- Mostrar tiempo de respuesta (ms) desde la detección hasta la alerta —
  variable dependiente clave de la tesis.

## Dependencias sugeridas
- .NET 8 / WinUI 3 (Windows App SDK)
- `System.Net.Sockets` para la conexión TCP
- `LiveCharts2` o `Microsoft.Toolkit.Uwp.UI.Controls` para gráficos en tiempo real
- (opcional) `Microsoft.ML.OnnxRuntime` si se decide correr también inferencia
  local sobre el mismo dashboard, además de la Raspberry Pi

## Estructura sugerida
```
dashboard-winui/
├── IdsDashboard.sln
├── IdsDashboard/
│   ├── App.xaml
│   ├── MainWindow.xaml
│   ├── Services/
│   │   └── TcpAlertClient.cs
│   ├── ViewModels/
│   └── Views/
└── README.md
```
