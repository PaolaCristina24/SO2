/**
 * @author
 */

#include "directorios.h"

/** Cambia los permisos de un fichero o directorio dado su camino
 * @param argc Número de argumentos
 * @param argv Array de argumentos
 * @return EXITO si se han cambiado los permisos correctamente, o un código de error negativo si no se han podido cambiar
 */
int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr,
        RED "Sintaxis incorrecta: ./mi_chmod <disco> <permisos> </ruta>\n" RESET);
        return FALLO;
    }

    // Montar disco
    if (bmount(argv[1]) == FALLO) {
        perror("Error en bmount");
        return FALLO;
    }

    int permisos = atoi(argv[2]);

    // Validar permisos
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED "Error: permisos inválidos\n" RESET);
        bumount();
        return FALLO;
    }

    char ruta[strlen(argv[3]) + 1];
    strcpy(ruta, argv[3]);

    // Cambiar permisos
    int error = mi_chmod(ruta, permisos);

    if (error < 0) {
        mostrar_error_buscar_entrada(error);
    }

    bumount();

    return error;
}