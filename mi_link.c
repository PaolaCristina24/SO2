#include <stdio.h>
#include <string.h>
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr,
                "Sintaxis: ./mi_link <disco> </ruta_original> </ruta_enlace>\n");
        return FALLO;
    }

    // No permitir directorios
    if (argv[2][strlen(argv[2]) - 1] == '/' ||
        argv[3][strlen(argv[3]) - 1] == '/')
    {
        fprintf(stderr, "Error: no se permiten enlaces a directorios\n");
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO)
    {
        perror("Error en bmount");
        return FALLO;
    }

    int res = mi_link(argv[2], argv[3]);

    if (res == FALLO)
    {
        fprintf(stderr, "Error en mi_link\n");
        bumount();
        return FALLO;
    }

    bumount();
    return EXITO;
}