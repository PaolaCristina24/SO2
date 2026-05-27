#include "simulacion.h"
#include "semaforo_mutex_posix.h"

// Variable global real 
int acabados = 0;

void reaper() {
    pid_t ended;
    signal(SIGCHLD, reaper); // Restablecer el manejador
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Sintaxis: ./simulacion <disco>\n");
        return FALLO;
    }

    // Montamos el dispositivo virtual (padre)
    if (bmount(argv[1]) == FALLO) {
        perror("Error en el bmount del padre");
        return FALLO;
    }

    // Asociamos la señal SIGCHLD al enterrador
    signal(SIGCHLD, reaper);

    // Creación del directorio de simulación con timestamp
    time_t tiempo = time(NULL);
    struct tm *tm = localtime(&tiempo);
    char rutaa[100];

    // Formato exacto: /simul_aaaammddhhmmss/
    sprintf(rutaa, "/simul_%04d%02d%02d%02d%02d%02d/",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);

    if (mi_creat(rutaa, 6) < 0) {
        perror("Error creando directorio simulacion");
        bumount();
        return FALLO;
    }

    printf("*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS ***\n", NUMPROCESOS, NUMESCRITURAS);
    fflush(stdout); // ¡CRÍTICO! Obligamos a la terminal a pintar el mensaje antes del bucle fork

    // Creación de los procesos hijos
    for (int i = 1; i <= NUMPROCESOS; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Error en fork");
            break;
        }

        // =================================================================
        // PROCESO HIJO
        // =================================================================
        if (pid == 0) {
            // Cada hijo debe montar el dispositivo de forma independiente
            if (bmount(argv[1]) == FALLO) {
                perror("Error en bmount hijo");
                exit(FALLO);
            }

            char ruta_proceso[150];
            char ruta_fichero[200];
            pid_t mi_pid = getpid();

            sprintf(ruta_proceso, "%sproceso_%d/", rutaa, mi_pid);
            if (mi_creat(ruta_proceso, 6) < 0) {
                perror("Error creando directorio del proceso");
                bumount();
                exit(FALLO);
            }

            sprintf(ruta_fichero, "%sprueba.dat", ruta_proceso);
            if (mi_creat(ruta_fichero, 6) < 0) {
                perror("Error creando prueba.dat");
                bumount();
                exit(FALLO);
            }

            // Inicializar la semilla aleatoria única para este hijo
            srand(time(NULL) + mi_pid);

            // Bucle de escrituras
            for (int nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++) {
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = mi_pid;
                registro.nEscritura = nescritura;
                registro.nRegistro = rand() % REGMAX;

                // Escribir en la posición lógica correspondiente (offset en bytes)
                int bytes_escritos = mi_write(ruta_fichero, &registro, 
                                              registro.nRegistro * sizeof(struct REGISTRO), 
                                              sizeof(struct REGISTRO));
                
                if (bytes_escritos < 0) {
                    fprintf(stderr, "Error al escribir en el proceso %d\n", mi_pid);
                }

                // Esperar 0.05 segundos (50.000 microsegundos) entre escrituras
                usleep(50000);
            }

            printf("[Proceso %d: Completadas %d escrituras en %s]\n", i, NUMESCRITURAS, ruta_fichero);
            fflush(stdout); // Forzamos vaciado en cada hijo

            // Desmontar el dispositivo antes de salir
            bumount();
            exit(EXITO);
        }

        // =================================================================
        // PROCESO PADRE (Lanzador)
        // =================================================================
        // CORREGIDO: usleep directo de 0.15s para espaciar los forks de forma segura
        usleep(150000); 
    }

    // El padre se duerme con pause() hasta que todos los hijos hayan sido recogidos por reaper
    while (acabados < NUMPROCESOS) {
        pause();
    }

    // El padre desmonta su dispositivo (llamando internamente a deleteSem)
    bumount();

    // Y como es el último proceso vivo, elimina el semáforo del kernel de Linux de raíz
    destruirSem(); 

    printf("Simulación completada con éxito.\n");
    return EXITO;
}