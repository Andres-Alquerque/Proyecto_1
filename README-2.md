# Proyecto Sala UCI — README

Este programa en **C++17** lee datos de pacientes de una **sala UCI** desde un archivo binario (`.bsf`), usa una **configuración de rangos** y un **listado de pacientes**, y genera **reportes**.

## Requisitos
- Compilador C++ con **C++17** (g++ 9+, clang 10+, o MSVC 2019+).
- (Opcional) **CMake 3.26+**.
- Archivos de ejemplo (ya incluidos):
  - `configuracion.txt` → rangos permitidos (T, P_SIS, P_DIA, E, O).
  - `pacientes.csv` → datos básicos de pacientes.
  - `patient_readings_simulation 1.bsf` → lecturas simuladas (puede haber más `.bsf`).

## Carpetas y archivos (vista rápida)
```
Final Entrega/
├─ CMakeLists.txt
├─ main.cpp
├─ funciones.cpp
├─ *.h                # Estructuras y prototipos
├─ configuracion.txt
├─ pacientes.csv
└─ *.bsf
```

## Cómo compilar

### Opción A: CMake
```bash
cd "Final Entrega"
cmake -S . -B build
cmake --build build --config Release
```
El ejecutable se llama **Talleres_Proyecto** (según `CMakeLists.txt`).

### Opción B: g++
```bash
cd "Final Entrega"
g++ -std=c++17 -O2 main.cpp funciones.cpp -o uci_app
```
> Si los headers (`*.h`) están en otra carpeta, agrega `-I<ruta>`.

## Cómo ejecutar
Ubícate en la carpeta **Final Entrega** y corre uno de estos:
```bash
./Talleres_Proyecto   # si usaste CMake
# o
./uci_app             # si usaste g++
```

## Qué hace el programa (menú)
1. **Cargar configuración y pacientes**  
   Lee `configuracion.txt` y `pacientes.csv`. Muestra cuántos datos se cargaron.
2. **Leer archivo .bsf**  
   Lee `patient_readings_simulation 1.bsf` y guarda toda la información en memoria.
3. **Reporte de anomalías** → crea `anomalias.txt`  
   Marca valores fuera de rango usando los límites de `configuracion.txt`.
4. **Reporte por paciente** → crea `mediciones_paciente_<ID>.txt`  
   Pide el **ID** y hace un resumen de sus lecturas.
5. **Exportar pacientes con ECG fuera de rango** → crea `pacientes_ecg_anomalos.dat`  
   Guarda la **cantidad** y los **IDs (u8)** de pacientes con ECG anómalo.
0. **Salir**

## Archivos de entrada (formato corto)
- **configuracion.txt**  
  Ejemplo:
  ```
  T,36.0,38.0
  P_SIS,90,140
  P_DIA,60,90
  E,-3.858747,1.228621
  O,95,100
  ```
- **pacientes.csv**  
  Separado por `;`. Incluye: `id;tipoDoc;documento;nombres;apellidos;fechaNac;...`
- ***.bsf**  
  Binario con la jerarquía: Sala → Máquina → Medición → Lecturas.

## Problemas comunes
- **No encuentra archivos**: ejecuta el programa desde `Final Entrega/` o cambia las rutas en el código.
- **Errores en Windows**: compila en *Release* y usa C++17.
- **Acentos raros**: usa la consola en UTF‑8.

## Licencia
Uso académico.
