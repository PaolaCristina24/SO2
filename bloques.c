
/**
 * @author Juana Luna
 * @author Paola Chacín
 * @author Yassin EL Gharsa
 */
#include "bloques.h"
#include "semaforo_mutex_posix.h"

static sem_t *mutex;
static unsigned int inside_sc = 0; 
//Variable que indica el descriptor del dispositivo virtual
static int descriptor = 0; 

/**
 * Monta el dispositivo virtual
 * @param camino: ruta del dispositivo virtual a montar
 * @return descriptor del dispositivo montado o FALLO en caso de error
*/
int bmount(const char *camino) {
    //Llamamos a umask para asegurarnos de que los permisos se apliquen correctamente al crear el archivo
    umask(000);
    // Abrimos con lectura/escritura y creación si no existe
    // Los permisos 0666 significan rw-rw-rw-
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    
    //Comprobamos si el descriptor es válido
    if (descriptor == FALLO) {
        perror("Error en bmount"); 
        return FALLO;
    }

    mutex = initSem();
    if (mutex == NULL || mutex == SEM_FAILED) {
        fprintf(stderr, "Error en bmount: No se pudo enlazar el semáforo mutex.\n");
        return FALLO;
    }
    
    return descriptor;
}
/**
 * Desmonta el dispositivo virtual
 * @return EXITO en caso de éxito o FALLO en caso de error
 */
int bumount() {
    if (descriptor == FALLO) return FALLO;

    if (close(descriptor) == FALLO) {
        perror("Error en bumount");
        return FALLO;
    }
    descriptor = FALLO;
    
    // USAMOS LA FUNCIÓN REAL DE TU LIBRERÍA DE SEMÁFOROS
    // Si tu función destruirSem() cierra el semáforo global, la llamamos aquí:
    destruirSem(); 
    mutex = NULL; 

    return EXITO; 
}


/**
 * Escribe un bloque de datos en el dispositivo virtual
 * @param nbloque: bloque fisico a escribir
 * @param buf: puntero al buffer que contiene los datos a escribir
 * @return cantidad de bytes escritos o FALLO en caso de error
*/
int bwrite(unsigned int nbloque, const void *buf) {
    //Calculamos el desplazamiento
    off_t desplazamiento = (off_t)nbloque * BLOCKSIZE; //Por si * supera el rango de int

    // Posicionamos el puntero del fichero
    // SEEK_SET indica que el desplazamiento es respecto al inicio del fichero
    if (lseek(descriptor, desplazamiento, SEEK_SET) == FALLO) {
        perror("Error en lseek de bwrite");
        return FALLO;
    }

    // Escribimos el bloque
    ssize_t bytes_escritos = write(descriptor, buf, BLOCKSIZE);

    //Control de errores
    if (bytes_escritos == FALLO) {
        perror("Error en write de bwrite");
        return FALLO;
    }

    // Devolvemos la cantidad de bytes escritos (debería ser BLOCKSIZE)
    return (int)bytes_escritos; //casting int para devolver un valor entero 
}

/**
 * Lee un bloque de datos del dispositivo virtual
 * @param nbloque: bloque fisico a leer
 * @param buf: puntero al buffer donde almacenar los datos leidos
 * @return cantidad de bytes leidos o FALLO en caso de error
 */
int bread(unsigned int nbloque, void *buf) {
    // Calculamos el desplazamiento 
    off_t desplazamiento = (off_t)nbloque * BLOCKSIZE; 

    //Movemos el puntero del fichero
    if (lseek(descriptor, desplazamiento, SEEK_SET) == FALLO) {
        perror("Error en lseek de bread");
        return FALLO;
    }

    // Leemos el bloque
    ssize_t bytes_leidos = read(descriptor, buf, BLOCKSIZE);// 

    //Control de errores
    if (bytes_leidos == FALLO) {
        perror("Error en read de bread");
        return FALLO;
    }

    // Devolvemos la cantidad de bytes leídos (debería ser BLOCKSIZE)
    return (int)bytes_leidos;
}

// =========================================================================
// ENVOLTORIOS DE EXCLUSIÓN MUTUA REENTRANTES (En bloques.c)
// =========================================================================

void mi_waitSem() 
{
    if (!inside_sc) { 
        // Llama a la función del Paso 1 pasando nuestro semáforo global
        waitSem(mutex); 
    }
    inside_sc++; 
}

void mi_signalSem() 
{
    if (inside_sc > 0) {
        inside_sc--; 
        if (!inside_sc) { 
            // Llama a la función del Paso 1 pasando nuestro semáforo global
            signalSem(mutex); 
        }
    }
}