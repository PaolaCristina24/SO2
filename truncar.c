#include <stdio.h>
#include <stdlib.h>
#include "ficheros.h"

int main(int argc, char **argv)
{

    // Comprobamos que se pasan bien los argumentos
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);

    // Montamos el dispositivo antes de hacer nada
    if (bmount(nombre_dispositivo) == FALLO)
    {
        perror("Error al montar dispositivo");
        return FALLO;
    }

    int liberados;

    // Si nbytes es 0 liberamos el inodo entero
    if (nbytes == 0)
    {
        liberados = liberar_inodo(ninodo);

        if (liberados == FALLO)
        {
            fprintf(stderr, "Error liberando inodo\n");
            bumount();
            return FALLO;
        }

        printf("Inodo %d liberado correctamente\n", ninodo);
    }
    else
    {
        // Si no, truncamos el fichero al tamaño indicado
        liberados = mi_truncar_f(ninodo, nbytes);

        if (liberados == FALLO)
        {
            fprintf(stderr, "Error truncando inodo\n");
            bumount();
            return FALLO;
        }

        printf("Bloques liberados: %d\n", liberados);
    }

    // Sacamos info del inodo para comprobar que todo ha ido bien
    struct STAT stat;

    if (mi_stat_f(ninodo, &stat) == FALLO)
    {
        fprintf(stderr, "Error en mi_stat_f\n");
        bumount();
        return FALLO;
    }
    char atime[80];
    char mtime[80];
    char ctime[80];

    struct tm *tm;

    tm = localtime(&stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", tm);

    tm = localtime(&stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", tm);

    tm = localtime(&stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", tm);


    printf(YELLOW);
    printf("DATOS INODO:\n");

    printf("tipo %c\n", stat.tipo);
    printf("permisos %d\n", stat.permisos);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);
    printf("nlinks %d\n", stat.nlinks);
    printf("tamEnBytesLog %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados %d\n", stat.numBloquesOcupados);
    printf(GRAY);

    // Desmontamos el dispositivo (importante no olvidarlo)
    if (bumount() == FALLO)
    {
        perror("Error al desmontar");
        return FALLO;
    }

    return EXITO;
}