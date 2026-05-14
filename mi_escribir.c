#include "directorios.h"

int main(int argc, char **argv) {
    // 1. Validación de argumentos
    if (argc != 5) {
        fprintf(stderr, "Error de sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return FALLO;
    }

    char *nombre_disco = argv[1];
    char *ruta = argv[2];
    char *texto = argv[3];
    unsigned int offset = atoi(argv[4]);
    int nbytes = strlen(texto);

    // 2. Montar el dispositivo
    if (bmount(nombre_disco) == FALLO) {
        fprintf(stderr, "Error: No se ha podido montar el disco %s\n", nombre_disco);
        return FALLO;
    }

    // 3. Llamar a mi_write (capa de directorios)
    // mi_write ya se encarga de buscar el inodo, gestionar la caché y llamar a mi_write_f
    printf("Escribiendo %d bytes en %s con offset %u...\n", nbytes, ruta, offset);
    
    int escritos = mi_write(ruta, texto, offset, nbytes);

    if (escritos < 0) {
        // El error ya debería haber sido gestionado/mostrado por mi_write o buscar_entrada
        // pero podemos capturar errores genéricos aquí
        fprintf(stderr, "Error al escribir en el fichero.\n");
        bumount();
        return FALLO;
    }

    // 4. Mostrar resultados
    printf("Bytes escritos: %d\n", escritos);

    // 5. Desmontar el dispositivo
    if (bumount() == FALLO) {
        return FALLO;
    }

    return EXITO;
}