#include <stdio.h>
#include "semaforo_mutex_posix.h"

// Inicializa el semáforo de forma limpia (borrando el anterior del kernel si existía)
sem_t *initSem() {
    sem_t *sem;

    // Forzamos el borrado del semáforo residual en el sistema operativo
    sem_unlink(SEM_NAME); 

    // Creamos el semáforo limpio con valor inicial 1
    sem = sem_open(SEM_NAME, O_CREAT, S_IRWXU, SEM_INIT_VALUE);
    if (sem == SEM_FAILED) {
        perror("Error grave en sem_open()");
        return NULL;
    }
    return sem;
}

// Hace un wait atómico real sobre el semáforo POSIX
void waitSem(sem_t *sem) {
    if (sem_wait(sem) == -1) {
        perror("Error en sem_wait()");
    }
}

// Hace un signal atómico real sobre el semáforo POSIX
void signalSem(sem_t *sem) {
    if (sem_post(sem) == -1) {
        perror("Error en sem_post()");
    }
}

// Cierra el enlace del proceso con el semáforo
void destruirSem() {
    // Si tienes una variable global sem_t *mutex o pasas el puntero, se puede cerrar.
    // Lo estándar para el unmount de los hijos es desvincularse:
    sem_unlink(SEM_NAME);
}