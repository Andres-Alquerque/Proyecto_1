#ifndef LECTURA_SENSOR_H
#define LECTURA_SENSOR_H

// Tipos de lectura
enum TipoSensor : unsigned char { TS_T='T', TS_P='P', TS_E='E', TS_O='O' };

struct Lectura {
    unsigned char tipo;  // 'T','P','E','O'
    double valor;        // T/E/O usan 'valor'
    float  p_sis;        // sólo si tipo=='P'
    float  p_dia;        // sólo si tipo=='P'
};

#endif
