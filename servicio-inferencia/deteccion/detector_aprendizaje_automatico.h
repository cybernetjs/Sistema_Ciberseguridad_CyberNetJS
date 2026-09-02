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

class DetectorAprendizajeAutomatico : public IClasificadorEventos {
public:
    bool cargar_modelo(const std::string& ruta_modelo);

    VeredictoClasificacion clasificar(const EventoRed& evento) override;
    std::string nombre() const override;

private:
    std::vector<double> construir_vector_caracteristicas(const EventoRed& evento) const;
    double evaluar_arbol(const ArbolXgboost& arbol, const std::vector<double>& caracteristicas) const;
    std::string construir_clave_flujo(const EventoRed& evento) const;

    std::atomic<bool> modelo_cargado_{false};
    std::string ruta_modelo_;

    std::vector<std::string> orden_caracteristicas_;
    std::vector<double> media_;
    std::vector<double> desviacion_;
    double sesgo_inicial_ = 0.0;
    double umbral_probabilidad_alerta_ = 0.999;
    long paquetes_minimos_alerta_ = 30;
    double pps_minimo_alerta_ = 150.0;
    double cooldown_alerta_segundos_ = 60.0;
    std::vector<ArbolXgboost> arboles_;
    std::unordered_map<std::string, double> ultima_alerta_por_flujo_;
};

}