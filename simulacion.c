//Nivel 12 - Simulación de procesos
#include "simulacion.h"

/*
 * Función enterrador.
 * Recoge procesos hijos finalizados para evitar zombies.
 */
void reaper() {

    pid_t ended;

    // Recogemos todos los hijos terminados
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {

        acabados++;

        fprintf(stderr,
                "[Proceso padre: recogido hijo %d (%d/%d)]\n",
                ended,
                acabados,
                NUMPROCESOS);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr,
                "Sintaxis: ./simulacion <disco>\n");
        return FALLO;
    }

    // Montamos el disco con bmount
    if (bmount(argv[1]) == FALLO)
    {
        perror("Error en el bmount");
        return FALLO;
    }

    // Asociamos señal SIGCHLD al reaper
    signal(SIGCHLD, reaper);

   // Creacion del directorio de simulacion con el timestamp actual
    time_t tiempo = time(NULL);
    struct tm *tm = localtime(&tiempo);

    char rutaa[100];

    sprintf(rutaa,
            "/simul_%04d%02d%02d%02d%02d%02d/",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec);

    if (mi_creat(rutaa, 6) < 0)
    {
        perror("Error creando directorio simulacion");
        bumount();
        return FALLO;
    }

    printf("Directorio simulacion: %s\n", rutaa);

    // Creacion de los 100 procesos 
    pid_t pid;
    for (int i = 0; i < NUMERO_DE_PROCESOS; i++)
    {
        pid = fork();

        if (pid < 0)
        {
            perror("Error en fork");
            break;
        }

        // =========================
        // PROCESO HIJO
        // =========================

        if (pid == 0)
        {
            // Montar disco en el hijo
            if (bmount(argv[1]) == FALLO)
            {
                perror("Error en bmount hijo");
                exit(FALLO);
            }

            char ruta_proceso[200];
            char ruta_fichero[250];

            sprintf(ruta_proceso,
                    "%sproceso_%d/",
                    rutaa,
                    getpid());

            // Crear directorio proceso_PID
            if (mi_creat(ruta_proceso, 6) < 0)
            {
                perror("Error creando directorio del proceso");
                bumount();
                exit(FALLO);
            }

            sprintf(ruta_fichero,
                    "%sprueba.dat",
                    ruta_proceso);

            // Crear fichero prueba.dat
            if (mi_creat(ruta_fichero, 6) < 0)
            {
                perror("Error creando prueba.dat");
                bumount();
                exit(FALLO);
            }

#if DEBUG12
            fprintf(stderr,
                    "[Proceso %d creado correctamente]\n",
                    getpid());
#endif

            bumount();
            exit(EXITO);
        }

        // Espera de 0.15 segundos
        usleep(150000);
    }

    // =========================
    // ESPERAR HIJOS
    // =========================

    for (int i = 0; i < NUMERO_DE_PROCESOS; i++)
    {
        wait(NULL);
    }

    bumount();

    return EXITO;
}