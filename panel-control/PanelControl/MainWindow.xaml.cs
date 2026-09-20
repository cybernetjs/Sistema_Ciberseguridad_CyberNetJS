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

    public MainWindow()
    {
        InitializeComponent();

        string rutaCsv = ObtenerRutaConfigurable("PANEL_RUTA_CSV", "eventos_procesados.csv");

        _lectorEventos = new LectorEventosCsv(rutaCsv);
        _lectorEventos.EventosNuevos += OnEventosNuevos;
        _lectorEventos.CambioEstadoConexion += OnCambioEstadoConexion;

        Closed += (_, _) => _cancelacion.Cancel();

        _ = _lectorEventos.IniciarAsync(_cancelacion.Token);
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
}