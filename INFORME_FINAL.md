# Informe Final — Proyecto Sala UCI

## 1. Resumen del proyecto
Aplicación en **C++17** que **lee**, **guarda en memoria** y **analiza** lecturas de pacientes de una **Sala UCI** desde un archivo binario **.bsf**. Usa una **configuración de rangos** para marcar valores fuera de lo normal y genera **reportes**.

## 2. Estructuras de datos (resumen)
**Configuración**
```cpp
struct Configuracion { char tipo[8]; double minVal; double maxVal; };
struct ConfigData { Configuracion* items; int count; };
```
Guarda límites por tipo: `T`, `P_SIS`, `P_DIA`, `E`, `O`.

**Pacientes**
```cpp
struct ArchivoPacientes {
  unsigned char idBSF; int idCSV;
  char tipoDoc[4]; char documento[16];
  char nombres[48]; char apellidos[48];
  char fechaNac[11];
};
struct PacientesData { ArchivoPacientes* items; int count; };
```
`idBSF` es el ID (1 byte) que aparece en el `.bsf`.

**Lecturas**
```cpp
enum TipoSensor : unsigned char { TS_T='T', TS_P='P', TS_E='E', TS_O='O' };
struct Lectura { unsigned char tipo; double valor; float p_sis; float p_dia; };
```
Para `T`, `E`, `O` se usa `valor`. Para presión `P` se usan `p_sis` y `p_dia`.

**Mediciones, Máquinas y Sala**
```cpp
struct Medicion { char idPaciente[11]; char fecha[24]; unsigned int numLecturas; Lectura* lecturas; };
struct MaquinaUCI { unsigned char id; int numMediciones; Medicion* mediciones; };
struct SalaUCI { unsigned char id; unsigned char nmaq; MaquinaUCI* maquinas; };
```
Se usa **new[]/delete[]** para arreglos. Hay funciones `liberar_*` para limpiar.

## 3. Formato del archivo `.bsf` (idea general)
- Orden de bytes: **little‑endian**.
- **Sala**: `sala_id (u8)`, `nmaq (u8)`  
- **Máquina**: `maq_id (u8)`, `nmed (u16)`  
- **Medición**: `idPaciente (u8)`, `fecha[32]` (ASCII con ceros al final), `nLect (u16)`  
- **Lectura**:
  - Si `tipo == 'P'`: `p_sis (float32)`, `p_dia (float32)`
  - Si no: `valor (double64)`

## 4. Funciones más complejas (explicadas en simple)

### 4.1 `bool leer_bsf(const char* ruta, SalaUCI& sala)`
Lee el binario `.bsf` y arma toda la estructura en memoria.
Pasos clave (en bucles anidados):
1. Lee **sala_id** y **nmaq**. Reserva `sala.maquinas`.
2. Por cada **máquina**: lee `maq_id` y `nmed`. Reserva `mediciones`.
3. Por cada **medición**: lee `idPaciente`, `fecha[32]`, `nLect`. Reserva `lecturas`.
4. Por cada **lectura**: lee `tipo`.  
   - Si `tipo=='P'`, lee `p_sis` y `p_dia` (float).  
   - Si no, lee `valor` (double).
5. Valida tamaños leídos y evita **overflows**. Si algo falla, libera memoria y retorna `false`.

### 4.2 `bool reporte_anomalias_global(const SalaUCI& sala, const ConfigData& cfg, const char* rutaTxt)`
Genera `anomalias.txt` con **valores fuera de rango** y **estadísticas** por tipo.
Pasos:
1. Recorre todas las lecturas de todas las máquinas y mediciones.
2. Busca límites con `cfg_get` para el tipo correspondiente (`T`, `E`, `O`, `P_SIS`, `P_DIA`).
3. Compara y marca fuera de rango. Acumula **min, max, sum, conteo** por tipo.
4. Al final, escribe el archivo con la lista y el resumen (incluye **promedio**).

### 4.3 `bool reporte_mediciones_paciente(const SalaUCI& sala, int idPaciente, const ConfigData& cfg, const char* ruta)`
Crea `mediciones_paciente_<ID>.txt` con un **resumen** de las lecturas de un paciente.
Pasos:
1. Filtra mediciones con `idPaciente` dado.
2. Para cada lectura, compara con los límites de `cfg` y etiqueta **normal/anómalo**.
3. Calcula estadísticas simples por tipo y escribe el archivo.

### 4.4 `bool exportar_pacientes_ecg_anomalos(const SalaUCI& sala, const PacientesData& pacs, const ConfigData& cfg, const char* rutaBin)`
Genera un **binario** con pacientes cuyo **ECG (`E`)** está fuera de rango.
Pasos:
1. Recorre todas las lecturas y detecta si un paciente tiene al menos un `E` fuera de rango.
2. Guarda los **IDs (u8)** únicos detectados.
3. Escribe en `rutaBin`: primero `u16 cantidad`, luego la lista de IDs (u8).

### 4.5 `bool cargar_configuracion_txt(const char* ruta, ConfigData& cfg)` *(parsing tolerante)*
Lee `configuracion.txt` aceptando `,` o `;` como separadores y corrige **comas decimales** si aparecen.
Pasos:
1. Limpia espacios y caracteres raros por línea.
2. Divide en **clave, min, max**. Convierte a `double` con cuidado.
3. Inserta en `cfg.items`. Valida que `min < max`.

## 5. Flujo del programa (resumen)
1. (Opcional) Cargar `configuracion.txt` y `pacientes.csv`.
2. Leer el archivo `.bsf`.
3. Generar reportes según el menú.
4. Liberar memoria y terminar.

## 6. Cómo probar rápido
```bash
g++ -std=c++17 -O2 main.cpp funciones.cpp -o uci_app
./uci_app
# 1) Cargar config/pacientes
# 2) Leer "patient_readings_simulation 1.bsf"
# 3) "anomalias.txt"
# 4) "mediciones_paciente_<ID>.txt"
# 5) "pacientes_ecg_anomalos.dat"
```

## 7. Resultados esperados
- `anomalias.txt` → fuera de rango + estadísticas por tipo.
- `mediciones_paciente_<ID>.txt` → resumen del paciente.
- `pacientes_ecg_anomalos.dat` → cantidad + IDs (u8) con ECG anómalo.


## 9. Uso de IA (aclaración breve)

Se usó **ChatGPT** como apoyo en algunas partes difíciles del código. En concreto, ayudó a escribir o mejorar fragmentos de:

- `leer_bsf(...)` — estructura de bucles y validaciones.
- `reporte_anomalias_global(...)` — acumuladores y promedio.
- `exportar_pacientes_ecg_anomalos(...)` — formato del archivo binario.
- `cargar_configuracion_txt(...)` — manejo de separadores y comas decimales.

La IA dio ideas y ejemplos; el código final se revisó y ajustó manualmente por el equipo.

**Prompts usados (resumen):**
- "Explica un parser simple para un .bsf con jerarquía sala→máquina→medición→lectura en C++17 y cómo validar tamaños."
- "Cómo calcular min, max y promedio por tipo de lectura en un recorrido único."
- "Cómo escribir un binario con u16 de cantidad y lista de u8 de IDs."
- "Cómo leer configuracion.txt aceptando , y ; y corrigiendo comas decimales."

*(No hay enlace público a la conversación; se deja constancia del uso de IA como pide la entrega.)*


## 8. Limitaciones y mejoras
- Validación simple de formatos en `pacientes.csv`.
- El `.bsf` asume tamaños y **little‑endian** comunes en PC.
- No se exporta toda la estructura a JSON/CSV.
- Faltan **tests** y validaciones extra.
