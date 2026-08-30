using Microsoft.UI.Xaml;
using PanelControl.Modelos;
using PanelControl.ModelosVista;
using PanelControl.Servicios;

namespace PanelControl;

public sealed partial class MainWindow : Window
{
    public ModeloVistaPrincipal ModeloVista { get; } = new();

    private readonly CancellationTokenSource _cancelacion = new();
    private readonly LectorEventosCsv _lectorEventos;
    private readonly LectorMetricas _lectorMetricas;
    private readonly LectorArchivoTexto _lectorLog;

    public MainWindow()
    {
        InitializeComponent();

        string rutaCsv = ObtenerRutaConfigurable("PANEL_RUTA_CSV", "eventos_procesados.csv");
        string rutaMetricas = ObtenerRutaConfigurable("PANEL_RUTA_METRICAS", "modelo_iot23_metricas.json");
        string rutaLog = ObtenerRutaConfigurable("PANEL_RUTA_LOG", "servicio.log");

        _lectorEventos = new LectorEventosCsv(rutaCsv);
        _lectorEventos.EventosNuevos += OnEventosNuevos;
        _lectorEventos.CambioEstadoConexion += OnCambioEstadoConexion;

        _lectorMetricas = new LectorMetricas(rutaMetricas);

        _lectorLog = new LectorArchivoTexto(rutaLog);
        _lectorLog.LineasNuevas += OnLineasLogNuevas;

        Closed += (_, _) => _cancelacion.Cancel();

        _ = _lectorEventos.IniciarAsync(_cancelacion.Token);
        _ = _lectorLog.IniciarAsync(_cancelacion.Token);
        _ = CargarMetricasPeriodicamenteAsync(_cancelacion.Token);
    }

    private static string ObtenerRutaConfigurable(string variableEntorno, string valorPorDefecto)
    {
        string? valor = Environment.GetEnvironmentVariable(variableEntorno);
        return string.IsNullOrWhiteSpace(valor) ? valorPorDefecto : valor;
    }

    private void OnEventosNuevos(IReadOnlyList<RegistroEvento> nuevos)
    {
        DispatcherQueue.TryEnqueue(() => ModeloVista.AgregarEventos(nuevos));
    }

    private void OnCambioEstadoConexion(bool conectado)
    {
        DispatcherQueue.TryEnqueue(() =>
        {
            ModeloVista.Conectado = conectado;
            ModeloVista.TextoEstadoConexion = conectado ? "Conectado" : "Esperando eventos...";
        });
    }

    private void OnLineasLogNuevas(IReadOnlyList<string> nuevas)
    {
        DispatcherQueue.TryEnqueue(() => ModeloVista.AgregarLineasLog(nuevas));
    }

    private async Task CargarMetricasPeriodicamenteAsync(CancellationToken token)
    {
        while (!token.IsCancellationRequested)
        {
            var metricas = await _lectorMetricas.LeerAsync();
            if (metricas != null)
            {
                DispatcherQueue.TryEnqueue(() => ModeloVista.ActualizarMetricas(metricas));
            }

            try
            {
                await Task.Delay(TimeSpan.FromSeconds(10), token);
            }
            catch (TaskCanceledException)
            {
            }
        }
    }
}
