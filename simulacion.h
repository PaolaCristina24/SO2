#ifndef SIMULACION_H
#define SIMULACION_H

#include "directorios.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <time.h>
#include <string.h>

#define NUMERO_DE_PROCESOS 100

/*
 * Estructura que representa un registro de escritura
 * realizado por un proceso hijo.
 */ 
struct REGISTRO {
    time_t fecha;       // Fecha de la escritura
    pid_t pid;          // PID del proceso
    int nEscritura;     // Número de escritura 
    int nRegistro;      // Posición aleatoria dentro del fichero
};

// Variable global para contar procesos acabados
static int acabados = 0;

/*
 * Función enterrador.
 * Recoge procesos hijos finalizados para evitar zombies.
 */
void reaper();

#endif