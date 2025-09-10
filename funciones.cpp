#include "funciones.h"
#include <cstring>
#include <cstdlib>
#include <string>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <cstdint>
#include <cctype>
#include <ctime>
using namespace std;

bool parseFechaHora(const char* fechaHora, struct tm &tm_out) {
    int dd, MM, yyyy, hh, mm, ss;
    if (sscanf(fechaHora, "%d/%d/%d %d:%d:%d", &dd, &MM, &yyyy, &hh, &mm, &ss) == 6) {
        tm_out.tm_mday = dd;
        tm_out.tm_mon  = MM - 1;
        tm_out.tm_year = yyyy - 1900;
        tm_out.tm_hour = hh;
        tm_out.tm_min  = mm;
        tm_out.tm_sec  = ss;
        return true;
    }
    return false;
}

bool compararFechas(const char* fecha1, const char* fecha2) {
    struct tm tm1{}, tm2{};
    if (!parseFechaHora(fecha1, tm1) || !parseFechaHora(fecha2, tm2)) {
        return false;
    }
    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);
    return difftime(t1, t2) > 0;
}
static void trim(std::string& s){
    size_t a = 0;
    const size_t n = s.size();
    while (a < n) {
        unsigned char c = static_cast<unsigned char>(s[a]);
        if (c==' ' || c=='\t' || c=='\r' || c=='\n') ++a; else break;
    }
    size_t b = n;
    while (b > a) {
        unsigned char c = static_cast<unsigned char>(s[b-1]);
        if (c==' ' || c=='\t' || c=='\r' || c=='\n') --b; else break;
    }
    if (a > 0 || b < n) s = s.substr(a, b - a);
}
static bool rd_u32(FILE* f, uint32_t& v){
    unsigned char b[4];
    if(fread(b,1,4,f)!=4) return false;
    v = (uint32_t)b[0] | ((uint32_t)b[1]<<8) | ((uint32_t)b[2]<<16) | ((uint32_t)b[3]<<24);
    return true;
}
static bool rd_u64(FILE* f, uint64_t& v){
    unsigned char b[8];
    if(fread(b,1,8,f)!=8) return false;
    v = 0;
    for(int i=0;i<8;++i) v |= ((uint64_t)b[i]) << (8*i);
    return true;
}

static bool split_line(const char* line, char sep, char*** tokens, int& n){
    *tokens=nullptr; n=0;
    if(!line) return false;
    string s(line);
    while(!s.empty() && (s.back()=='\n' || s.back()=='\r')) s.pop_back();
    int parts=1; for(char c: s) if(c==sep) parts++;
    char** arr=(char**)malloc(parts*sizeof(char*));
    if(!arr) return false;

    int idx=0;
    size_t start=0;
    for(size_t i=0;i<=s.size();++i){
        if(i==s.size() || s[i]==sep){
            string tok = s.substr(start, i-start);
            arr[idx]=(char*)malloc(tok.size()+1);
            memcpy(arr[idx], tok.c_str(), tok.size()+1);
            idx++; start=i+1;
        }
    }
    *tokens=arr; n=idx; return true;
}
static void free_tokens(char** t, int n){
    if(!t) return; for(int i=0;i<n;++i) free(t[i]); free(t);
}



bool cargar_configuracion_txt(const char* ruta, ConfigData& cfg){
    cfg.items=nullptr; cfg.count=0;
    ifstream in(ruta);
    if(!in){
        cerr << "ERROR: no pude abrir configuracion: " << ruta << "\n";
        return false;
    }

    

    string line;
    int nValidas = 0;


    streampos inicio = in.tellg();
    bool first = true;
    while(getline(in,line)){
        if(first){
            if(line.size()>=3 && (unsigned char)line[0]==0xEF && (unsigned char)line[1]==0xBB && (unsigned char)line[2]==0xBF)
                line.erase(0,3);
            first=false;
        }
        trim(line);
        if(line.empty() || line[0]=='#') continue;
        nValidas++;
    }

    if(nValidas==0){ in.close(); return false; }
    cfg.items = new Configuracion[nValidas];
    cfg.count = 0;


    in.clear();
    in.seekg(inicio);
    first = true;
    while(getline(in,line)){
        if(first){
            if(line.size()>=3 && (unsigned char)line[0]==0xEF && (unsigned char)line[1]==0xBB && (unsigned char)line[2]==0xBF)
                line.erase(0,3);
            first=false;
        }
        trim(line);
        if(line.empty() || line[0]=='#') continue;

        for(char& c: line) if(c==',') c='.';

        Configuracion cfgitem{};
        bool parsed = false;

        // ---- parser con ';' ----
        size_t p1=line.find(';');
        if(p1!=string::npos){
            size_t p2=line.find(';', p1+1);
            if(p2!=string::npos){
                string t=line.substr(0,p1);
                string s1=line.substr(p1+1,p2-(p1+1));
                string s2=line.substr(p2+1);
                trim(t); trim(s1); trim(s2);
                if(!t.empty() && !s1.empty() && !s2.empty()){
                    strncpy(cfgitem.tipo,t.c_str(),sizeof(cfgitem.tipo)-1);
                    cfgitem.minVal=atof(s1.c_str());
                    cfgitem.maxVal=atof(s2.c_str());
                    parsed=true;
                }
            }
        }


        if(!parsed){
            size_t p0=line.find('.');
            if(p0!=string::npos){
                string tipo=line.substr(0,p0);
                string rest=line.substr(p0+1);
                trim(tipo); trim(rest);
                if(!tipo.empty() && !rest.empty()){
                    // tokenizar por '.'
                    char* temp = strdup(rest.c_str());
                    char* token = strtok(temp,".");
                    char* parts[10]; int np=0;
                    while(token && np<10){
                        parts[np++] = token;
                        token=strtok(nullptr,".");
                    }
                    string s1,s2;
                    if(np==2){
                        s1=parts[0]; s2=parts[1];
                    }else if(np==4){
                        s1=string(parts[0])+"."+parts[1];
                        s2=string(parts[2])+"."+parts[3];
                    }else if(np>4){
                        s1=string(parts[np-4])+"."+parts[np-3];
                        s2=string(parts[np-2])+"."+parts[np-1];
                    }else if(np==3){
                        s1=parts[0];
                        s2=string(parts[1])+"."+parts[2];
                    }
                    free(temp);
                    strncpy(cfgitem.tipo,tipo.c_str(),sizeof(cfgitem.tipo)-1);
                    cfgitem.minVal=atof(s1.c_str());
                    cfgitem.maxVal=atof(s2.c_str());
                    parsed=true;
                }
            }
        }

        if(parsed){
            cfg.items[cfg.count++] = cfgitem;
        }else{
            cerr << "ADVERTENCIA: linea config invalida: " << line << "\n";
        }
    }

    in.close();
    return (cfg.count>0);
}

void liberar_config(ConfigData& cfg){
    delete[] cfg.items; cfg.items=nullptr; cfg.count=0;
}


bool cargar_pacientes_txt(const char* ruta, PacientesData& pacs){
    pacs.items=nullptr; pacs.count=0;
    ifstream in(ruta);
    if(!in) return false;

    string line; int count=0;
    while(getline(in,line)){
        if(line.empty()||line[0]=='#') continue; count++;
    }
    if(count==0){ in.close(); return false; }
    in.clear(); in.seekg(0);

    ArchivoPacientes* arr = new ArchivoPacientes[count];
    int idx=0;
    while(getline(in,line)){
        if(line.empty()||line[0]=='#') continue;
        char** toks; int n;
        if(!split_line(line.c_str(), ';', &toks, n)) continue;

        if(n>=6){
            arr[idx].idCSV = atoi(toks[0]);
            arr[idx].idBSF = (unsigned char)(arr[idx].idCSV & 0xFF);
            strncpy(arr[idx].tipoDoc,  toks[1], sizeof(arr[idx].tipoDoc)-1);
            strncpy(arr[idx].documento,toks[2], sizeof(arr[idx].documento)-1);
            strncpy(arr[idx].nombres,  toks[3], sizeof(arr[idx].nombres)-1);
            strncpy(arr[idx].apellidos,toks[4], sizeof(arr[idx].apellidos)-1);
            strncpy(arr[idx].fechaNac, toks[5], sizeof(arr[idx].fechaNac)-1);
            arr[idx].tipoDoc[3]=arr[idx].documento[15]=arr[idx].nombres[47]=arr[idx].apellidos[47]=arr[idx].fechaNac[10]='\0';
            idx++;
        }
        free_tokens(toks,n);
    }
    in.close();
    pacs.items=arr; pacs.count=idx;
    return idx>0;
}

void liberar_pacientes(PacientesData& pacs){
    delete[] pacs.items; pacs.items=nullptr; pacs.count=0;
}


static bool rd_u8 (FILE* f, unsigned char& v){ return fread(&v,1,1,f)==1; }
static bool rd_u16(FILE* f, uint16_t& v){
    unsigned char b[2];
    if(fread(b,1,2,f)!=2) return false;
    v = (uint16_t)b[0] | ((uint16_t)b[1]<<8);
    return true;
}
static bool rd_f32(FILE* f, float& v){ return fread(&v,1,4,f)==4; }
static bool rd_f64(FILE* f, double& v){ return fread(&v,1,8,f)==8; }
static bool rd_bytes(FILE* f, void* buf, size_t n){ return fread(buf,1,n,f)==n; }



static constexpr uint32_t MAX_MEDICIONES_POR_MAQUINA = 100000;
static constexpr uint32_t MAX_LECTURAS_POR_MEDICION = 100000;


static const bool DEBUG_BSF = true;

bool leer_bsf(const char* nombreArchivo, SalaUCI &sala) {
    std::ifstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "Error: no se pudo abrir " << nombreArchivo << "\n";
        return false;
    }


    uint8_t idSala = 0;
    uint8_t nmaq = 0;
    if (!archivo.read(reinterpret_cast<char*>(&idSala), sizeof(idSala))) {
        std::cerr << "Error leyendo idSala\n";
        return false;
    }
    if (!archivo.read(reinterpret_cast<char*>(&nmaq), sizeof(nmaq))) {
        std::cerr << "Error leyendo nmaq\n";
        return false;
    }

    sala.id = idSala;
    sala.nmaq = nmaq;

    if (DEBUG_BSF) std::cout << "DEBUG: Sala ID=" << (int)sala.id << " nmaq=" << (int)sala.nmaq << "\n";


    if (sala.nmaq > 0) {
        sala.maquinas = new MaquinaUCI[sala.nmaq];
    } else {
        sala.maquinas = nullptr;
    }


    for (int i = 0; i < sala.nmaq; ++i) {

        uint8_t maqId = 0;
        if (!archivo.read(reinterpret_cast<char*>(&maqId), sizeof(maqId))) {
            std::cerr << "Error leyendo id de maquina " << i << "\n";

            liberar_sala(sala);
            return false;
        }
        sala.maquinas[i].id = maqId;


        uint32_t numMed = 0;
        if (!archivo.read(reinterpret_cast<char*>(&numMed), sizeof(numMed))) {
            std::cerr << "Error leyendo numMediciones de maquina " << (int)maqId << "\n";
            liberar_sala(sala);
            return false;
        }


        if (numMed > MAX_MEDICIONES_POR_MAQUINA) {
            std::cerr << "Valor numMediciones demasiado grande (" << numMed
                      << ") en maquina " << (int)maqId << ". Posible archivo corrupto.\n";
            liberar_sala(sala);
            return false;
        }

        sala.maquinas[i].numMediciones = numMed;

        if (DEBUG_BSF) std::cout << "DEBUG: Maquina " << (int)maqId << " numMediciones=" << numMed << "\n";


        if (numMed > 0) {
            sala.maquinas[i].mediciones = new Medicion[numMed];
        } else {
            sala.maquinas[i].mediciones = nullptr;
        }


        for (uint32_t j = 0; j < numMed; ++j) {
            Medicion &med = sala.maquinas[i].mediciones[j];


            char tmpId[12]; // 11 bytes + 1 para '\0'
            if (!archivo.read(tmpId, 11)) {
                std::cerr << "Error leyendo idPaciente (maq " << (int)maqId << " med " << j+1 << ")\n";
                liberar_sala(sala);
                return false;
            }
            tmpId[11] = '\0';


            size_t idFieldSize = sizeof(med.idPaciente);
            size_t toCopy = std::min((size_t)11, idFieldSize);
            std::memcpy(med.idPaciente, tmpId, toCopy);

            med.idPaciente[idFieldSize - 1] = '\0';


            char tmpFecha[25]; // 24 + 1
            if (!archivo.read(tmpFecha, 24)) {
                std::cerr << "Error leyendo fecha (maq " << (int)maqId << " med " << j+1 << ")\n";
                liberar_sala(sala);
                return false;
            }
            tmpFecha[24] = '\0';
            size_t fechaFieldSize = sizeof(med.fecha);
            size_t toCopyF = std::min((size_t)24, fechaFieldSize);
            std::memcpy(med.fecha, tmpFecha, toCopyF);
            med.fecha[fechaFieldSize - 1] = '\0';


            uint32_t numLect = 0;
            if (!archivo.read(reinterpret_cast<char*>(&numLect), sizeof(numLect))) {
                std::cerr << "Error leyendo numLecturas (maq " << (int)maqId << " med " << j+1 << ")\n";
                liberar_sala(sala);
                return false;
            }

            if (numLect > MAX_LECTURAS_POR_MEDICION) {
                std::cerr << "Valor numLecturas demasiado grande (" << numLect
                          << ") en maq " << (int)maqId << " med " << j+1 << ". Archivo corrupto?\n";
                liberar_sala(sala);
                return false;
            }

            med.numLecturas = numLect;

            if (DEBUG_BSF) {
                std::cout << "DEBUG: Med " << j+1
                          << " | Paciente='" << med.idPaciente << "'"
                          << " | Fecha='" << med.fecha << "'"
                          << " | Lecturas=" << med.numLecturas << "\n";
            }


            if (numLect > 0) {
                med.lecturas = new Lectura[numLect];
            } else {
                med.lecturas = nullptr;
            }


            for (uint32_t k = 0; k < numLect; ++k) {
                med.lecturas[k].tipo = 0;
                med.lecturas[k].valor = 0.0;
                med.lecturas[k].p_sis = 0;
                med.lecturas[k].p_dia = 0;
            }


            for (uint32_t k = 0; k < numLect; ++k) {
                Lectura &lec = med.lecturas[k];


                char tipo = 0;
                if (!archivo.read(reinterpret_cast<char*>(&tipo), 1)) {
                    std::cerr << "Error leyendo tipo lectura (maq " << (int)maqId << " med " << j+1 << " lec " << k+1 << ")\n";
                    liberar_sala(sala);
                    return false;
                }
                lec.tipo = tipo;

                if (tipo == 'P') {

                    int32_t ps = 0, pd = 0;
                    if (!archivo.read(reinterpret_cast<char*>(&ps), sizeof(ps)) ||
                        !archivo.read(reinterpret_cast<char*>(&pd), sizeof(pd))) {
                        std::cerr << "Error leyendo P syst/dia (maq " << (int)maqId << " med " << j+1 << " lec " << k+1 << ")\n";
                        liberar_sala(sala);
                        return false;
                    }
                    lec.p_sis = static_cast<int>(ps);
                    lec.p_dia = static_cast<int>(pd);
                }
                else if (tipo == 'T' || tipo == 'E' || tipo == 'O') {
                    // double
                    double v = 0.0;
                    if (!archivo.read(reinterpret_cast<char*>(&v), sizeof(v))) {
                        std::cerr << "Error leyendo double (maq " << (int)maqId << " med " << j+1 << " lec " << k+1 << ")\n";
                        liberar_sala(sala);
                        return false;
                    }
                    lec.valor = v;
                }
                else {
                    std::cerr << "Tipo desconocido '" << tipo << "' en maq " << (int)maqId
                              << " med " << j+1 << " lec " << k+1 << ". Abortando.\n";
                    liberar_sala(sala);
                    return false;
                }

                if (DEBUG_BSF) {
                    if (lec.tipo == 'P')
                        std::cout << "  DEBUG Lectura P: " << lec.p_sis << "/" << lec.p_dia << "\n";
                    else
                        std::cout << "  DEBUG Lectura " << lec.tipo << ": " << lec.valor << "\n";
                }
            }
        }
    }

    archivo.close();
    return true;
}
void liberar_sala(SalaUCI& sala){
    if(!sala.maquinas){ sala.nmaq=0; return; }
    for(int i=0;i<sala.nmaq;++i){
        if(sala.maquinas[i].mediciones){
            for(int j=0;j<sala.maquinas[i].numMediciones;++j){
                delete[] sala.maquinas[i].mediciones[j].lecturas;
            }
            delete[] sala.maquinas[i].mediciones;
        }
    }
    delete[] sala.maquinas; sala.maquinas=nullptr; sala.nmaq=0; sala.id=0;
}


bool cfg_get(const ConfigData& cfg, const char* clave, double& mn, double& mx){
    for(int i=0;i<cfg.count;++i){
        if(strcmp(cfg.items[i].tipo, clave)==0){
            mn=cfg.items[i].minVal; mx=cfg.items[i].maxVal; return true;
        }
    }
    return false;
}


bool reporte_anomalias_global(const SalaUCI& sala, const ConfigData& cfg, const char* rutaTxt) {
    ofstream out(rutaTxt);
    if (!out.is_open()) {
        cout << "No se pudo abrir archivo de salida: " << rutaTxt << endl;
        return false;
    }


    double mnT, mxT, mnE, mxE, mnO, mxO, mnPS, mxPS, mnPD, mxPD;
    bool hasT  = cfg_get(cfg, "T", mnT, mxT);
    bool hasE  = cfg_get(cfg, "E", mnE, mxE);
    bool hasO  = cfg_get(cfg, "O", mnO, mxO);
    bool hasPS = cfg_get(cfg, "P_SIS", mnPS, mxPS);
    bool hasPD = cfg_get(cfg, "P_DIA", mnPD, mxPD);

    out << "==== REPORTE GLOBAL DE ANOMALIAS ====\n";

    for (int i = 0; i < sala.nmaq; ++i) {
        const MaquinaUCI& MQ = sala.maquinas[i];
        for (unsigned int j = 0; j < MQ.numMediciones; ++j) {
            const Medicion& M = MQ.mediciones[j];
            bool anomalia = false;

            out << "Paciente " << M.idPaciente
                << " | Fecha " << M.fecha << "\n";


            for (unsigned int k = 0; k < M.numLecturas; ++k) {
                const Lectura& L = M.lecturas[k];
                if (L.tipo == 'T' && hasT) {
                    if (L.valor < mnT || L.valor > mxT) {
                        out << "  Anomalia T: " << L.valor << "\n";
                        anomalia = true;
                    }
                }
                else if (L.tipo == 'O' && hasO) {
                    if (L.valor < mnO || L.valor > mxO) {
                        out << "  Anomalia O: " << L.valor << "\n";
                        anomalia = true;
                    }
                }
                else if (L.tipo == 'P' && hasPS && hasPD) {
                    if (L.p_sis < mnPS || L.p_sis > mxPS ||
                        L.p_dia < mnPD || L.p_dia > mxPD) {
                        out << "  Anomalia P: " << L.p_sis << "/" << L.p_dia << "\n";
                        anomalia = true;
                    }
                }
            }


            if (hasE) {
                double minVal =  1e300;
                double maxVal = -1e300;
                bool tieneECG = false;

                for (unsigned int k = 0; k < M.numLecturas; ++k) {
                    const Lectura& L = M.lecturas[k];
                    if (L.tipo == 'E') {
                        tieneECG = true;
                        if (L.valor < minVal) minVal = L.valor;
                        if (L.valor > maxVal) maxVal = L.valor;
                    }
                }

                if (tieneECG) {
                    double sumaAbs    = fabs(minVal) + fabs(maxVal);
                    double sumaAbsLim = fabs(mnE) + fabs(mxE);
                    if (sumaAbs > sumaAbsLim) {
                        out << "  Anomalia ECG: min=" << minVal
                            << " max=" << maxVal
                            << " (sumaAbs=" << sumaAbs
                            << " > limite=" << sumaAbsLim << ")\n";
                        anomalia = true;
                    }
                }
            }

            if (!anomalia) {
                out << "  Todas las lecturas dentro de rango\n";
            }

            out << "-------------------------\n";
        }
    }

    out.close();
    cout << "Archivo de anomalías generado en " << rutaTxt << endl;
    return true;
}


void reporte_mediciones_paciente(const SalaUCI &sala, const ConfigData &cfg, const char* idPaciente) {

    char filename[64];
    snprintf(filename, sizeof(filename), "mediciones_paciente_%s.txt", idPaciente);
    ofstream fout(filename);
    if (!fout.is_open()) {
        cout << "No se pudo abrir " << filename << endl;
        return;
    }

    fout << "Reporte de mediciones del paciente " << idPaciente << "\n";

    for (int i = 0; i < sala.nmaq; ++i) {
        for (unsigned int j = 0; j < sala.maquinas[i].numMediciones; ++j) {
            const Medicion& M = sala.maquinas[i].mediciones[j];
            if (strcmp(M.idPaciente, idPaciente) != 0) continue;

            fout << "Fecha: " << M.fecha << "\n";


            double sumT=0,minT=1e9,maxT=-1e9; int cntT=0;
            double sumO=0,minO=1e9,maxO=-1e9; int cntO=0;
            double sumPS=0,sumPD=0; int cntP=0;
            double minPS=1e9,maxPS=-1e9,minPD=1e9,maxPD=-1e9;


            for (unsigned int k=0; k<M.numLecturas; ++k) {
                const Lectura &L = M.lecturas[k];
                if (L.tipo=='T') {
                    double min=0,max=0;
                    cfg_get(cfg, "T", min, max);
                    bool anom = (L.valor<min || L.valor>max);
                    fout << "  Temp: " << L.valor << (anom ? " [ANOMALIA]\n":" [OK]\n");
                    sumT+=L.valor; if(L.valor<minT)minT=L.valor; if(L.valor>maxT)maxT=L.valor; cntT++;
                }
                else if (L.tipo=='O') {
                    double min=0,max=0;
                    cfg_get(cfg, "O", min, max);
                    bool anom = (L.valor<min || L.valor>max);
                    fout << "  Oxi: " << L.valor << (anom ? " [ANOMALIA]\n":" [OK]\n");
                    sumO+=L.valor; if(L.valor<minO)minO=L.valor; if(L.valor>maxO)maxO=L.valor; cntO++;
                }
                else if (L.tipo=='P') {
                    double minS=0,maxS=0,minD=0,maxD=0;
                    cfg_get(cfg, "P_SIS", minS, maxS);
                    cfg_get(cfg, "P_DIA", minD, maxD);
                    bool anom = (L.p_sis<minS||L.p_sis>maxS||L.p_dia<minD||L.p_dia>maxD);
                    fout << "  Presion: " << L.p_sis << "/" << L.p_dia << (anom ? " [ANOMALIA]\n":" [OK]\n");
                    sumPS+=L.p_sis; sumPD+=L.p_dia; cntP++;
                    if(L.p_sis<minPS)minPS=L.p_sis; if(L.p_sis>maxPS)maxPS=L.p_sis;
                    if(L.p_dia<minPD)minPD=L.p_dia; if(L.p_dia>maxPD)maxPD=L.p_dia;
                }
                else if (L.tipo=='E') {
                    fout << "  ECG: " << L.valor << "\n";
                }
            }


            if(cntT) fout << " Temp[min="<<minT<<", max="<<maxT<<", prom="<<(sumT/cntT)<<"]\n";
            if(cntO) fout << " Oxi[min="<<minO<<", max="<<maxO<<", prom="<<(sumO/cntO)<<"]\n";
            if(cntP) fout << " P.sis[min="<<minPS<<", max="<<maxPS<<", prom="<<(sumPS/cntP)<<"], "
                          << "P.dia[min="<<minPD<<", max="<<maxPD<<", prom="<<(sumPD/cntP)<<"]\n";


            double minVal=1e9,maxVal=-1e9; bool tieneECG=false;
            double limInf=0,limSup=0;
            cfg_get(cfg, "E", limInf, limSup);

            for (unsigned int k=0; k<M.numLecturas; ++k) {
                const Lectura &L = M.lecturas[k];
                if(L.tipo=='E'){tieneECG=true;if(L.valor<minVal)minVal=L.valor;if(L.valor>maxVal)maxVal=L.valor;}
            }
            if(tieneECG){
                double sumaAbs=fabs(minVal)+fabs(maxVal);
                double sumaAbsLim=fabs(limInf)+fabs(limSup);
                if(sumaAbs>sumaAbsLim) fout << " ECG: ANOMALO\n";
            }

            fout << "--------------------------\n";
        }
    }
    fout.close();
    cout << "Reporte generado en " << filename << endl;
}


bool exportar_pacientes_ecg_anomalos(const SalaUCI& sala, const PacientesData& pacs,
                                     const ConfigData& cfg, const char* rutaBin){
    double mnE,mxE; if(!cfg_get(cfg,"E",mnE,mxE)) return false;

    bool marca[256]={false};

    for(int i=0;i<sala.nmaq;++i){
        const MaquinaUCI& MQ=sala.maquinas[i];
        for(int j=0;j<MQ.numMediciones;++j){
            const Medicion& M=MQ.mediciones[j];
            for(int k=0;k<M.numLecturas;++k){
                const Lectura& L=M.lecturas[k];
                if(L.tipo==TS_E && (L.valor<mnE || L.valor>mxE)){
                    int idNum = atoi(M.idPaciente); // convierte C-string a int
                    marca[idNum] = true;
                }
            }
        }
    }

    FILE* f=fopen(rutaBin,"wb");
    if(!f) return false;
    uint16_t c=0; for(int i=0;i<256;++i) if(marca[i]) c++;
    fwrite(&c,1,2,f);
    for(int i=0;i<256;++i) if(marca[i]){
        unsigned char id=(unsigned char)i; fwrite(&id,1,1,f);
    }
    fclose(f);
    return true;
}