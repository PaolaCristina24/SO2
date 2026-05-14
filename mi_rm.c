#include "directorios.h"

int main(int argc, char **argv) {

    //Comprobamos la sintaxis
    if (argc != 3) {
        fprintf(stderr, RED "Sintexis incorrecta:./mi_rm disco /ruta\n" RESET);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO) {
        perror("Error en bmount");
        return FALLO;
    }

    //Ejecutamos unlink
    if (mi_unlink(argv[2]) == FALLO) {
        perror("Error en mi_unlink");
        bumount();
        return FALLO;
    }

    bumount();

    return EXITO;
}