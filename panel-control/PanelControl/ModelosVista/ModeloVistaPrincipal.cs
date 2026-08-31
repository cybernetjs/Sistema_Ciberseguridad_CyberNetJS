using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Microsoft.UI.Xaml.Media;
using PanelControl.Modelos;
using Windows.UI;

namespace PanelControl.ModelosVista;

public sealed class ModeloVistaPrincipal : INotifyPropertyChanged
{
    private const int MaximoEventosVisibles = 200;

    public event PropertyChangedEventHandler? PropertyChanged;

    public ObservableCollection<RegistroEvento> Eventos { get; } = new();
    public ObservableCollection<RegistroEvento> EventosFiltrados { get; } = new();
    public ObservableCollection<string> LineasLog { get; } = new();

    private bool _soloAmenazas;
    public bool SoloAmenazas
    {
        get => _soloAmenazas;
        set
        {
            if (Establecer(ref _soloAmenazas, value))
            {
                OnPropertyChanged(nameof(EventosVisibles));
            }
        }
    }

    public ObservableCollection<RegistroEvento> EventosVisibles => SoloAmenazas ? EventosFiltrados : Eventos;

    private const int MaximoLineasLogVisibles = 500;

    private int _totalEventos;
    public int TotalEventos
    {
        get => _totalEventos;
        set => Establecer(ref _totalEventos, value);
    }

    private int _totalAlertas;
    public int TotalAlertas
    {
        get => _totalAlertas;
        set => Establecer(ref _totalAlertas, value);
    }

    private double _sumaTiempoRespuestaMs;

    public string TiempoRespuestaPromedioTexto => TotalEventos == 0
        ? "-"
        : $"{(_sumaTiempoRespuestaMs / TotalEventos):0.###} ms";

    private string _textoEstadoConexion = "Conectando...";
    public string TextoEstadoConexion
    {
        get => _textoEstadoConexion;
        set => Establecer(ref _textoEstadoConexion, value);
    }

    private bool _conectado;
    public bool Conectado
    {
        get => _conectado;
        set
        {
            if (Establecer(ref _conectado, value))
            {
                OnPropertyChanged(nameof(ColorEstadoConexion));
            }
        }
    }

    public SolidColorBrush ColorEstadoConexion =>
        new(Conectado ? Color.FromArgb(255, 34, 197, 94) : Color.FromArgb(255, 220, 38, 38));

    private double _accuracy;
    public double Accuracy
    {
        get => _accuracy;
        set
        {
            if (Establecer(ref _accuracy, value))
            {
                OnPropertyChanged(nameof(AccuracyTexto));
            }
        }
    }
    public string AccuracyTexto => $"{Accuracy:0.0%}";

    private double _precision;
    public double Precision
    {
        get => _precision;
        set
        {
            if (Establecer(ref _precision, value))
            {
                OnPropertyChanged(nameof(PrecisionTexto));
            }
        }
    }
    public string PrecisionTexto => $"{Precision:0.0%}";

    private double _recall;
    public double Recall
    {
        get => _recall;
        set
        {
            if (Establecer(ref _recall, value))
            {
                OnPropertyChanged(nameof(RecallTexto));
            }
        }
    }
    public string RecallTexto => $"{Recall:0.0%}";

    private double _f1Score;
    public double F1Score
    {
        get => _f1Score;
        set
        {
            if (Establecer(ref _f1Score, value))
            {
                OnPropertyChanged(nameof(F1Texto));
            }
        }
    }
    public string F1Texto => $"{F1Score:0.0%}";

    public void AgregarEventos(IReadOnlyList<RegistroEvento> nuevos)
    {
        foreach (var evento in nuevos)
        {
            Eventos.Insert(0, evento);
            if (evento.EsAmenaza)
            {
                EventosFiltrados.Insert(0, evento);
                TotalAlertas++;
            }
            TotalEventos++;
            _sumaTiempoRespuestaMs += evento.TiempoRespuestaMs;
        }

        while (Eventos.Count > MaximoEventosVisibles)
        {
            Eventos.RemoveAt(Eventos.Count - 1);
        }

        while (EventosFiltrados.Count > MaximoEventosVisibles)
        {
            EventosFiltrados.RemoveAt(EventosFiltrados.Count - 1);
        }

        OnPropertyChanged(nameof(TiempoRespuestaPromedioTexto));
    }

    public void ActualizarMetricas(MetricasModelo metricas)
    {
        Accuracy = metricas.Accuracy;
        Precision = metricas.Precision;
        Recall = metricas.Recall;
        F1Score = metricas.F1Score;
    }

    public void AgregarLineasLog(IReadOnlyList<string> nuevas)
    {
        foreach (var linea in nuevas)
        {
            LineasLog.Insert(0, linea);
        }

        while (LineasLog.Count > MaximoLineasLogVisibles)
        {
            LineasLog.RemoveAt(LineasLog.Count - 1);
        }
    }

    private bool Establecer<T>(ref T campo, T valor, [CallerMemberName] string? nombrePropiedad = null)
    {
        if (EqualityComparer<T>.Default.Equals(campo, valor))
        {
            return false;
        }
        campo = valor;
        OnPropertyChanged(nombrePropiedad);
        return true;
    }

    private void OnPropertyChanged(string? nombrePropiedad)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nombrePropiedad));
    }
}
