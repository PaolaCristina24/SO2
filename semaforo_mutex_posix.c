#include "semaforo_mutex_posix.h"
#include <stdio.h>

sem_t *initSem() {
    sem_t *sem;
    // Abrimos/creamos el semáforo POSIX con nombre de forma segura
    sem = sem_open(SEM_NAME, O_CREAT, S_IRWXU, SEM_INIT_VALUE);
    if (sem == SEM_FAILED) {
        perror("Error sem_open()");
        return NULL;
    }
    return sem;
}

// Cierra el descriptor del semáforo en el proceso actual (lo hacen hijos y padre)
void deleteSem(sem_t *sem) {
    if (sem) {
        if (sem_close(sem) == -1) {
            perror("Error en sem_close() dentro de deleteSem");
        }
    }
}

// Elimina el semáforo por completo del sistema operativo (SOLO lo llamará el padre)
void destruirSem() {
    if (sem_unlink(SEM_NAME) == -1) {
        perror("Error en sem_unlink() dentro de destruirSem");
    }
}

void signalSem(sem_t *sem) {
    sem_wait(sem); // En semáforos POSIX, wait resta 1 (Operación P)
}

void waitSem(sem_t *sem) {
    sem_post(sem); // En semáforos POSIX, post suma 1 (Operación V)
}