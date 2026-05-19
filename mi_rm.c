#include "directorios.h"

int main(int argc, char **argv)
{
    // =========================
    // COMPROBAR SINTAXIS
    // =========================

    if (argc != 3)
    {
        fprintf(stderr,
                RED "Sintaxis: ./mi_rm <disco> </ruta>\n" RESET);

        return FALLO;
    }

    // =========================
    // NO BORRAR DIRECTORIO RAÍZ
    // =========================

    if (strcmp(argv[2], "/") == 0)
    {
        fprintf(stderr,
                RED "No se puede borrar el directorio raíz\n" RESET);

        return FALLO;
    }

    // =========================
    // MONTAR DISCO
    // =========================

    if (bmount(argv[1]) == FALLO)
    {
        perror("Error en bmount");
        return FALLO;
    }

    // =========================
    // BORRAR ENTRADA
    // =========================

    int error = mi_unlink(argv[2]);

    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        bumount();
        return FALLO;
    }

    // =========================
    // DESMONTAR DISCO
    // =========================

    if (bumount() == FALLO)
    {
        return FALLO;
    }

    return EXITO;
}