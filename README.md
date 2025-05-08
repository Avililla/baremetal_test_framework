# Bare-Metal Test Framework (BMT)

**Versión: 1.0**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

<!-- Opcional: Añade más badges si tienes CI, cobertura de código, etc. -->
<!-- [![Build Status](URL_DE_TU_BADGE_DE_BUILD)](URL_A_TU_PIPELINE) -->

Framework de testing unitario ligero, estilo gtest, para entornos bare-metal con salida de resultados por UART y un script auxiliar en Python para parsear dichos resultados.

**Creado por: Alejandro Avila Marcos**

---

## Tabla de Contenidos

- [Bare-Metal Test Framework (BMT)](#bare-metal-test-framework-bmt)
  - [Tabla de Contenidos](#tabla-de-contenidos)
  - [Características](#características)
  - [Motivación](#motivación)
  - [Estructura del Proyecto](#estructura-del-proyecto)
  - [Requisitos](#requisitos)
    - [Para el Firmware](#para-el-firmware)
    - [Para el Script de PC](#para-el-script-de-pc)
  - [Instalación / Configuración](#instalación--configuración)
    - [Librería en C (Firmware)](#librería-en-c-firmware)
    - [Script Auxiliar en Python (PC)](#script-auxiliar-en-python-pc)
  - [Cómo Empezar (Guía Rápida)](#cómo-empezar-guía-rápida)
    - [1. Implementar la Interfaz de Plataforma](#1-implementar-la-interfaz-de-plataforma)
    - [2. Escribir Tests](#2-escribir-tests)
    - [3. Ejecutar los Tests](#3-ejecutar-los-tests)
    - [4. Parsear Resultados en el PC](#4-parsear-resultados-en-el-pc)
  - [API de la Librería](#api-de-la-librería)
    - [Macros Principales](#macros-principales)
    - [Funciones de Plataforma a Implementar (`bmt_platform_io.h`)](#funciones-de-plataforma-a-implementar-bmt_platform_ioh)
  - [Ejemplos](#ejemplos)
  - [Documentación](#documentación)
  - [Script Auxiliar Python](#script-auxiliar-python)
  - [Contribuciones](#contribuciones)
  - [Licencia](#licencia)
  - [Créditos y Agradecimientos](#créditos-y-agradecimientos)

---

## Características

- Sintaxis de tests familiar, inspirada en Google Test (`TEST`, `ASSERT_EQ`, `EXPECT_TRUE`, etc.).
- Diseñado para C en entornos bare-metal con recursos limitados.
- Salida de resultados de tests formateada a través de UART.
- Script auxiliar en Python para capturar y parsear la salida UART, mostrando un resumen de los tests y generando reportes JUnit XML.
- Mínimas dependencias externas para el código C del firmware (configurable).
- Abstracción de la capa de hardware (HAL) para E/S de plataforma (UART, timers).
- Auto-registro de tests (usando atributos de constructor de GCC).
- Documentación generada con Doxygen.

## Motivación

La necesidad de un framework de testing unitario simple y efectivo para proyectos de firmware en microcontroladores Xilinx (Zynq-7000, UltraScale), donde las soluciones de testing completas pueden ser demasiado pesadas o complejas de integrar. Este framework busca ofrecer una alternativa ligera y fácil de adoptar, manteniendo una experiencia de desarrollo similar a la de frameworks más establecidos como Google Test.

## Estructura del Proyecto

```
baremetal_test_framework/
├── Doxyfile                  # Configuración de Doxygen
├── include/                  # Archivos de cabecera públicos de la librería
│   ├── baremetal_gtest.h     # API principal para escribir tests
│   └── bmt_platform_io.h   # Interfaz de E/S de plataforma (a implementar por el usuario)
├── src/                      # Código fuente interno de la librería
│   └── bmt_runner.c          # Núcleo de ejecución de tests
├── examples/                 # Ejemplos de uso para diferentes plataformas
│   └── xilinx_zynq7000/
│       ├── platform_uart_zynq7000.c # Implementación de E/S para Zynq-7000
│       └── main_tests.c             # Ejemplo de archivo de tests para Zynq-7000
├── python_parser/            # Script auxiliar de Python (o el nombre que le hayas dado)
│   ├── parse_bmt_results.py  # Script para parsear la salida UART (o el nombre que le hayas dado)
│   └── requirements.txt      # Dependencias del script Python
├── LICENSE                   # Archivo de licencia (ej. MIT)
└── README.md                 # Este archivo
```

## Requisitos

### Para el Firmware

- Toolchain C compatible con C99/C11 (ej. GCC para ARM, como el incluido en Xilinx Vitis/SDK).
- Implementación por parte del usuario de las funciones definidas en `bmt_platform_io.h` para la comunicación UART y la temporización en su hardware específico.
- (Opcional, dependiendo de las aserciones usadas y la implementación de `bmt_report_failure`) Algunas funciones de `libc` como `snprintf`, `vsnprintf`, `strcmp`, `strncmp`, `fabsf` (generalmente disponibles en implementaciones de `libc` para embebidos como `newlib-nano`).

### Para el Script de PC

- Python 3.6 o superior.
- Dependencias listadas en `python_parser/requirements.txt` (ej. `pyserial`, `junit-xml`).

## Instalación / Configuración

### Librería en C (Firmware)

1.  Copia los directorios `include/` y `src/` a tu proyecto de firmware.
2.  Añade los archivos `.c` de `src/` a tu sistema de compilación.
3.  Asegúrate de que el directorio `include/` esté en tus rutas de inclusión del compilador.
4.  Crea tu propia implementación de `bmt_platform_io.c` (puedes basarte en los ejemplos) y compílala con tu proyecto.

### Script Auxiliar en Python (PC)

1.  Navega al directorio `python_parser/`.
2.  (Recomendado) Crea un entorno virtual:
    ```bash
    python -m venv venv
    source venv/bin/activate  # En Linux/macOS
    # venv\Scripts\activate   # En Windows (cmd)
    # venv\Scripts\Activate.ps1 # En Windows (PowerShell)
    ```
3.  Instala las dependencias:
    ```bash
    pip install -r requirements.txt
    ```

## Cómo Empezar (Guía Rápida)

### 1. Implementar la Interfaz de Plataforma

Crea un archivo (ej. `my_platform_io.c`) e implementa las siguientes funciones declaradas en `bmt_platform_io.h`:

- `void bmt_platform_io_init(void);` (para inicializar UART, timers)
- `void bmt_platform_putchar(char c);`
- `void bmt_platform_puts(const char *str);`
- `uint32_t bmt_platform_get_msec_ticks(void);`

Consulta el directorio `examples/` para ver implementaciones de referencia.

### 2. Escribir Tests

En tus archivos de test `.c`:

```c
#include "baremetal_gtest.h" // API principal de BMT
#include "tus_modulos_a_probar.h" // Tus propios módulos

// Test simple
TEST(MiSuite, MiTest) {
    int resultado = mi_funcion_a_probar(2, 2);
    ASSERT_EQ(resultado, 4);
    EXPECT_TRUE(otra_condicion_que_deberia_ser_verdadera());
}

// Otro test
TEST(OtraSuite, TestDePunteros) {
    void* ptr = mi_funcion_que_devuelve_puntero();
    ASSERT_NOT_NULL(ptr);
    // ... más aserciones con ptr ...
}
```

### 3. Ejecutar los Tests

En tu función `main()` del firmware:

```c
#include "baremetal_gtest.h" // Para RUN_ALL_TESTS()
#include "bmt_platform_io.h" // Para bmt_platform_puts (si envías tokens adicionales)

int main(void) {
    // 1. Inicialización de tu hardware/BSP (relojes, etc.)
    //    Esto puede incluir la inicialización de la UART que printf usaría.
    //    RUN_ALL_TESTS() llama a bmt_platform_io_init() internamente,
    //    así que esta función puede enfocarse en inicializar lo que BMT necesite
    //    (ej. el timer para la duración de los tests) o ser muy simple si la
    //    UART ya está configurada por la BSP.
    //
    //    ejemplo_inicializacion_bsp();

    // 2. Ejecutar todos los tests registrados.
    //    Esto imprimirá la salida estándar de gtest-like por la UART.
    int num_fallos = RUN_ALL_TESTS();

    // 3. (Opcional) Imprimir un resumen final o actuar según num_fallos.
    //    El propio framework ya imprime un resumen detallado.
    if (num_fallos == 0) {
        bmt_platform_puts("TODOS LOS TESTS BMT PASARON - FINAL DEL PROGRAMA\r\n");
    } else {
        char final_msg_buf[64];
        // snprintf es útil aquí si está disponible
        // sprintf(final_msg_buf, "%d TEST(S) BMT FALLARON - FINAL DEL PROGRAMA\r\n", num_fallos);
        // bmt_platform_puts(final_msg_buf);
        bmt_platform_puts("ALGUNOS TESTS BMT FALLARON - FINAL DEL PROGRAMA\r\n");
    }

    // 4. IMPORTANTE: Enviar el token de finalización para el script de Python
    //    Asegúrate de que este token sea único y no aparezca en la salida normal de los tests.
    bmt_platform_puts("[BMT_DONE_ALL_TESTS]\r\n");

    while(1) {
        // Bucle infinito o estado de bajo consumo
    }
    return 0; // Aunque en bare-metal, main usualmente no retorna.
}
```

Compila y carga este firmware en tu placa.

### 4. Parsear Resultados en el PC

Conecta tu placa al PC vía UART. Ejecuta el script de Python desde la raíz del proyecto o desde donde lo tengas:

```bash
python python_parser/parse_bmt_results.py --port TU_PUERTO_SERIAL --baud 115200 --junit_xml test_report.xml
```

(Reemplaza `TU_PUERTO_SERIAL` y ajusta el baudrate y el nombre del archivo XML si es necesario).

## API de la Librería

Consulta la **Documentación Completa de la API** generada por Doxygen para más detalles (ver sección [Documentación](#documentación)).

### Macros Principales

Usa estas macros dentro de los bloques `TEST(...) { ... }`:

- `TEST(SuiteName, TestName)`: Define un test.
- `ASSERT_TRUE(condicion)`, `ASSERT_FALSE(condicion)`
- `ASSERT_EQ(esperado, actual)`, `ASSERT_NE(val1, val2)`
- `ASSERT_LT(val1, val2)`, `ASSERT_LE(val1, val2)`
- `ASSERT_GT(val1, val2)`, `ASSERT_GE(val1, val2)`
- `ASSERT_NULL(puntero)`, `ASSERT_NOT_NULL(puntero)`
- `ASSERT_STREQ(str1, str2)`, `ASSERT_STRNE(str1, str2)` (y versiones `CASE` y `N` para insensibilidad a mayúsculas/minúsculas y comparación de n caracteres; requieren `strcmp`/`strncmp`/`strcasecmp`).
- `ASSERT_NEAR(val1, val2, error_absoluto)` (para comparaciones de punto flotante; requiere `fabs`/`fabsf`).
- `EXPECT_*`: Versiones de las macros `ASSERT_*` que reportan un fallo pero **no** detienen la ejecución del test actual. Permiten reportar múltiples fallos en un solo test.
- `FAIL()`: Marca el test actual como fallido y lo detiene inmediatamente.
- `ADD_FAILURE()`: Marca el test actual como fallido pero permite que continúe (similar a un `EXPECT_*` sin condición).
- `SUCCEED()`: No hace nada más que indicar explícitamente que se ha alcanzado un punto de éxito (imprime un mensaje si está habilitado).

### Funciones de Plataforma a Implementar (`bmt_platform_io.h`)

Estas funciones **deben** ser implementadas por el usuario para adaptar la librería a su hardware específico:

- `void bmt_platform_io_init(void);`: Inicializa la E/S de la plataforma (ej. UART para la salida, timer para la duración de los tests). Llamada una vez por `RUN_ALL_TESTS()`.
- `void bmt_platform_putchar(char c);`: Envía un único carácter a través de la interfaz de comunicación (ej. UART).
- `void bmt_platform_puts(const char *str);`: Envía una cadena de caracteres (terminada en null) a través de la interfaz de comunicación.
- `uint32_t bmt_platform_get_msec_ticks(void);`: Devuelve un timestamp o contador de ticks, preferiblemente con resolución de milisegundos, para medir la duración de los tests. Si no se implementa o devuelve siempre 0, la duración de los tests se reportará como 0 ms.

## Ejemplos

El directorio `examples/` contiene implementaciones de ejemplo completas para diferentes plataformas (ej. Xilinx Zynq-7000). Estos ejemplos muestran:

- Una implementación funcional de `bmt_platform_io.c`.
- Una función `main()` que configura el sistema y ejecuta los tests.
- Varios tests de ejemplo que demuestran el uso de diferentes macros de aserción.

## Documentación

La documentación detallada de la API, generada con Doxygen, está disponible:

- Localmente: Después de generar la documentación, abre `doxygen_docs/html/index.html` en tu navegador.
- (Opcional) Online: [Enlace a tu documentación hosteada, ej. en GitHub Pages]

Para generar la documentación localmente:

1.  Asegúrate de tener Doxygen y Graphviz (para diagramas) instalados.
2.  Navega a la raíz del proyecto (donde está `Doxyfile`).
3.  Ejecuta el comando:
    ```bash
    doxygen Doxyfile
    ```
4.  Para generar el PDF (requiere una instalación de LaTeX completa):
    ```bash
    cd doxygen_docs/latex
    make  # o make.bat en Windows, o latexmk -pdf refman.tex
    ```
    El PDF resultante será `doxygen_docs/latex/refman.pdf`.

## Script Auxiliar Python

El script `python_parser/parse_bmt_results.py` (o como lo hayas llamado) se utiliza para:

- Conectarse al puerto serie especificado.
- Capturar la salida de los tests BMT en tiempo real.
- Parsear la salida para identificar tests pasados, fallidos y los detalles de los fallos.
- Imprimir un resumen formateado en la consola.
- (Opcional) Generar un reporte en formato JUnit XML, que puede ser utilizado por herramientas de Integración Continua como Jenkins para visualizar y rastrear los resultados de los tests.

**Uso desde la línea de comandos:**

```bash
python python_parser/parse_bmt_results.py --port <PUERTO_SERIAL> [--baud <BAUDIOS>] [--junit_xml <NOMBRE_ARCHIVO_XML>]
```

Argumentos:

- `--port <PUERTO_SERIAL>`: (Requerido) El puerto serie al que está conectada la placa (ej. `COM3` en Windows, `/dev/ttyUSB0` en Linux).
- `--baud <BAUDIOS>`: (Opcional) La velocidad de transmisión en baudios (default: `115200`).
- `--junit_xml <NOMBRE_ARCHIVO_XML>`: (Opcional) Nombre del archivo donde se guardará el reporte en formato JUnit XML (ej. `test_report.xml`).

## Contribuciones

¡Las contribuciones son bienvenidas! Si deseas contribuir a mejorar BMT:

1.  Haz un Fork del repositorio.
2.  Crea una nueva rama para tu característica o corrección (`git checkout -b feature/mi-nueva-caracteristica` o `git checkout -b fix/corregir-este-bug`).
3.  Realiza tus cambios y haz commit de forma clara y concisa (`git commit -m 'Añade nueva característica X que hace Y'`).
4.  Empuja tus cambios a tu fork (`git push origin feature/mi-nueva-caracteristica`).
5.  Abre un Pull Request (PR) hacia la rama `main` (o `develop`, según la estrategia de ramas del proyecto) del repositorio original.

Por favor, asegúrate de que tus cambios sigan el estilo de código existente, estén bien documentados (si añades nueva API), y que todos los tests (si aplica, o añade nuevos tests para tu característica) pasen.

## Licencia

Este proyecto se distribuye bajo los términos de la **Licencia MIT**.
Puedes encontrar una copia completa de la licencia en el archivo [LICENSE](./LICENSE) en la raíz del repositorio.

## Créditos y Agradecimientos

- **Autor Principal:** Alejandro Avila Marcos
- Inspirado por el aburrimiento de mi casa.

---

```

```
