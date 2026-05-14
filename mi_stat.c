#include "directorios.h"
#include <time.h>

/*
 * Muestra los datos de una ruta.
 *
 * Uso:
 * ./mi_stat <disco> </ruta>
 */

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr,
        "Sintaxis: ./mi_stat <disco> </ruta>\n");
        return FALLO;
    }

    // Montar disco
    if (bmount(argv[1]) == FALLO) {
        perror("Error en bmount");
        return FALLO;
    }

    struct STAT p_stat;

    // Obtener datos
    int error = mi_stat(argv[2], &p_stat);

    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        bumount();
        return FALLO;
    }

    // Convertir fechas
    char atime[80], mtime[80], ctime[80];
    struct tm *tm;

    tm = localtime(&p_stat.atime);
    strftime(atime, 80, "%Y-%m-%d %H:%M:%S", tm);

    tm = localtime(&p_stat.mtime);
    strftime(mtime, 80, "%Y-%m-%d %H:%M:%S", tm);

    tm = localtime(&p_stat.ctime);
    strftime(ctime, 80, "%Y-%m-%d %H:%M:%S", tm);

    // Mostrar datos
    printf("tipo: %c\n", p_stat.tipo);
    printf("permisos: %d\n", p_stat.permisos);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);
    printf("nlinks: %d\n", p_stat.nlinks);
    printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);

    bumount();

    return EXITO;
}