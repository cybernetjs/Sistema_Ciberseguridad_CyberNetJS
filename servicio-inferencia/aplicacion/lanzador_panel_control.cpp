#include "lanzador_panel_control.h"

#include <cstdlib>
#include <filesystem>
#include <system_error>
#include <vector>

namespace sdi {

namespace fs = std::filesystem;

std::string derivar_ruta_metricas(const std::string& ruta_modelo) {
    fs::path modelo(ruta_modelo);
    std::string nombre_archivo = modelo.filename().string();

    const std::string sufijo_arboles = "_arboles.json";
    std::string nombre_metricas;
    if (nombre_archivo.size() >= sufijo_arboles.size() &&
        nombre_archivo.compare(nombre_archivo.size() - sufijo_arboles.size(), sufijo_arboles.size(),
                                sufijo_arboles) == 0) {
        nombre_metricas = nombre_archivo.substr(0, nombre_archivo.size() - sufijo_arboles.size()) + "_metricas.json";
    } else {
        nombre_metricas = modelo.stem().string() + "_metricas.json";
    }

    fs::path carpeta_modelo = modelo.has_parent_path() ? modelo.parent_path() : fs::path(".");

    // Diseno tipico del repositorio: modelos-arboles/modelo_x_arboles.json
    // junto a modelos-entrenados/modelo_x_metricas.json (carpetas hermanas).
    if (carpeta_modelo.filename() == "modelos-arboles") {
        fs::path candidata = carpeta_modelo.parent_path() / "modelos-entrenados" / nombre_metricas;
        if (fs::exists(candidata)) {
            return candidata.string();
        }
    }

    fs::path misma_carpeta = carpeta_modelo / nombre_metricas;
    if (fs::exists(misma_carpeta)) {
        return misma_carpeta.string();
    }

    // No se encontro el archivo: se devuelve la ruta mas probable de todas
    // formas (misma carpeta que el modelo). panel-control simplemente
    // esperara a que el archivo aparezca.
    return misma_carpeta.string();
}

#ifdef _WIN32

#include <windows.h>

namespace {

std::string obtener_carpeta_ejecutable() {
    char buffer[MAX_PATH]{};
    DWORD longitud = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (longitud == 0 || longitud == MAX_PATH) {
        return ".";
    }
    return fs::path(buffer).parent_path().string();
}

// Rutas candidatas, en orden de preferencia, donde puede estar PanelControl.exe.
// Cubre tanto el despliegue final (el .exe del panel copiado junto al de
// servicio_inferencia, con o sin subcarpeta) como la carpeta de compilacion
// de desarrollo (panel-control/PanelControl/bin/...).
std::vector<fs::path> rutas_candidatas_panel(const fs::path& carpeta_exe) {
    return {
        carpeta_exe / "PanelControl.exe",
        carpeta_exe / "PanelControl" / "PanelControl.exe",
        carpeta_exe / "panel-control" / "PanelControl.exe",
        carpeta_exe / ".." / "panel-control" / "PanelControl" / "bin" / "x64" / "Debug" /
            "net8.0-windows10.0.19041.0" / "PanelControl.exe",
        carpeta_exe / ".." / ".." / "panel-control" / "PanelControl" / "bin" / "x64" / "Debug" /
            "net8.0-windows10.0.19041.0" / "PanelControl.exe",
    };
}

std::string ruta_absoluta_o_igual(const std::string& ruta) {
    std::error_code ec;
    fs::path absoluta = fs::absolute(fs::path(ruta), ec);
    return ec ? ruta : absoluta.string();
}

}  // namespace

bool lanzar_panel_control(const std::string& ruta_csv, const std::string& ruta_metricas,
                           const std::string& ruta_log, std::string* motivo_si_fallo) {
    std::string carpeta_exe_str = obtener_carpeta_ejecutable();
    fs::path carpeta_exe(carpeta_exe_str);

    // Permite forzar la ubicacion exacta sin tocar el codigo, por ejemplo si
    // PanelControl.exe vive en una carpeta que no esta entre las candidatas.
    fs::path ruta_panel;
    if (const char* forzada = std::getenv("SDI_RUTA_PANEL_CONTROL")) {
        ruta_panel = forzada;
    } else {
        for (const auto& candidata : rutas_candidatas_panel(carpeta_exe)) {
            if (fs::exists(candidata)) {
                ruta_panel = candidata;
                break;
            }
        }
    }

    if (ruta_panel.empty() || !fs::exists(ruta_panel)) {
        if (motivo_si_fallo) {
            *motivo_si_fallo =
                "No se encontro PanelControl.exe junto a servicio_inferencia.exe. "
                "Copia PanelControl.exe (con sus archivos de publicacion) en la misma carpeta, "
                "o define la variable de entorno SDI_RUTA_PANEL_CONTROL con la ruta exacta.";
        }
        return false;
    }

    // El panel lee estas variables de entorno para saber que archivos seguir
    // (ver panel-control/PanelControl/MainWindow.xaml.cs). Se pasan como
    // rutas absolutas porque el panel puede arrancar con otro directorio de
    // trabajo.
    SetEnvironmentVariableA("PANEL_RUTA_CSV", ruta_absoluta_o_igual(ruta_csv).c_str());
    SetEnvironmentVariableA("PANEL_RUTA_METRICAS", ruta_absoluta_o_igual(ruta_metricas).c_str());
    SetEnvironmentVariableA("PANEL_RUTA_LOG", ruta_absoluta_o_igual(ruta_log).c_str());

    std::string linea_comando = "\"" + ruta_panel.string() + "\"";
    std::vector<char> buffer_comando(linea_comando.begin(), linea_comando.end());
    buffer_comando.push_back('\0');

    STARTUPINFOA info_inicio{};
    info_inicio.cb = sizeof(info_inicio);
    PROCESS_INFORMATION info_proceso{};

    std::string carpeta_panel = ruta_panel.parent_path().string();

    BOOL creado = CreateProcessA(
        ruta_panel.string().c_str(),   // aplicacion
        buffer_comando.data(),         // linea de comandos (modificable)
        nullptr, nullptr,              // atributos de seguridad
        FALSE,                         // no hace falta heredar handles; las variables de entorno
                                        // ya quedaron puestas con SetEnvironmentVariableA y se
                                        // heredan igual porque lpEnvironment es nullptr
        0,                             // sin banderas especiales
        nullptr,                       // hereda el entorno del proceso actual
        carpeta_panel.empty() ? nullptr : carpeta_panel.c_str(),
        &info_inicio, &info_proceso);

    if (!creado) {
        if (motivo_si_fallo) {
            *motivo_si_fallo = "CreateProcess fallo al intentar iniciar " + ruta_panel.string() +
                                " (codigo de error " + std::to_string(GetLastError()) + ").";
        }
        return false;
    }

    CloseHandle(info_proceso.hThread);
    CloseHandle(info_proceso.hProcess);
    return true;
}

#else

bool lanzar_panel_control(const std::string&, const std::string&, const std::string&, std::string* motivo_si_fallo) {
    if (motivo_si_fallo) {
        *motivo_si_fallo = "panel-control (PanelControl.exe) es una aplicacion WinUI 3 para Windows; "
                            "no se lanza automaticamente en esta plataforma.";
    }
    return false;
}

#endif

}  // namespace sdi
