#ifndef ARCHIVO_CONFIGURACION_H
#define ARCHIVO_CONFIGURACION_H

struct Configuracion {
    char tipo[8];     // "T","P_SIS","P_DIA","E","O"
    double minVal;
    double maxVal;
};

struct ConfigData {
    Configuracion* items;
    int count;
};

#endif
