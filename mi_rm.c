#include "directorios.h"

int main(int argc, char **argv) {

    // 1. Comprobamos la sintaxis (Corregido "Sintexis" por "Sintaxis")
    if (argc != 3) {
        fprintf(stderr, RED "Sintaxis: ./mi_rm <disco> <ruta>\n" RESET);
        return FALLO;
    }



    // 2. Montamos el disco
    if (bmount(argv[1]) == FALLO) {
        perror("Error en bmount");
        return FALLO;
    }

    // 3. Ejecutamos mi_unlink y capturamos su código de error específico
    int error = mi_unlink(argv[2]);
    
    if (error < 0) {
        // En lugar de perror, usamos la función de la práctica para ver el error exacto
        mostrar_error_buscar_entrada(error);
        bumount();
        return FALLO;
    }

    // 4. Desmontamos el disco
    if (bumount() == FALLO) {
        return FALLO;
    }

    return EXITO;
}