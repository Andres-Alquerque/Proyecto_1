#ifndef SALA_UCI_H
#define SALA_UCI_H

#include "MaquinaUCI.h"

struct SalaUCI {
    unsigned char id;
    unsigned char nmaq;
    MaquinaUCI* maquinas;
};

#endif
