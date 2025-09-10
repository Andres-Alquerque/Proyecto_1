#ifndef MEDICION_PACIENTE_H
#define MEDICION_PACIENTE_H
#include "LecturaSensor.h"

struct Medicion {
    char idPaciente[11];
    char fecha[24];
    unsigned int  numLecturas;
    Lectura* lecturas;
};

#endif
