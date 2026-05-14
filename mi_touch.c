#include "directorios.h"

/*
 * Crea un fichero vacío.
 *
 * Uso:
 * ./mi_touch <disco> <permisos> </ruta_fichero>
 */

int main(int argc, char **argv)
{
    // -----------------------------------------
    // Comprobar sintaxis
    // -----------------------------------------
    if (argc != 4) {
        fprintf(stderr,
        "Sintaxis: ./mi_touch <disco> <permisos> </ruta_fichero>\n");
        return FALLO;
    }

    int permisos = atoi(argv[2]);

    // Validar permisos
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, "Error: permisos inválidos\n");
        return FALLO;
    }

    // La ruta NO debe acabar en /
    if (argv[3][strlen(argv[3]) - 1] == '/') {
        fprintf(stderr, "Error: la ruta es un directorio.\n");
        return FALLO;
    }

    // -----------------------------------------
    // Montar disco
    // -----------------------------------------
    if (bmount(argv[1]) == FALLO) {
        perror("Error en bmount");
        return FALLO;
    }

    // -----------------------------------------
    // Crear fichero
    // -----------------------------------------
    int error = mi_creat(argv[3], permisos);

    if (error < 0) {
        mostrar_error_buscar_entrada(error);
    }

    // -----------------------------------------
    // Desmontar disco
    // -----------------------------------------
    bumount();

    return error;
}