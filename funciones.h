#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <cstddef>
#include "SalaUCI.h"
#include "ArchivoConfiguracion.h"
#include "ArchivoPacientes.h"

struct SalaUCI;

// --------- carga y liberación ----------
bool cargar_configuracion_txt(const char* ruta, ConfigData& cfg);
bool cargar_pacientes_txt(const char* ruta, PacientesData& pacs);
void liberar_config(ConfigData& cfg);
void liberar_pacientes(PacientesData& pacs);

// --------- BSF ----------
bool leer_bsf(const char* ruta, SalaUCI& sala);
void liberar_sala(SalaUCI& sala);

// --------- util ----------
bool cfg_get(const ConfigData& cfg, const char* clave, double& mn, double& mx);
bool parseFechaHora(const char* fechaHora, struct tm &tm_out);
bool compararFechas(const char* fecha1, const char* fecha2);

// --------- reportes / export ----------
bool reporte_anomalias_global(const SalaUCI &sala, const ConfigData &cfg, const char* rutaTxt);
void reporte_mediciones_paciente(const SalaUCI &sala, const ConfigData &cfg, const char* idPaciente);
bool exportar_pacientes_ecg_anomalos(const SalaUCI& sala, const PacientesData& pacs,
                                     const ConfigData& cfg, const char* rutaBin);

#endif
