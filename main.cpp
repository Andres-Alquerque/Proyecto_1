#include <iostream>
#include <limits>
#include <cstdio>
#include <cstdint>
#include "funciones.h"
using namespace std;
static void limpiar(){
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}
bool escribir_bsf(const char* ruta){
    FILE* f = fopen(ruta, "wb");
    if(!f) return false;


    unsigned char sala_id = 1;
    unsigned char nmaq = 1;  // solo 1 máquina por ahora
    fwrite(&sala_id, 1, 1, f);
    fwrite(&nmaq, 1, 1, f);


    unsigned char maq_id = 10;
    uint16_t nmed = 1;  // solo 1 medición
    fwrite(&maq_id, 1, 1, f);
    fwrite(&nmed, 1, 2, f);


    unsigned char pid = 5;  // id paciente
    char fecha[32] = "31/08/2025 12:00:00";
    uint16_t nlect = 2; // dos lecturas
    fwrite(&pid, 1, 1, f);
    fwrite(fecha, 1, 32, f);
    fwrite(&nlect, 1, 2, f);


    unsigned char tipo1 = 'T';   // TS_T
    double temp = 37.2;
    fwrite(&tipo1, 1, 1, f);
    fwrite(&temp, 1, 8, f);


    unsigned char tipo2 = 'P';   // TS_P
    float sis = 120.0, dia = 80.0;
    fwrite(&tipo2, 1, 1, f);
    fwrite(&sis, 1, 4, f);
    fwrite(&dia, 1, 4, f);

    fclose(f);
    return true;
}
int main(){

    if(escribir_bsf("test.bsf"))
        cout << "Archivo test.bsf creado correctamente\n";
    else
        cout << "Error al crear test.bsf\n";
    //
    SalaUCI sala{}; sala.id=0; sala.nmaq=0; sala.maquinas=nullptr;
    ConfigData cfg{}; cfg.items=nullptr; cfg.count=0;
    PacientesData pacs{}; pacs.items=nullptr; pacs.count=0;


    const char* rutaConfig    = "configuracion.txt";
    const char* rutaPacientes = "pacientes.csv";
    const char* rutaBSF = "patient_readings_simulation_small 1.bsf";


    int op=-1;
    do{
        cout << "\n===== MENU =====\n";
        cout << "1) Cargar configuracion y pacientes (por defecto)\n";
        cout << "2) Leer archivo .bsf (ruta por defecto)\n";
        cout << "3) Generar reporte de anomalias (anomalias.txt)\n";
        cout << "4) Reporte por paciente (mediciones_paciente_ID.txt)\n";
        cout << "5) Exportar ECG anomalos (pacientes_ecg_anomalos.dat) y reporte por paciente\n";
        cout << "0) Salir\n> ";
        if(!(cin>>op)){ limpiar(); continue; }

        if(op==1){
            liberar_config(cfg); liberar_pacientes(pacs);
            if(!cargar_configuracion_txt(rutaConfig,cfg))
                cout << "No se pudo cargar " << rutaConfig << "\n";
            else
                cout << "Configuracion: " << cfg.count << " entradas.\n";

            if(!cargar_pacientes_txt(rutaPacientes,pacs))
                cout << "No se pudo cargar " << rutaPacientes << "\n";
            else
                cout << "Pacientes: " << pacs.count << "\n";

        } else if(op==2){
            liberar_sala(sala);
            if(!leer_bsf(rutaBSF, sala)){
                cout << "No se pudo leer " << rutaBSF << "\n";
            } else {
                cout << "Sala " << (int)sala.id << " con " << (int)sala.nmaq << " maquinas.\n";
                for(int i=0; i<sala.nmaq; i++){
                    MaquinaUCI& maq = sala.maquinas[i];
                    cout << "  Maquina " << (int)maq.id << " con " << maq.numMediciones << " mediciones.\n";

                    for(int j=0; j<maq.numMediciones; j++){
                        Medicion& med = maq.mediciones[j];
                        cout << "    Medicion " << j+1
                             << " | Paciente: " << med.idPaciente
                             << " | Fecha: " << med.fecha
                             << " | Lecturas: " << med.numLecturas << "\n";

                        for(int k=0; k<med.numLecturas; k++){
                            Lectura& lec = med.lecturas[k];
                            if(lec.tipo == 'P'){
                                cout << "       Lectura P: " << lec.p_sis << "/" << lec.p_dia << "\n";
                            } else {
                                cout << "       Lectura " << lec.tipo << ": " << lec.valor << "\n";
                            }
                        }
                    }
                }
            }
        }
        else if(op==3){
            if(!reporte_anomalias_global(sala, cfg, "anomalias.txt"))
                cout << "Fallo generando anomalias.txt\n";
            else
                cout << "anomalias.txt generado.\n";

        }  else if (op == 4) {
            int x;
            cout << "Id de paciente (1..255): ";
            cin >> x;

            char idPaciente[16];
            snprintf(idPaciente, sizeof(idPaciente), "%d", x);

            char rutaSal[128];
            snprintf(rutaSal, sizeof(rutaSal), "mediciones_paciente_%d.txt", x);

            try {
                reporte_mediciones_paciente(sala, cfg, idPaciente);
                cout << "Reporte generado: " << rutaSal << "\n";
            } catch (...) {
                cout << "Fallo reporte paciente.\n";
            }
        } else if (op == 5) {
            if (!exportar_pacientes_ecg_anomalos(sala, pacs, cfg, "pacientes_ecg_anomalos.dat"))
                cout << "Fallo exportando pacientes_ecg_anomalos.dat\n";
            else
                cout << "Export OK: pacientes_ecg_anomalos.dat\n";

            unsigned x;
            cout << "Ademas, id paciente para reporte: ";
            cin >> x;


            char idPaciente[16];
            snprintf(idPaciente, sizeof(idPaciente), "%u", x);


            char rutaSal[128];
            snprintf(rutaSal, sizeof(rutaSal), "mediciones_paciente_%u.txt", x);


            try {
                reporte_mediciones_paciente(sala, cfg, idPaciente);
                cout << "Reporte generado: " << rutaSal << "\n";
            } catch (...) {
                cout << "Fallo reporte paciente.\n";
            }
        }


    }while(op!=0);

    liberar_sala(sala);
    liberar_config(cfg);
    liberar_pacientes(pacs);
    return 0;
}
