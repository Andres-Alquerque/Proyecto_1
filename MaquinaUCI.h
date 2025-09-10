#ifndef MAQUINA_UCI_H
#define MAQUINA_UCI_H

#include "MedicionPaciente.h"   // ←necesario

struct MaquinaUCI {
    unsigned char id;
    int numMediciones;
    Medicion* mediciones;
};

#endif
