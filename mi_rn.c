
#include "directorios.h"

int main(int argc, char **argv) {
    // Sintaxis: ./mi_rn <disco> </ruta/antiguo> <nuevo>
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: ./mi_rn <disco> </ruta/antiguo> <nuevo>\n");
        return FALLO;
    }

    char *disco = argv[1];
    char *camino_antiguo = argv[2];
    char *nombre_nuevo = argv[3];

    // Evitar renombrar la raíz absoluta
    if (strcmp(camino_antiguo, "/") == 0) {
        fprintf(stderr, "Error: No se puede renombrar el directorio raíz.\n");
        return FALLO;
    }

    // Montar el dispositivo virtual
    if (bmount(disco) == FALLO) {
        perror("Error en bmount");
        return FALLO;
    }

    // Ejecutar la función de renombrado
    int resultado = mi_rn(camino_antiguo, nombre_nuevo);
    if (resultado < 0) {
        if (resultado != FALLO) {
            mostrar_error_buscar_entrada(resultado);
        }
        bumount();
        return FALLO;
    }

    printf("Se ha renombrado con éxito.\n");

    // Desmontar el dispositivo virtual
    bumount();
    return EXITO;
}