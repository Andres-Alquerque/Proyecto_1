#ifndef LECTURA_SENSOR_H
#define LECTURA_SENSOR_H


enum TipoSensor : unsigned char { TS_T='T', TS_P='P', TS_E='E', TS_O='O' };

struct Lectura {
    unsigned char tipo;
    double valor;
    float  p_sis;
    float  p_dia;
};

#endif
