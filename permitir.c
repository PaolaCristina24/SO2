#include "ficheros.h"

int main(int argc, char *argv[]) {
    // 1. Validación de sintaxis
    if (argc != 4) {
        fprintf(stderr, "Error de sintaxis: ./permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        fprintf(stderr, "Nota: <permisos> debe ser un valor entre 0 y 7\n");
        return -1;
    }

    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    unsigned char permisos = (unsigned char) atoi(argv[3]);

    // 2. Montar el dispositivo
    if (bmount(nombre_dispositivo) == -1) {
        fprintf(stderr, "Error al montar el dispositivo\n");
        return -1;
    }

    // 3. Llamada a mi_chmod_f
    // Esta función actualizará el campo 'permisos' en la estructura del inodo
    if (mi_chmod_f(ninodo, permisos) == -1) {
        fprintf(stderr, "Error al cambiar los permisos del inodo %d\n", ninodo);
        bumount();
        return -1;
    }

    printf("Permisos del inodo %d cambiados a: %d\n", ninodo, permisos);

    // 4. Desmontar el dispositivo
    if (bumount() == -1) {
        fprintf(stderr, "Error al desmontar el dispositivo\n");
        return -1;
    }

    return 0;
}