using System.Globalization;
using System.Text;
using PanelControl.Modelos;

namespace PanelControl.Servicios;

public sealed class LectorEventosCsv
{
    private readonly string _rutaArchivo;
    private readonly TimeSpan _intervaloSondeo;
    private long _posicionLeida;
    private bool _encabezadoOmitido;
    private string _lineaParcial = "";

    public event Action<IReadOnlyList<RegistroEvento>>? EventosNuevos;
    public event Action<bool>? CambioEstadoConexion;

    public LectorEventosCsv(string rutaArchivo, TimeSpan? intervaloSondeo = null)
    {
        _rutaArchivo = rutaArchivo;
        _intervaloSondeo = intervaloSondeo ?? TimeSpan.FromMilliseconds(500);
    }

    public async Task IniciarAsync(CancellationToken token)
    {
        bool conectadoAnterior = false;

        while (!token.IsCancellationRequested)
        {
            bool existe = File.Exists(_rutaArchivo);

            if (existe != conectadoAnterior)
            {
                conectadoAnterior = existe;
                CambioEstadoConexion?.Invoke(existe);
            }

            if (existe)
            {
                try
                {
                    LeerNuevasLineas();
                }
                catch (IOException)
                {
                }
            }

            try
            {
                await Task.Delay(_intervaloSondeo, token);
            }
            catch (TaskCanceledException)
            {
            }
        }
    }

    private void LeerNuevasLineas()
    {
        using var flujo = new FileStream(_rutaArchivo, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);

        if (flujo.Length < _posicionLeida)
        {
            _posicionLeida = 0;
            _lineaParcial = "";
            _encabezadoOmitido = false;
        }

        if (flujo.Length <= _posicionLeida)
        {
            return;
        }

        flujo.Seek(_posicionLeida, SeekOrigin.Begin);
        using var lector = new StreamReader(flujo, Encoding.UTF8);
        string textoNuevo = lector.ReadToEnd();
        _posicionLeida = flujo.Position;

        string textoCompleto = _lineaParcial + textoNuevo;
        var lineas = textoCompleto.Split('\n');

        bool terminaEnSalto = textoCompleto.EndsWith("\n");
        _lineaParcial = terminaEnSalto ? "" : lineas[^1];
        int cantidadLineasCompletas = terminaEnSalto ? lineas.Length : lineas.Length - 1;

        var nuevosRegistros = new List<RegistroEvento>();
        for (int i = 0; i < cantidadLineasCompletas; i++)
        {
            string linea = lineas[i].TrimEnd('\r');
            if (linea.Length == 0)
            {
                continue;
            }

            if (!_encabezadoOmitido)
            {
                _encabezadoOmitido = true;
                continue;
            }

            var registro = ParsearLinea(linea);
            if (registro != null)
            {
                nuevosRegistros.Add(registro);
            }
        }

        if (nuevosRegistros.Count > 0)
        {
            EventosNuevos?.Invoke(nuevosRegistros);
        }
    }

    private static RegistroEvento? ParsearLinea(string linea)
    {
        var campos = DividirLineaCsv(linea);
        if (campos.Count < 26)
        {
            return null;
        }

        try
        {
            return new RegistroEvento
            {
                MarcaTiempoUnix = double.Parse(campos[0], CultureInfo.InvariantCulture),
                IpOrigen = campos[1],
                IpDestino = campos[2],
                PuertoDestino = int.Parse(campos[3], CultureInfo.InvariantCulture),
                Protocolo = int.Parse(campos[4], CultureInfo.InvariantCulture),
                Duracion = double.Parse(campos[5], CultureInfo.InvariantCulture),
                PaquetesOrigen = int.Parse(campos[6], CultureInfo.InvariantCulture),
                PaquetesDestino = int.Parse(campos[7], CultureInfo.InvariantCulture),
                BytesOrigen = int.Parse(campos[8], CultureInfo.InvariantCulture),
                BytesDestino = int.Parse(campos[9], CultureInfo.InvariantCulture),
                TasaTransferencia = double.Parse(campos[10], CultureInfo.InvariantCulture),
                TtlOrigen = int.Parse(campos[11], CultureInfo.InvariantCulture),
                TtlDestino = int.Parse(campos[12], CultureInfo.InvariantCulture),
                CargaOrigen = double.Parse(campos[13], CultureInfo.InvariantCulture),
                CargaDestino = double.Parse(campos[14], CultureInfo.InvariantCulture),
                IntervaloOrigen = double.Parse(campos[15], CultureInfo.InvariantCulture),
                IntervaloDestino = double.Parse(campos[16], CultureInfo.InvariantCulture),
                FluctuacionOrigen = double.Parse(campos[17], CultureInfo.InvariantCulture),
                FluctuacionDestino = double.Parse(campos[18], CultureInfo.InvariantCulture),
                ConteoServicioOrigen = int.Parse(campos[19], CultureInfo.InvariantCulture),
                ConteoDestinoReciente = int.Parse(campos[20], CultureInfo.InvariantCulture),
                Clasificador = campos[21],
                EsAmenaza = campos[22] == "1",
                Etiqueta = campos[23],
                Confianza = double.Parse(campos[24], CultureInfo.InvariantCulture),
                TiempoRespuestaMs = double.Parse(campos[25], CultureInfo.InvariantCulture),
            };
        }
        catch (FormatException)
        {
            return null;
        }
    }

    private static List<string> DividirLineaCsv(string linea)
    {
        var campos = new List<string>();
        var actual = new StringBuilder();
        bool dentroComillas = false;

        for (int i = 0; i < linea.Length; i++)
        {
            char c = linea[i];
            if (dentroComillas)
            {
                if (c == '"')
                {
                    if (i + 1 < linea.Length && linea[i + 1] == '"')
                    {
                        actual.Append('"');
                        i++;
                    }
                    else
                    {
                        dentroComillas = false;
                    }
                }
                else
                {
                    actual.Append(c);
                }
            }
            else
            {
                if (c == ',')
                {
                    campos.Add(actual.ToString());
                    actual.Clear();
                }
                else if (c == '"')
                {
                    dentroComillas = true;
                }
                else
                {
                    actual.Append(c);
                }
            }
        }
        campos.Add(actual.ToString());
        return campos;
    }
}
