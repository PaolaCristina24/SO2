#include "verificacion.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr,
                "Sintaxis: ./verificacion <disco> </directorio_simulacion/>\n");
        return FALLO;
    }

    // Montar disco
    if (bmount(argv[1]) == FALLO)
    {
        perror("Error en bmount");
        return FALLO;
    }


    // COMPROBAR DIRECTORIO DE SIMULACIÓN
     struct STAT statdir;

    if (mi_stat(argv[2], &statdir) < 0)
    {
        fprintf(stderr, "Error obteniendo stat del directorio\n");
        bumount();
        return FALLO;
    }

    int numentradas =
        statdir.tamEnBytesLog / sizeof(struct entrada);

    printf("dir_sim: %s\n", argv[2]);
    printf("numentradas: %d NUMPROCESOS: %d\n",
           numentradas,
           NUMERO_DE_PROCESOS);

    if (numentradas != NUMERO_DE_PROCESOS)
    {
        fprintf(stderr,
                "Error: número de entradas incorrecto\n");

        bumount();
        return FALLO;
    }

    // CREAR informe.txt
    char informe[200];

    sprintf(informe,
            "%sinforme.txt",
            argv[2]);

    if (mi_creat(informe, 6) < 0)
    {
        perror("Error creando informe.txt");

        bumount();
        return FALLO;
    }


    // LEER ENTRADAS DEL DIRECTORIO
    struct entrada entradas[NUMERO_DE_PROCESOS];

    if (mi_read(argv[2],
                entradas,
                0,
                sizeof(entradas)) < 0)
    {
        perror("Error leyendo entradas");

        bumount();
        return FALLO;
    }

    // RECORRER CADA PROCESO
    for (int i = 0; i < NUMERO_DE_PROCESOS; i++)
    {
        struct INFORMACION info;

        memset(&info, 0, sizeof(struct INFORMACION));


        // EXTRAER PId
        char *pidstr =
            strchr(entradas[i].nombre, '_');

        info.pid = atoi(pidstr + 1);


        // RUTA prueba.dat
        char prueba[300];

        sprintf(prueba,
                "%s%s/prueba.dat",
                argv[2],
                entradas[i].nombre);


        // LEER REGISTROS
        int offset = 0;

        struct REGISTRO buffer_escrituras[256];

        int leidos;

        while ((leidos = mi_read(prueba,
                                 buffer_escrituras,
                                 offset,
                                 sizeof(buffer_escrituras))) > 0)
        {
            int registros_leidos =
                leidos / sizeof(struct REGISTRO);

            for (int j = 0;
                 j < registros_leidos;
                 j++)
            {
                struct REGISTRO reg =
                    buffer_escrituras[j];

                // Verificamos que el registro
                // pertenece al proceso
                if (reg.pid == info.pid)
                {
                    // Primera escritura válida
                    if (info.nEscrituras == 0)
                    {
                        info.PrimeraEscritura = reg;
                        info.UltimaEscritura = reg;
                        info.MenorPosicion = reg;
                        info.MayorPosicion = reg;
                    }
                    else
                    {
                        // Primera escritura
                        if (reg.nEscritura <
                            info.PrimeraEscritura.nEscritura)
                        {
                            info.PrimeraEscritura = reg;
                        }

                        // Última escritura
                        if (reg.nEscritura >
                            info.UltimaEscritura.nEscritura)
                        {
                            info.UltimaEscritura = reg;
                        }

                        // Menor posición
                        if (reg.nRegistro <
                            info.MenorPosicion.nRegistro)
                        {
                            info.MenorPosicion = reg;
                        }

                        // Mayor posición
                        if (reg.nRegistro >
                            info.MayorPosicion.nRegistro)
                        {
                            info.MayorPosicion = reg;
                        }
                    }

                    info.nEscrituras++;
                }
            }

            offset += sizeof(buffer_escrituras);

            // Limpiar buffer
            memset(buffer_escrituras,
                   0,
                   sizeof(buffer_escrituras));
        }

        printf("[%d) %d escrituras validadas en %s]\n",
               i + 1,
               info.nEscrituras,
               prueba);

        // GENERAR TEXTO INFORME

        char buffer[2000];
        memset(buffer, 0, sizeof(buffer));

        char fecha[80];

        sprintf(buffer,
                "PID: %d\n"
                "Numero de escrituras: %u\n",
                info.pid,
                info.nEscrituras);

        strftime(fecha,
                 80,
                 "%Y-%m-%d %H:%M:%S",
                 localtime(&info.PrimeraEscritura.fecha));

        sprintf(buffer + strlen(buffer),
                "Primera Escritura\t%d\t%d\t%s\n",
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                fecha);

        strftime(fecha,
                 80,
                 "%Y-%m-%d %H:%M:%S",
                 localtime(&info.UltimaEscritura.fecha));

        sprintf(buffer + strlen(buffer),
                "Ultima Escritura\t%d\t%d\t%s\n",
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                fecha);

        strftime(fecha,
                 80,
                 "%Y-%m-%d %H:%M:%S",
                 localtime(&info.MenorPosicion.fecha));

        sprintf(buffer + strlen(buffer),
                "Menor Posicion\t%d\t%d\t%s\n",
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                fecha);

        strftime(fecha,
                 80,
                 "%Y-%m-%d %H:%M:%S",
                 localtime(&info.MayorPosicion.fecha));

        sprintf(buffer + strlen(buffer),
                "Mayor Posicion\t%d\t%d\t%s\n\n",
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                fecha);

        // ESCRIBIR EN informe.txt
        struct STAT statinfo;

        mi_stat(informe, &statinfo);

        if (mi_write(informe,
                     buffer,
                     statinfo.tamEnBytesLog,
                     strlen(buffer)) < 0)
        {
            perror("Error escribiendo informe");

            bumount();
            return FALLO;
        }
    }

    bumount();

    return EXITO;
}