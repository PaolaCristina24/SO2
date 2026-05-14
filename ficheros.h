/**
 * @author Juana Luna
 * @author Paola Chacín
 * @author Yassin EL Gharsa
 */
#include "ficheros_basico.h"

struct STAT {
    unsigned char tipo;     // 'd' (directorio) o 'f' (fichero)
    unsigned char permisos; // Permisos (lectura, escritura...)
    unsigned int nlinks;    // Cantidad de enlaces
    unsigned int tamEnBytesLog; // Tamaño lógico del fichero
    unsigned int numBloquesOcupados; // Bloques físicos usados
    time_t atime; // Fecha último acceso
    time_t mtime; // Fecha última modificación
    time_t ctime; // Fecha creación
};

// Declaración de funciones
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);