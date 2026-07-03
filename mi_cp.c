#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "directorios.h"
//COPIAR UN FICHERO O DIRECTORIO: 
//La cantidad de inodos y bloques tiene que haber aumentado correspondientemente 
//(mostrar superbloque antes y después). Los stat de los inodos originales han de coincidir 
//(salvo en los sellos de tiempo) con los nuevos creados.
// FUNCION AUX
void mostrar_estado_superbloque(const char *momento);
int copiar_recursivo(const char *camino_origen, const char *directorio_destino);

int main(int argc, char *argv[]) {
    // VALIDACIÓN DE ARGUMENTOS
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <disco> </origen/nombre_origen> </destino/>\n", argv[0]);
        return FALLO;
    }

    const char *disco = argv[1];
    const char *camino_origen = argv[2];
    const char *directorio_destino = argv[3];

    // Montar el dispositivo
    if (bmount(disco) == FALLO) {
        fprintf(stderr, "Error al montar el disco %s\n", disco);
        return FALLO;
    }

    //  MOSTRAR SUPERBLOQUE ANTES
    mostrar_estado_superbloque("ANTES");

    //  EJECUTAR LA COPIA RECURSIVA
    printf("\nIniciando proceso de copia...\n");
    int resultado = copiar_recursivo(camino_origen, directorio_destino);
    
    if (resultado == EXITO) {
        printf("Copia realizada con éxito.\n\n");
    } else {
        fprintf(stderr, "Error durante el proceso de copia.\n\n");
    }

    //  MOSTRAR SUPERBLOQUE DESPUÉS
    mostrar_estado_superbloque("DESPUÉS");

    // Desmontar
    bumount();
    return (resultado == EXITO) ? 0 : -1;
}

/**
 * Muestra los bloques e inodos libres, se lee el superbloque
 */
void mostrar_estado_superbloque(const char *momento) {
    struct superbloque sb;
    if (bread(0, &sb) != FALLO) {
        printf("=== SUPERBLOQUE (%s) ===\n", momento);
        printf("Bloques libres: %u\n", sb.cantBloquesLibres);
        printf("Inodos libres:  %u\n", sb.cantInodosLibres);
        printf("===============================\n");
    }
}

/**
 * Función recursiva que copia tanto ficheros como directorios (mirar el contenido)
 */
int copiar_recursivo(const char *camino_origen, const char *directorio_destino) {
    unsigned int p_inodo_dir_orig = 0, p_inodo_origen = 0, p_entrada_orig = 0;
    struct inodo inodo_origen;

    // Obtener el inodo del pincipal
    if (buscar_entrada(camino_origen, &p_inodo_dir_orig, &p_inodo_origen, &p_entrada_orig, 0, 0) < 0) {
        fprintf(stderr, "Error: El origen '%s' no existe.\n", camino_origen);
        return FALLO;
    }

    if (leer_inodo(p_inodo_origen, &inodo_origen) == FALLO) return FALLO;

    
    char origen_limpio[PATH_MAX];
    strncpy(origen_limpio, camino_origen, sizeof(origen_limpio) - 1);
    size_t len_orig = strlen(origen_limpio);
    if (len_orig > 1 && origen_limpio[len_orig - 1] == '/') {
        origen_limpio[len_orig - 1] = '\0'; 
    }

    // Extrae el nombre real
    const char *nombre = strrchr(origen_limpio, '/');
    nombre = (nombre == NULL) ? origen_limpio : nombre + 1;

    // Contruccion del camino destino para el elemento 
    char camino_destino_completo[PATH_MAX + TAMNOMBRE];
    if (directorio_destino[strlen(directorio_destino) - 1] == '/') {
        snprintf(camino_destino_completo, sizeof(camino_destino_completo), "%s%s", directorio_destino, nombre);
    } else {
        snprintf(camino_destino_completo, sizeof(camino_destino_completo), "%s/%s", directorio_destino, nombre);
    }

    // Si es un fichero regular
    if (inodo_origen.tipo == 'f') {
        return mi_cp(p_inodo_origen, camino_destino_completo, inodo_origen.permisos);
    }

    // Si el origen es un directorio
    if (inodo_origen.tipo == 'd') {
        // Forzar barra inclinada final para que buscar_entrada cree un directorio real 
        char destino_dir_crear[PATH_MAX + TAMNOMBRE + 4];
        snprintf(destino_dir_crear, sizeof(destino_dir_crear), "%s/", camino_destino_completo);

        unsigned int p_inodo_dir_dest = 0, p_inodo_destino = 0, p_entrada_dest = 0;
        int error = buscar_entrada(destino_dir_crear, &p_inodo_dir_dest, &p_inodo_destino, &p_entrada_dest, 1, inodo_origen.permisos);
        if (error < 0) return FALLO;

        // Recorre secuencialmente las entradas del directorio origen
        int num_entradas = inodo_origen.tamEnBytesLog / sizeof(struct entrada);
        struct entrada ent;

        for (int i = 0; i < num_entradas; i++) {
            if (mi_read_f(p_inodo_origen, &ent, i * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) {
                return FALLO;
            }

            // Evita los bucles infinitos
            if (strcmp(ent.nombre, ".") == 0 || strcmp(ent.nombre, "..") == 0) {
                continue;
            }

            // Construye el camino del hijo usando el origen ya limpio
            char nuevo_origen_hijo[PATH_MAX + TAMNOMBRE];
            snprintf(nuevo_origen_hijo, sizeof(nuevo_origen_hijo), "%s/%s", origen_limpio, ent.nombre);

            // Llamada recursiva pasando el nuevo directorio contenedor
            if (copiar_recursivo(nuevo_origen_hijo, destino_dir_crear) == FALLO) {
                return FALLO;
            }
        }
        return EXITO;
    }

    return FALLO;
}