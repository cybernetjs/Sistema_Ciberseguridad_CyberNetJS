using Microsoft.UI.Xaml.Media;
using Windows.UI;

namespace PanelControl.Modelos;

public sealed class RegistroEvento
{
    public double MarcaTiempoUnix { get; set; }
    public string IpOrigen { get; set; } = "";
    public string IpDestino { get; set; } = "";
    public int PuertoDestino { get; set; }
    public int Protocolo { get; set; }
    public double Duracion { get; set; }
    public int PaquetesOrigen { get; set; }
    public int PaquetesDestino { get; set; }
    public int BytesOrigen { get; set; }
    public int BytesDestino { get; set; }
    public double TasaTransferencia { get; set; }
    public int TtlOrigen { get; set; }
    public int TtlDestino { get; set; }
    public double CargaOrigen { get; set; }
    public double CargaDestino { get; set; }
    public double IntervaloOrigen { get; set; }
    public double IntervaloDestino { get; set; }
    public double FluctuacionOrigen { get; set; }
    public double FluctuacionDestino { get; set; }
    public int ConteoServicioOrigen { get; set; }
    public int ConteoDestinoReciente { get; set; }
    public string Clasificador { get; set; } = "";
    public bool EsAmenaza { get; set; }
    public string Etiqueta { get; set; } = "";
    public double Confianza { get; set; }
    public double TiempoRespuestaMs { get; set; }

    public string HoraTexto => DateTimeOffset
        .FromUnixTimeMilliseconds((long)(MarcaTiempoUnix * 1000))
        .LocalDateTime.ToString("HH:mm:ss.fff");

    public string ConfianzaTexto => $"{Confianza:0.0%}";
    public string TiempoRespuestaTexto => $"{TiempoRespuestaMs:0.###} ms";

    public SolidColorBrush ColorFondo => new(EsAmenaza ? Color.FromArgb(60, 220, 38, 38) : Colors.Transparent);
}
