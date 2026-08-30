using System.Text;

namespace PanelControl.Servicios;

public sealed class LectorArchivoTexto
{
    private readonly string _rutaArchivo;
    private readonly TimeSpan _intervaloSondeo;
    private long _posicionLeida;
    private string _lineaParcial = "";

    public event Action<IReadOnlyList<string>>? LineasNuevas;

    public LectorArchivoTexto(string rutaArchivo, TimeSpan? intervaloSondeo = null)
    {
        _rutaArchivo = rutaArchivo;
        _intervaloSondeo = intervaloSondeo ?? TimeSpan.FromMilliseconds(500);
    }

    public async Task IniciarAsync(CancellationToken token)
    {
        while (!token.IsCancellationRequested)
        {
            if (File.Exists(_rutaArchivo))
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

        var nuevasLineas = new List<string>();
        for (int i = 0; i < cantidadLineasCompletas; i++)
        {
            string linea = lineas[i].TrimEnd('\r');
            if (linea.Length > 0)
            {
                nuevasLineas.Add(linea);
            }
        }

        if (nuevasLineas.Count > 0)
        {
            LineasNuevas?.Invoke(nuevasLineas);
        }
    }
}
