#pragma once

#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>

#include "interfaz_clasificador_eventos.h"

namespace sdi {

struct NodoArbol {
    bool es_hoja = false;
    double valor_hoja = 0.0;
    int indice_caracteristica = -1;
    double condicion = 0.0;
    int id_si = -1;
    int id_no = -1;
};

using ArbolXgboost = std::unordered_map<int, NodoArbol>;

struct DiagnosticoIA {
    bool evaluado = false;
    double probabilidad = 0.0;
    bool es_amenaza = false;
    std::string tipo_predicho = "benigno";
};

class DetectorAprendizajeAutomatico : public IClasificadorEventos {
public:
    bool cargar_modelo(const std::string& ruta_modelo);

    VeredictoClasificacion clasificar(const EventoRed& evento) override;
    std::string nombre() const override;

    DiagnosticoIA diagnosticar(const EventoRed& evento) const;

private:
    struct ResultadoModelo {
        std::string clase_predicha = "benigno";
        double probabilidad_clase_predicha = 0.0;
    };

    ResultadoModelo evaluar_modelo(const EventoRed& evento) const;
    std::vector<double> construir_vector_caracteristicas(const EventoRed& evento) const;
    double evaluar_arbol(const ArbolXgboost& arbol, const std::vector<double>& caracteristicas) const;
    std::string construir_clave_flujo(const EventoRed& evento) const;
    bool clase_requiere_gate_volumen(const std::string& clase) const;
    double umbral_aplicable_para_clase(const std::string& clase) const;
    bool cumple_gate_volumen(const EventoRed& evento) const;

    std::atomic<bool> modelo_cargado_{false};
    std::string ruta_modelo_;

    std::vector<std::string> orden_caracteristicas_;
    std::vector<double> media_;
    std::vector<double> desviacion_;
    std::vector<double> sesgo_inicial_;
    int num_clases_ = 1;
    std::vector<std::string> nombres_clases_;

    double umbral_probabilidad_alerta_ = 0.90;
    double umbral_probabilidad_alerta_volumen_ = 0.55;
    long paquetes_minimos_alerta_ = 30;
    double pps_minimo_alerta_ = 150.0;
    long conexiones_mismo_destino_minimas_ = 50;
    long conexiones_origen_minimas_ = 20;
    double cooldown_alerta_segundos_ = 60.0;
    std::vector<ArbolXgboost> arboles_;
    std::unordered_map<std::string, double> ultima_alerta_por_flujo_;
};

}