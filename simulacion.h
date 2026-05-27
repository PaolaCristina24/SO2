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
#include <signal.h>

#define NUMPROCESOS 100
#define NUMESCRITURAS 50
#define REGMAX 500000

struct REGISTRO {
    time_t fecha;       // Fecha de la escritura (segundos)
    pid_t pid;          // PID del proceso creador
    int nEscritura;     // Nº de escritura (de 1 a 50)
    int nRegistro;      // Nº de registro aleatorio [0..REGMAX-1]
};

void reaper();

#endif