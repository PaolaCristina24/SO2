/**
 * @author Juana Luna
 * @author Paola Chacín
 * @author Yassin EL Gharsa
 */

#include "ficheros.h"

/**
 * Escribe el contenido de un buffer en un fichero
 * @param ninodo: Número de inodo del fichero
 * @param buf_original: Buffer con el contenido a escribir
 * @param offset: POsición dentro del fichero donde se comenzará a escribir
 * @param nbytes: Número de bytes a escribir
 * @return: Número de bytes escritos o FALLO (-1) en caso de error
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
    // Entramos en sección crítica para evitar escrituras concurrentes
    mi_waitSem();

    struct inodo inodo;
    int resultado = 0; // Variable única para el retorno del flujo

    // Leer inodo
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        resultado = FALLO;
        goto salida_write_f;
    }
    
    // Comprobar permisos de escritura
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, "\033[31mNo hay permisos de escritura\n\033[0m");
        resultado = FALLO;
        goto salida_write_f;
    }

    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    unsigned char buf_bloque[BLOCKSIZE];
    int nbfisico;

    unsigned int bytes_escritos = 0;
    const unsigned char *buf = (const unsigned char *)buf_original;

    // CASO 1: todo en un solo bloque
    if (primerBL == ultimoBL) {
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (nbfisico == FALLO) {
            resultado = FALLO;
            goto salida_write_f;
        }
        if (bread(nbfisico, buf_bloque) == FALLO) {
            resultado = FALLO;
            goto salida_write_f;
        }
        memcpy(buf_bloque + desp1, buf, nbytes);
        if (bwrite(nbfisico, buf_bloque) == FALLO) {
            resultado = FALLO;
            goto salida_write_f;
        }
        bytes_escritos = nbytes;
    } else {
        // Caso 2: necesitamos escribir en varios bloques
        // FASE 1: primer bloque
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (nbfisico == FALLO) {
            resultado = FALLO;
            goto salida_write_f;
        }
        if (bread(nbfisico, buf_bloque) == FALLO) {
            resultado = FALLO;
            goto salida_write_f;
        }
        unsigned int bytes_f1 = BLOCKSIZE - desp1;
        memcpy(buf_bloque + desp1, buf, bytes_f1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) {
            resultado = FALLO;
            goto salida_write_f;
        }
        bytes_escritos += bytes_f1;

        // FASE 2: bloques intermedios
        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++) {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 1);
            if (nbfisico == FALLO) {
                resultado = FALLO;
                goto salida_write_f;
            }
            if (bwrite(nbfisico, buf + bytes_escritos) == FALLO) {
                resultado = FALLO;
                goto salida_write_f;
            }
            bytes_escritos += BLOCKSIZE;
        }

        // FASE 3: último bloque
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (nbfisico == FALLO) {
            resultado = FALLO;
            goto salida_write_f;
        }
        if (bread(nbfisico, buf_bloque) == FALLO) {
            resultado = FALLO;
            goto salida_write_f;
        }
        unsigned int bytes_f3 = desp2 + 1;
        memcpy(buf_bloque, buf + bytes_escritos, bytes_f3);
        if (bwrite(nbfisico, buf_bloque) == FALLO) {
            resultado = FALLO;
            goto salida_write_f;
        }
        bytes_escritos += bytes_f3;
    }

    // =========================================================================
    // ACTUALIZAR METADATOS DEL INODO DE FORMA ATÓMICA REENTRANTE
    // =========================================================================
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        resultado = FALLO;
        goto salida_write_f;
    }

    if (offset + nbytes > inodo.tamEnBytesLog) {
        inodo.tamEnBytesLog = offset + nbytes;
    }

    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        resultado = FALLO;
        goto salida_write_f;
    }

    // Si todo ha ido bien, guardamos el recuento de bytes reales
    resultado = bytes_escritos;

salida_write_f:
    mi_signalSem(); // Desbloqueo garantizado sin fugas de semáforos
    return resultado;
}
/**
 * Lee el contenido de un fichero en un buffer
 * @param ninodo: Número de inodo del fichero
 * @param buf_original: Buffer donde se almacenará el contenido leído
 * @param offset: Posición dentro del fichero desde donde se comenzará a leer
 * @param nbytes: Número de bytes a leer
 * @return: Número de bytes leídos o FALLO (-1) en caso de error
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){

    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
        return FALLO;

    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
        return FALLO;
    }

    // Como no se puede leer mas del EOF, devolvemos 0
    if (offset >= inodo.tamEnBytesLog)
        return 0;

    // pretende leer más allá de EOF
    if ((offset + nbytes) > inodo.tamEnBytesLog)
    {
        nbytes = inodo.tamEnBytesLog - offset; // leemos sólo los bytes que podemos desde el offset hasta EOF
    }

    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    unsigned char buf_bloque[BLOCKSIZE];
    int nbfisico;
    unsigned int bytes_leidos = 0;

    if (primerBL == ultimoBL)
    {
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO)
        {
            if (bread(nbfisico, buf_bloque) == FALLO)
                return FALLO;
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        else
        {
            // HUECO: Llenar con ceros
            memset(buf_original, 0, nbytes);
        }
        bytes_leidos = nbytes;
    }
    else
    {
        // Fase 1: Primer bloque
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO)
        {
            if (bread(nbfisico, buf_bloque) == FALLO)
                return FALLO;
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        else
        {
            // HUECO
            memset(buf_original, 0, BLOCKSIZE - desp1);
        }
        bytes_leidos += (BLOCKSIZE - desp1);

        // Fase 2: Bloques intermedios
        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++)
        {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 0);
            if (nbfisico != FALLO)
            {
                if (bread(nbfisico, buf_bloque) == FALLO)
                    return FALLO;
                memcpy((unsigned char *)buf_original + bytes_leidos, buf_bloque, BLOCKSIZE);
            }
            else
            {
                // HUECO: Muy importante para archivos de 480MB
                memset((unsigned char *)buf_original + bytes_leidos, 0, BLOCKSIZE);
            }
            bytes_leidos += BLOCKSIZE;
        }

        // Fase 3: Último bloque
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nbfisico != FALLO)
        {
            if (bread(nbfisico, buf_bloque) == FALLO)
                return FALLO;
            memcpy((unsigned char *)buf_original + bytes_leidos, buf_bloque, desp2 + 1);
        }
        else
        {
            // HUECO
            memset((unsigned char *)buf_original + bytes_leidos, 0, desp2 + 1);
        }
        bytes_leidos += (desp2 + 1);
    }

    // Solo protegemos esta parte crítica porque modifica el inodo
    mi_waitSem();

    if (leer_inodo(ninodo, &inodo) == FALLO){
        mi_signalSem();
        return FALLO;
    }

    inodo.atime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO){
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();

    return bytes_leidos;
}
/*
 *Ontiene metainformación de u ficero
 * @param ninodo: Número de inodo del fichero
 * @param p_stat: Puntero a la estructura de metainformación
 * @return: EXITO (0) o FALLO (-1) en caso de error
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
        return FALLO;

    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    return EXITO;
}

/*
 * Cambia los permisos de un fichero
 * @param ninodo: Número de inodo del fichero
 * @param permisos: Nuevos permisos a establecer 
 * @return: EXITO (0) o FALLO (-1) en caso de error
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    // Entramos en sección crítica porque modificamos el inodo
    mi_waitSem();

    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    // Guardamos el inodo actualizado
    if (escribir_inodo(ninodo, &inodo) == FALLO){
        mi_signalSem();
        return FALLO;
    }

    // Salimos de sección crítica
    mi_signalSem();

    return EXITO;
}

/**
 * Trunca un fichero a un tamaño específico.
 * @param ninodo: Número de inodo del fichero
 * @param nbytes: Nuevo tamaño del fichero
 * @return: Número de bloques liberados o FALLO (-1) en caso de error
 */

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    struct inodo inodo;

    // =========================
    // LEER INODO
    // =========================

    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        perror("Error leyendo inodo");
        return FALLO;
    }

    // =========================
    // COMPROBAR PERMISOS
    // =========================

    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr,
                "Error: el inodo no tiene permisos de escritura\n");

        return FALLO;
    }

    // =========================
    // COMPROBAR TAMAÑO
    // =========================

    if (nbytes > inodo.tamEnBytesLog)
    {
        fprintf(stderr,
                "Error: nbytes mayor que tamEnBytesLog\n");

        return FALLO;
    }

    // =========================
    // CALCULAR PRIMER BL
    // =========================

    unsigned int primerBL;

    if (nbytes % BLOCKSIZE == 0)
    {
        primerBL = nbytes / BLOCKSIZE;
    }
    else
    {
        primerBL = nbytes / BLOCKSIZE + 1;
    }

    // =========================
    // LIBERAR BLOQUES
    // =========================

    int bloquesLiberados =
        liberar_bloques_inodo(primerBL, &inodo);

    if (bloquesLiberados == FALLO)
    {
        perror("Error liberando bloques");
        return FALLO;
    }

    // =========================
    // ACTUALIZAR METADATOS
    // =========================

    inodo.tamEnBytesLog = nbytes;

    inodo.numBloquesOcupados -= bloquesLiberados;

    time_t ahora = time(NULL);

    inodo.mtime = ahora;
    inodo.ctime = ahora;

    // =========================
    // ESCRIBIR INODO
    // =========================

    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        perror("Error escribiendo inodo");
        return FALLO;
    }

    return bloquesLiberados;
}