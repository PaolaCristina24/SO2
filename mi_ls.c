#include "directorios.h"

int main(int argc, char **argv)
{
    char buffer[TAMBUFFER];
    memset(buffer, 0, TAMBUFFER);

    int modo_largo = 0;
    char *disco;
    char *ruta;

    // --------------------------------------------------
    // Comprobar sintaxis
    // --------------------------------------------------
    if (argc == 3) {
        // modo normal
        disco = argv[1];
        ruta = argv[2];

    } else if (argc == 4 && strcmp(argv[1], "-l") == 0) {
        // modo extendido
        modo_largo = 1;
        disco = argv[2];
        ruta = argv[3];

    } else {
        fprintf(stderr,
        "Sintaxis: ./mi_ls <disco> </ruta>\n"
        "          ./mi_ls -l <disco> </ruta>\n");
        return FALLO;
    }

    // --------------------------------------------------
    // Montar dispositivo
    // --------------------------------------------------
    if (bmount(disco) == FALLO) {
        perror("Error en bmount");
        return FALLO;
    }

    // --------------------------------------------------
    // Llamar a mi_dir()
    // buffer contendrá el listado
    // --------------------------------------------------
    int nentradas = mi_dir(ruta, buffer);

    if (nentradas < 0) {
        mostrar_error_buscar_entrada(nentradas);
        bumount();
        return FALLO;
    }

    // --------------------------------------------------
    // Mostrar resultado
    // --------------------------------------------------
    if (modo_largo) {

        // Si hay entradas mostramos cabeceras
        if (nentradas > 0) {
            printf("Total: %d\n", nentradas);
            printf("Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n");
            printf("---------------------------------------------------------------\n");
        }

        printf("%s", buffer);

    } else {

        // Formato simple: solo nombres
        printf("Total: %d\n", nentradas);
        printf("%s", buffer);
    }

    // --------------------------------------------------
    // Desmontar dispositivo
    // --------------------------------------------------
    bumount();

    return EXITO;
}