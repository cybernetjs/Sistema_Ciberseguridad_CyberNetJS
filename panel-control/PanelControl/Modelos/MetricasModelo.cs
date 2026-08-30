using System.Text.Json.Serialization;

namespace PanelControl.Modelos;

public sealed class MetricasModelo
{
    [JsonPropertyName("accuracy")]
    public double Accuracy { get; set; }

    [JsonPropertyName("precision")]
    public double Precision { get; set; }

    [JsonPropertyName("recall")]
    public double Recall { get; set; }

    [JsonPropertyName("f1_score")]
    public double F1Score { get; set; }

    [JsonPropertyName("matriz_confusion")]
    public int[][] MatrizConfusion { get; set; } = Array.Empty<int[]>();

    [JsonPropertyName("tiempo_prediccion_ms_por_registro")]
    public double TiempoPrediccionMsPorRegistro { get; set; }

    [JsonPropertyName("clases")]
    public List<string> Clases { get; set; } = new();

    [JsonPropertyName("caracteristicas_seleccionadas")]
    public List<string> CaracteristicasSeleccionadas { get; set; } = new();

    [JsonPropertyName("total_registros_entrenamiento")]
    public int TotalRegistrosEntrenamiento { get; set; }

    [JsonPropertyName("total_registros_prueba")]
    public int TotalRegistrosPrueba { get; set; }
}
