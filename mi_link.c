#include <stdio.h>
#include <string.h>
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: ./mi_link <disco> </ruta_original> </ruta_enlace>\n");
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

    // Guardamos el código de retorno de mi_link
    int res = mi_link(argv[2], argv[3]);

    // Si es un valor negativo, significa que hubo un error específico
    if (res < 0)
    {
        // Esta función se encarga de pintar en pantalla "Error: No existe..." o "Error: El archivo ya existe..."
        mostrar_error_buscar_entrada(res);
        bumount();
        return FALLO;
    }

    bumount();
    return EXITO;
}