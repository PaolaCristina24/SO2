#include "directorios.h"


int main(int argc, char const *argv[])
{
    
    if (argc != 3)
    {
        fprintf(stderr,
                RED "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n" RESET);
        return FALLO;
    }

    if (argv[2][strlen(argv[2]) - 1] == '/')
    {
        fprintf(stderr,
                RED "Error: la ruta es un directorio.\n" RESET);
        return FALLO;
    }

// Montamos el disco
    if (bmount(argv[1]) == FALLO)
    {
        perror("Error en bmount");
        return FALLO;
    }
// Ruta del fichero a leer

    const char *camino = argv[2];

    int tambuffer = 10 * BLOCKSIZE;

    char buffer[tambuffer];

    int offset = 0;

    int bytes_leidos = 0;

    int leidos;

    // Leemos el fichero

    memset(buffer, 0, sizeof(buffer));

    leidos = mi_read(camino, buffer, offset, tambuffer);

    while (leidos > 0)
    {
        write(1, buffer, leidos);

        bytes_leidos += leidos;

        offset += leidos;

        memset(buffer, 0, sizeof(buffer));

        leidos = mi_read(camino, buffer, offset, tambuffer);
    }

// Comprobamos si ha habido un error en la lectura

    if (leidos < 0)
    {
        fprintf(stderr,
                RED "\nError al leer el fichero (Código: %d)\n" RESET,
                leidos);

        bumount();

        return FALLO;
    }

// Imprimimos el total de bytes leídos

    fprintf(stderr,
            "\n\nTotal_leidos %d\n",
            bytes_leidos);
// Desmontamos el disco

    if (bumount() == FALLO)
    {
        perror("Error en bumount");
        return FALLO;
    }

    return EXITO;
}
