#ifndef SEMAFORO_MUTEX_POSIX_H
#define SEMAFORO_MUTEX_POSIX_H

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#define SEM_NAME "/mymutex" 
#define SEM_INIT_VALUE 1 

sem_t *initSem();
void deleteSem(sem_t *sem); // <-- CORREGIDO: Ahora recibe el parámetro
void signalSem(sem_t *sem);
void waitSem(sem_t *sem);
void destruirSem();         // <-- NUEVA: Para que el padre haga el unlink al final

#endif