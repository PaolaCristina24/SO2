#include "directorios.h"

int main(int argc, char **argv)
{
    char buffer[TAMBUFFER];
    memset(buffer, 0, TAMBUFFER);

    int modo_largo = 0;
    char *disco;
    char *ruta;

    // Comprobar sintaxis de los argumentos
    if (argc == 3) {
        // Modo normal (simple)
        disco = argv[1];
        ruta = argv[2];
    } else if (argc == 4 && strcmp(argv[1], "-l") == 0) {
        // Modo extendido (largo)
        modo_largo = 1;
        disco = argv[2];
        ruta = argv[3];
    } else {
        fprintf(stderr,
        "Sintaxis: ./mi_ls <disco> </ruta>\n"
        "          ./mi_ls -l <disco> </ruta>\n");
        return FALLO;
    }

    // Montar dispositivo
    if (bmount(disco) == FALLO) {
        perror("Error en bmount");
        return FALLO;
    }

    //Preparar los parámetros tipo y flag basándonos en los argumentos validados
    char tipo_esperado = (ruta[strlen(ruta) - 1] != '/') ? 'f' : 'd';
    char flag_lista = modo_largo ? 'l' : 's'; // 'l' para extendido, 's' para simple

    // Llamar a mi_dir con los parámetros que toca
    int nentradas = mi_dir(ruta, buffer, tipo_esperado, flag_lista);

    if (nentradas < 0) {
        if (nentradas == -10) {
            fprintf(stderr, "Error: la sintaxis no concuerda con el tipo.\n");
        } else {
            mostrar_error_buscar_entrada(nentradas);
        }
        bumount();
        return FALLO;
    }

    //Mostrar el resultado por pantalla
    if (modo_largo) {
        // Mejora del formato extendido, tenía un fallo y le pedí ayuda a gemini
        if (nentradas > 0 || tipo_esperado == 'f') {
            if (tipo_esperado == 'd') {
                printf("Total: %d\n", nentradas);
            }
            printf("Tipo\tPermisos\tmTime\t\t\tTamaño\tNombre\n");
            printf("--------------------------------------------------------------------------------------------\n");
            printf("%s", buffer);
        }
    } else {
        // Formato simple: muestra solo el buffer
        if (tipo_esperado == 'd') {
            printf("Total: %d\n", nentradas);
        }
        printf("%s", buffer);
    }

    //desmontamos el dispositivo
    bumount();

    return EXITO;
}