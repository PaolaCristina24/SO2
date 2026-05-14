#include "ficheros.h"

int main(int argc, char *argv[]) {
    // 1. Comprobación de argumentos
    if (argc < 4) {
        fprintf(stderr, "Error de sintaxis: ./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        fprintf(stderr, "Offsets: 9000, 209000, 30725000, 409605000, 480000000\n");
        return -1;
    }

    char *nombre_dispositivo = argv[1];
    char *texto = argv[2];
    int diferentes_inodos = atoi(argv[3]);
    int longitud = strlen(texto);

    // Definición de los offsets solicitados
    unsigned int offsets[] = {9000, 209000, 30725000, 409605000, 480000000};
    int num_escrituras = sizeof(offsets) / sizeof(unsigned int);

    // 2. Montar el dispositivo
    if (bmount(nombre_dispositivo) == -1) {
        fprintf(stderr, "Error al montar el dispositivo\n");
        return -1;
    }

    int ninodo;
    if (diferentes_inodos == 0) {
        // Reservamos un solo inodo para todas las escrituras
        ninodo = reservar_inodo('f', 6);
        if (ninodo == -1) return -1;
        printf("Se ha reservado el inodo: %d\n", ninodo);
    }

    for (int i = 0; i < num_escrituras; i++) {
        if (diferentes_inodos == 1) {
            // Reservamos un inodo nuevo para cada offset
            ninodo = reservar_inodo('f', 6);
            if (ninodo == -1) break;
            printf("Escritura en offset %u. Se ha reservado el inodo: %d\n", offsets[i], ninodo);
        }

        printf("Indicando escritura en offset: %u\n", offsets[i]);
        
        // 3. Escribir en el inodo
        int bytes_escritos = mi_write_f(ninodo, texto, offsets[i], longitud);
        if (bytes_escritos < 0) {
            fprintf(stderr, "Error al escribir en el inodo %d\n", ninodo);
            continue;
        }

        // 4. Obtener y mostrar estadísticas del inodo
        struct STAT stat;
        if (mi_stat_f(ninodo, &stat) == -1) {
            fprintf(stderr, "Error al obtener stat del inodo %d\n", ninodo);
        } else {
            printf("Bytes escritos: %d\n", bytes_escritos);
            printf("stat.tamEnBytesLog = %u\n", stat.tamEnBytesLog);
            printf("stat.numBloquesOcupados = %u\n", stat.numBloquesOcupados);
        }
        printf("--------------------------------------------------\n");
    }

    // 5. Desmontar el dispositivo
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}