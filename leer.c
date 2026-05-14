#include "ficheros.h"

// Definimos un tamaño de buffer razonable (ej. 1500 bytes o múltiplos de bloque)
#define TAM_BUFFER 1500 

int main(int argc, char *argv[]) {
    // 1. Comprobación de argumentos
    if (argc != 3) {
        fprintf(stderr, "Error de sintaxis: ./leer <nombre_dispositivo> <ninodo>\n");
        return -1;
    }

    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);

    // 2. Montar el dispositivo
    if (bmount(nombre_dispositivo) == -1) {
        fprintf(stderr, "Error al montar el dispositivo\n");
        return -1;
    }



char buffer_texto[TAM_BUFFER];
int offset = 0;
int leidos = 0;
int total_leidos = 0;

// Bucle de lectura 
while ((leidos = mi_read_f(ninodo, buffer_texto, offset, TAM_BUFFER)) > 0) {
    total_leidos += leidos;
    
    if (write(1, buffer_texto, leidos) == -1) {
        perror("Error en write");
        break;
    }

    offset += leidos; // AVANCE CORRECTO: sumamos lo que realmente se leyó
    memset(buffer_texto, 0, TAM_BUFFER); 
}
    
    // 4. Obtener metadatos para verificar el tamaño lógico
    struct STAT stat;
    if (mi_stat_f(ninodo, &stat) == -1) {
        fprintf(stderr, "Error al obtener stat del inodo %d\n", ninodo);
    } else {
        // Mostramos información final por la salida de errores (STDERR_FILENO = 2)
        // Así no se mezcla con el contenido si se redirecciona a un fichero
        char info[128];
        sprintf(info, "\nTotal bytes leídos: %d\n", total_leidos);
        write(2, info, strlen(info));
        sprintf(info, "Tam. en bytes lógicos del inodo: %u\n", stat.tamEnBytesLog);
        write(2, info, strlen(info));
    }

    // 5. Desmontar el dispositivo
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}