
#include "directorios.h"

int main(int argc, char **argv)
{
    // Sintaxis: ./mi_mv <disco> </origen/nombre> </destino/>
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: ./mi_mv <disco> </origen/nombre> </destino/>\n");
        return FALLO;
    }

    char *disco = argv[1];
    char *camino_origen = argv[2];
    char *camino_destino = argv[3];

    // Restricción de seguridad básica
    if (strcmp(camino_origen, "/") == 0)
    {
        fprintf(stderr, "Error: No se puede mover el directorio raíz.\n");
        return FALLO;
    }

    // Montar el dispositivo virtual
    if (bmount(disco) == FALLO)
    {
        perror("Error en bmount");
        return FALLO;
    }

    // Ejecutar la función lógica de mover
    int resultado = mi_mv(camino_origen, camino_destino);
    if (resultado < 0)
    {
        if (resultado != FALLO)
        {
            mostrar_error_buscar_entrada(resultado);
        }
        bumount();
        return FALLO;
    }

    printf("Se ha movido el elemento con éxito.\n");

    // Desmontar el dispositivo virtual
    bumount();
    return EXITO;
}