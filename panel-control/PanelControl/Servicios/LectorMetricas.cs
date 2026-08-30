using System.Text.Json;
using PanelControl.Modelos;

namespace PanelControl.Servicios;

public sealed class LectorMetricas
{
    private readonly string _rutaArchivo;

    public LectorMetricas(string rutaArchivo)
    {
        _rutaArchivo = rutaArchivo;
    }

    public async Task<MetricasModelo?> LeerAsync()
    {
        if (!File.Exists(_rutaArchivo))
        {
            return null;
        }

        try
        {
            await using var flujo = File.OpenRead(_rutaArchivo);
            return await JsonSerializer.DeserializeAsync<MetricasModelo>(flujo);
        }
        catch (JsonException)
        {
            return null;
        }
        catch (IOException)
        {
            return null;
        }
    }
}
