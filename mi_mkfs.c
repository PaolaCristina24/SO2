/**
 * 
 * @author Juana Luna
 * @author Paola Chacín
 * @author Yassin EL Gharsa
 *
 * Programa: mi_mkfs.c
 * Inicializa un dispositivo virtual (fichero) con un número determinado 
 * de bloques, rellenándolos con ceros (formateo de bajo nivel).
 */

#include "ficheros_basico.h"

/**
 * main: Función principal del formateador.
 * Parámetros de entrada (vía consola):
 * - argv[1]: Nombre del dispositivo virtual (ej: "disco.bin")
 * - argv[2]: Número de bloques que tendrá el dispositivo (nbloques)
 * Retorna: EXITO (0) o FALLO (-1)
 */
int main(int argc, char **argv) {
    // 1. Validación de parámetros
    if (argc != 3) {
        fprintf(stderr, RED "Error de sintaxis: ./mi_mkfs <nombre_dispositivo> <nbloques>\n" RESET);
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    int nbloques = atoi(argv[2]); // Convertimos el string a entero

    // 2. Montar el dispositivo virtual
    // Llamamos a bmount()
    if (bmount(nombre_dispositivo) == FALLO) {
        return FALLO;
    }

    // 3. Preparar el buffer de ceros
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE); // Rellenamos el buffer con 0s

    // 4. Inicializar los nbloques a 0s
    // Llamamos a bwrite() repetidamente
    printf(BLUE "Formateando %d bloques en %s...\n" RESET, nbloques, nombre_dispositivo);
    
    for (int i = 0; i < nbloques; i++) {
        if (bwrite(i, buffer) == FALLO) {
            fprintf(stderr, RED "Error al inicializar el bloque %d\n" RESET, i);
            bumount(); // Intentamos cerrar antes de salir por error
            return FALLO;
        }
    }

    int ninodos = nbloques / 4; // Heurística: 1 inodo cada 4 bloques

    // 5. Inicializar el superbloque (SB)
    if (initSB(nbloques, ninodos) == FALLO) {
        bumount();
        return FALLO;
    }

    // 6. Inicializar el mapa de bits (MB)
    if (initMB() == FALLO) {
        bumount();
        return FALLO;
    }

    // 7. Inicializar el array de inodos (AI)
    if (initAI() == FALLO) {
        bumount();
        return FALLO;
    }

    reservar_inodo('d', 7); // Reservamos el inodo 0 para el directorio raíz

    // 8. Desmontar el dispositivo virtual
    if (bumount() == FALLO) {
        return FALLO;
    }

    printf(GREEN "Dispositivo virtual '%s' creado correctamente con %d bloques.\n" RESET, nombre_dispositivo, nbloques);
    
    return EXITO;
}

