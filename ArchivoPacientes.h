#ifndef ARCHIVO_PACIENTES_H
#define ARCHIVO_PACIENTES_H

// CSV: id; tipoDoc; documento; nombres; apellidos; fechaNac; ...
struct ArchivoPacientes {
    unsigned char idBSF; // id 1 byte para BSF (mapeado desde idCSV&0xFF)
    int  idCSV;
    char tipoDoc[4];
    char documento[16];
    char nombres[48];
    char apellidos[48];
    char fechaNac[11]; // "DD/MM/AAAA"
};

struct PacientesData {
    ArchivoPacientes* items;
    int count;
};

#endif
