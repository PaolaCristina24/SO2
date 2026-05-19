/**
 * @author Juana Luna
 * @author Paola Chacín
 * @author Yassin EL Gharsa
 * 
 *  Programa que crea directorios.
 */


#include "directorios.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    // =========================
    // COMPROBAR SINTAXIS
    // =========================

    if (argc != 4)
    {
        fprintf(stderr,
                RED "Sintaxis: ./mi_mkdir <nombre_dispositivo> <permisos> </ruta_directorio/>\n" RESET);
        return FALLO;
    }

    // =========================
    // COMPROBAR PERMISOS
    // =========================

    int permisos = atoi(argv[2]);

    if (permisos < 0 || permisos > 7)
    {
        fprintf(stderr,
                RED "Error: modo inválido: <%s>\n" RESET,
                argv[2]);
        return FALLO;
    }

    // =========================
    // COMPROBAR RUTA
    // =========================

    const char *ruta = argv[3];

    // Debe empezar por /
    if (ruta[0] != '/')
    {
        fprintf(stderr,
                RED "Error: Camino incorrecto.\n" RESET);
        return FALLO;
    }

    // Debe acabar en /
    if (ruta[strlen(ruta) - 1] != '/')
    {
        fprintf(stderr,
                RED "Error: la ruta no es un directorio.\n" RESET);
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
    // CREAR DIRECTORIO
    // =========================

    int error = mi_creat(ruta, permisos);

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
        perror("Error en bumount");
        return FALLO;
    }

    return EXITO;
}