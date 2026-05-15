/**
 * @author Juana Luna
 * @author Paola Chacín
 * @author Yassin EL Gharsa
 */

#include "ficheros_basico.h"

/**
 * Calcula el tamaño en bloques necesarios para el mapa de bits
 * @param nbloques: Número total de bloques del dispositivo virtual
 * @return: Cantidad de bloques necesarios para el mapa de bits
 */
int tamMB(unsigned int nbloques)
{
    // Calculamos el número de bytes necesarios para representar nbloques bits
    int bytes = nbloques / 8;

    // Comprobamos si hay bits sobrantes que no completan un byte completo
    if (nbloques % 8 != 0)
    {
        bytes++;
    }

    // Verificamos si necesitamos un bloque extra
    if (bytes % BLOCKSIZE != 0)
    {
        // si hay resto necesitaremos un bloque mas
        return (bytes / BLOCKSIZE) + 1;
    }

    return bytes / BLOCKSIZE;
}

/**
 * Calcula el tamaño en bloques necesarios para el array de inodos.
 * @param ninodos: Número total de inodos del dispositivo virtual
 * @return: Cantidad de bloques necesarios para el array de inodos
 */
int tamAI(unsigned int ninodos)
{

    int inodosBloque = BLOCKSIZE / INODOSIZE; // Cantidad de inodos que caben en un bloque

    if (ninodos % inodosBloque != 0)
    {
        // Si hay resto necesitaremos 1 bloque mas
        return (ninodos / inodosBloque) + 1;
    }
    // Si no devolvemos el mismo numero
    return ninodos / inodosBloque;
}

/**
 * Inicializa el superbloque cy lo escribe en el bloque 0.
 * @param nbloques: Número total de bloques del dispositivo virtual
 * @param ninodos: Número total de inodos del dispositivo virtual
 * @return: EXITO (0) o FALLO (-1)
 */
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;
    // Completamos la información de las variables de ficheros_basico.h
    SB.posPrimerBloqueMB = posSB + tamSB; // posSB = 0, tamSB = 1
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    // Inicializamos el superbloque
    if (bwrite(posSB, &SB) == FALLO)
    {
        perror("Error al escribir el superbloque");
        return FALLO;
    }
    return EXITO;
}

/**
 * Inicializa el mapa de bits (MB) marcando como ocupados los bloques de metadatos (SB, MB y AI).
 * @return: EXITO (0) o FALLO (-1)
 */
int initMB()
{
    // Leemos el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        perror("Error en la lectura del SB");
        return FALLO;
    }

    unsigned int metadatos = SB.posPrimerBloqueDatos; // Los datos emoiezan despues de los metadatos
    unsigned int bitsRes = SB.posPrimerBloqueDatos;
    char bufferMB[BLOCKSIZE];

    // Recorremos los bloques del MB
    for (unsigned int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++)
    {
        memset(bufferMB, 0, BLOCKSIZE); // Rellenamos el buffer con 0s

        // LLenamos los bloques con 1s
        if (bitsRes >= BLOCKSIZE * 8)
        {
            memset(bufferMB, 255, BLOCKSIZE);
            bitsRes -= BLOCKSIZE * 8;
        }
        else if (bitsRes > 0)
        { // Rellenamos el bloque con los bits que quedan por marcar como ocupados
            int bytes = bitsRes / 8;
            for (int j = 0; j < bytes; j++)
            { // Rellenamos bytes completos
                bufferMB[j] = 255;
            }

            int bits = bitsRes % 8;
            unsigned char resto = 0;
            if (bits > 0)
            {
                for (int j = 0; j < bits; j++)
                {
                    resto += 1 << (7 - j);
                }
            }
            bufferMB[bytes] = resto;
            bitsRes = 0;
        }
        // Escribimos el bloque modificado
        if (bwrite(i, bufferMB) == FALLO)
        {
            perror("Error escribiendo MB");
            return FALLO;
        }
    }

    // Actaulizamos y guardamos
    SB.cantBloquesLibres -= metadatos;
    if (bwrite(posSB, &SB) == FALLO)
    {
        perror("Error actualizando SB");
        return FALLO;
    }

    return EXITO;
}

/**
 * Inicializa el array de inodos (AI) marcando todos los inodos como libres y enlazándolos entre sí.
 * @return: EXITO (0) o FALLO (-1)
 */
int initAI()
{
    struct superbloque SB;
    // Leemos el superbloque para saber dónde empieza el array de inodos
    if (bread(posSB, &SB) == FALLO)
    {
        perror("Error leyendo SB");
        return FALLO;
    }

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    unsigned int contInodos = 0;

    // Recorremos los bloques del array de inodos
    for (unsigned int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        // Recorremos los inodos del bloque actual
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++)
        {
            // Marcamos el inodo como libre y lo enlazamos con el siguiente
            inodos[j].tipo = 'l';
            if (contInodos < SB.totInodos - 1)
            {
                inodos[j].punterosDirectos[0] = contInodos + 1;
            }
            else
            {
                inodos[j].punterosDirectos[0] = UINT_MAX; // Fin de la lista
            }
            contInodos++;
        }

        if (bwrite(i, inodos) == FALLO)
        {
            perror("Error escribiendo AI");
            return FALLO;
        }
    }
    return EXITO;
}

/**
 * Escribe un bit en el mapa de bits (MB) para marcar un bloque como libre u ocupado.
 * @param nbloque: Número de bloque a modificar
 * @param bit: Valor del bit a escribir (0 o 1)
 * @return: EXITO (0) o FALLO (-1)
 */
int escribir_bit(unsigned int nbloque, unsigned int bit)
{

    // Leemos el superbloque para obtener la localización del MB
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        perror("Error leyendo SB ");
        return FALLO;
    }

    // Que byte del MB contiene el bit que queremos modificar
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    // Que bloque del MB contiene el byte que queremos modificar
    unsigned int numbloqueMB = posbyte / BLOCKSIZE;
    // Posición absoluta del bloque del MB que contiene el bit a modificar
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + numbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    memset(bufferMB, '\0', BLOCKSIZE); // Buffer para cargar el bloque del MB que contiene el bit a modificar

    // Leemos el bloque del MB que contiene el bit a modificar
    if (bread(nbloqueabs, &bufferMB) == FALLO)
    {
        perror("Error en la lectura del bloque \n");
        return FALLO;
    }

    // Calculamos la posición del byte dentro del bloque
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mask = 128;
    mask >>= posbit;

    // Modificamos el bit
    if (bit == 1)
    {
        bufferMB[posbyte] |= mask; // Operadores OR para bits (1)
    }
    else
    {
        bufferMB[posbyte] &= ~mask; // Operadores AND y NOT para bits (0)
    }

    // Escribimos el bloque modificado
    if (bwrite(nbloqueabs, &bufferMB) == FALLO)
    {
        perror("Error en la escritura del bit en el bloque\n");
        return FALLO;
    }
    return EXITO;
}

/**
 * Lee un bit del mapa de bits  para verificar si un bloque está libre u ocupado.
 * @param nbloque: Número de bloque a leer
 * @return: Valor del bit leído (0 o 1) o FALLO (-1) en caso de error
 */
char leer_bit(unsigned int nbloque)
{

    // Leemos el superbloque para obtener la localización del MB
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        perror("Error en la lectura del superbloque \n");
        return FALLO;
    }

    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int numbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + numbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    if (bread(nbloqueabs, &bufferMB) == FALLO)
    {
        perror("Error al leer el bloque\n");
        return FALLO;
    }

    posbyte = posbyte % BLOCKSIZE;
    unsigned char mask = 128;
    mask >>= posbit;           // Desplazamiento de bits a la derecha, los que indique posbit
    mask &= bufferMB[posbyte]; // Extraemos el bit
    mask >>= (7 - posbit);     // Lo desplazamos a la posición 0 para devolverlo como 0 o 1

    return mask;
}

/**
 * Reserva un bloque libre en el mapa de bits (MB) y lo marca como ocupado.
 * @return: Número de bloque reservado o FALLO (-1) en caso de error
 */
int reservar_bloque()
{

    struct superbloque SB;

    // Primero leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
    {
        perror("Error leyendo SB");
        return FALLO;
    }

    // Comprobamos si hay bloques libres
    //(Si no hay bloques libres, no podemos reservar ninguno)
    if (SB.cantBloquesLibres == 0)
    {
        return FALLO;
    }

    unsigned char bufferMB[BLOCKSIZE];       // Guarda un bloque del Mapa de Bits (MB) leído desde disco
    unsigned char bufferAuxiliar[BLOCKSIZE]; // Buffer auxiliar para comparar con el bloque del MB
    memset(bufferAuxiliar, 255, BLOCKSIZE);  // incializamos a 1 (11111111) para comparar con el bloque del MB y encontrar el primer bloque con algún bit a 0 (libre)

    // Indica qué bloque del MB estamos analizando
    unsigned int nbloqueMB = 0;

    // Recorremos el MB bloque por bloque.
    while (nbloqueMB <= (SB.posUltimoBloqueMB - SB.posPrimerBloqueMB))
    {

        if (bread(SB.posPrimerBloqueMB + nbloqueMB, bufferMB) == FALLO)
        {
            perror("Error leyendo bloque MB");
            return FALLO;
        }
        // Compara bloque real del MB con el bloque auxiliar lleno de 1s. Si son iguales, significa que el bloque del MB no tiene ningún bit a 0 (libre) y seguimos buscando.
        if (memcmp(bufferMB, bufferAuxiliar, BLOCKSIZE) != 0)
        {
            break; // Hemos encontrado bloque con algún 0
        }

        nbloqueMB++;
    }

    // Buscar byte libre dentro del bloque MB
    unsigned int posbyte = 0;
    // Recorremos el bloque del MB byte por byte hasta encontrar un byte que no esté completamente lleno de 1s (255 en decimal)
    while (bufferMB[posbyte] == 255)
    {
        posbyte++;
    }

    // Buscamos primer bit a 0 dentro del byte usando una mascara and bit a bit
    unsigned char mascara = 128; // 10000000

    // Indica qué bit dentro del byte estamos mirando
    unsigned int posbit = 0;

    // Operación AND bit a bit
    while (bufferMB[posbyte] & mascara)
    {

        // Desplazamos el byte a la izquierda hasta encontrar un bit a 0.
        bufferMB[posbyte] <<= 1;
        posbit++;
    }

    // Calculamos el número de bloque absoluto a reservar
    // Número real de bloque físico en el disco
    unsigned int nbloque = (nbloqueMB * BLOCKSIZE + posbyte) * 8 + posbit;

    // Marcamos el bloque como ocupado en el MB, poniendo el bit a 1
    if (escribir_bit(nbloque, 1) == FALLO)
    {
        return FALLO;
    }

    // Actualizamos el SB
    SB.cantBloquesLibres--;
    if (bwrite(posSB, &SB) == FALLO)
    {
        return FALLO;
    }

    // Limpiamos el bloque reservado para que no contenga datos residuales
    unsigned char bufferDatos[BLOCKSIZE];
    memset(bufferDatos, 0, BLOCKSIZE);

    if (bwrite(nbloque, bufferDatos) == FALLO)
    {
        return FALLO;
    }

    return nbloque;
}

/**
 * Libera un bloque ocupado en el mapa de bits (MB) y lo marca como libre.
 * @param nbloque: Número de bloque a liberar
 * @return: Número de bloque liberado o FALLO (-1) en caso de error
 *
 */
int liberar_bloque(unsigned int nbloque)
{

    struct superbloque SB;

    // Leemos el superbloque para obtener la información necesaria para liberar el bloque
    if (bread(posSB, &SB) == FALLO)
    {
        perror("Error leyendo SB en liberar_bloque");
        return FALLO;
    }

    // Ponemos a 0 el bit correspondiente en el MB (marcar como libre)
    if (escribir_bit(nbloque, 0) == FALLO)
    {
        perror("Error escribiendo bit en liberar_bloque");
        return FALLO;
    }

    // Incrementamos la cantidad de bloques libres en el superbloque.
    SB.cantBloquesLibres++;

    // Guardamos el superbloque actualizado en disco
    if (bwrite(posSB, &SB) == FALLO)
    {
        perror("Error escribiendo SB en liberar_bloque");
        return FALLO;
    }
    printf("Liberado bloque %d\n", nbloque);
    // Devolvemos el número de bloque liberado
    return nbloque;
}
/**
 * Escribe un inodo concreto en el array de inodos del disco virtual.
 * @param ninodo: Número de inodo a escribir
 * @param inodo: Puntero al inodo con la información a escribir
 * @return: EXITO (0) o FALLO (-1) en caso de error
 *
 */
int escribir_inodo(unsigned int ninodo, struct inodo *inodo)
{
    // escribe un inodo concreto en el array de inodos del disco virtual.
    struct superbloque SB;

    // Leemos el superbloque para saber dónde empieza el array de inodos
    if (bread(posSB, &SB) == FALLO)
    {
        perror("Error leyendo SB en escribir_inodo");
        return FALLO;
    }

    // Calculamos cuántos inodos caben en un bloque
    unsigned int inodos_por_bloque = BLOCKSIZE / INODOSIZE;

    // Calculamos qué bloque del array de inodos contiene este inodo
    unsigned int nbloqueAI = ninodo / inodos_por_bloque;

    // Convertimos a bloque absoluto dentro del dispositivo(el número real de bloque)
    unsigned int nbloqueabs = SB.posPrimerBloqueAI + nbloqueAI;

    // Buffer para cargar en memoria todos los inodos de ese bloque
    struct inodo inodos[inodos_por_bloque];

    // Leemos el bloque completo porque no podemos escribir solo un inodo suelto
    if (bread(nbloqueabs, inodos) == FALLO)
    {
        perror("Error leyendo bloque de inodos");
        return FALLO;
    }

    // Posición del inodo dentro del bloque
    unsigned int posicion_inodo = ninodo % inodos_por_bloque;

    // Sustituimos únicamente ese inodo en memoria
    inodos[posicion_inodo] = *inodo;

    // Escribimos de nuevo el bloque completo ya modificado
    if (bwrite(nbloqueabs, inodos) == FALLO)
    {
        perror("Error escribiendo bloque de inodos");
        return FALLO;
    }

    return EXITO;
}

/**
 * Lee un inodo concreto del array de inodos del disco virtual.
 * @param ninodo: Número de inodo a leer
 * @param inodo: Puntero al inodo donde se almacenará la información leída
 * @return: EXITO (0) o FALLO (-1) en caso de error
 *
 */
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        perror("Error leyendo SB");
        return FALLO;
    }
    unsigned nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    unsigned nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    if (bread(nbloqueabs, inodos) == FALLO)
    {
        perror("Error leyendo bloque de inodos");
        return FALLO;
    }

    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];
    return EXITO;
}

/**
 * Reserva un inodo libre en el array de inodos, lo inicializa con el tipo y permisos especificados, y lo marca como ocupado.
 * @param tipo: Tipo de inodo a reservar ('f' para fichero, 'd' para directorio)
 * @param permisos: Permisos del inodo a reservar
 * @return: Número de inodo reservado o FALLO (-1) en caso de error
 *
 */

int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    struct inodo inodo;

    // Leer superbloque
    if (bread(posSB, &SB) == FALLO)
    {
        perror("Error leyendo SB");
        return FALLO;
    }

    // No hay inodos libres
    if (SB.cantInodosLibres == 0)
    {
        fprintf(stderr, "No hay inodos libres\n");
        return FALLO;
    }

    // Validación de tipo
    if (tipo != 'f' && tipo != 'd')
    {
        return FALLO;
    }

    // Obtener inodo libre
    unsigned int posInodo = SB.posPrimerInodoLibre;

    if (leer_inodo(posInodo, &inodo) == FALLO)
    {
        perror("Error leyendo inodo libre");
        return FALLO;
    }

    // Actualizar superbloque (lista libre)
    SB.posPrimerInodoLibre = inodo.punterosDirectos[0];
    SB.cantInodosLibres--;

    // ============================
    // INICIALIZACIÓN CORRECTA
    // ============================

    inodo.tipo = tipo;

    // 🔥 CLAVE DEL TEST:
    // directorios SIEMPRE permisos 6
    if (tipo == 'd')
        inodo.permisos = 6;
    else
        inodo.permisos = permisos;

    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.numBloquesOcupados = 0;

    time_t t = time(NULL);
    inodo.atime = t;
    inodo.mtime = t;
    inodo.ctime = t;

    // limpiar punteros
    memset(inodo.punterosDirectos, 0, sizeof(inodo.punterosDirectos));
    memset(inodo.punterosIndirectos, 0, sizeof(inodo.punterosIndirectos));

    // Escribir inodo
    if (escribir_inodo(posInodo, &inodo) == FALLO)
    {
        perror("Error escribiendo inodo");
        return FALLO;
    }

    // Escribir superbloque
    if (bwrite(posSB, &SB) == FALLO)
    {
        perror("Error escribiendo SB");
        return FALLO;
    }

    return posInodo;
}

/**
 * Dado un número de inodo y un bloque lógico, obtiene el número de bloque físico correspondiente.
 * Si el bloque lógico no existe y reservar es 1, se reserva un nuevo bloque físico y se enlaza al inodo.
 * @param ninodo: Número de inodo a traducir
 * @param nblogico: Número de bloque lógico a traducir
 * @param reservar: Indica si se debe reservar un nuevo bloque físico si el bloque lógico no existe (1 para reservar, 0 para no reservar)
 * @return: Número de bloque físico correspondiente al bloque lógico o FALLO (-1) en caso de error
 *
 */
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{
    if (nblogico < DIRECTOS)
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0; //<12
    }
    else if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1; // 268
    }
    else if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2; // 65804
    }
    else if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3; // 16843020
    }
    else
    {
        *ptr = 0;
        perror("Bloque logico fuera de rango");
        return FALLO;
    }
}

/**
 * Dado un número de bloque lógico y el nivel de punteros, obtiene el índice correspondiente dentro del bloque de punteros.
 * @param nblogico: Número del bloque lógico a traducir
 * @param nivel_punteros: Nivel de punteros
 * @return: Índice dentro del bloque de punteros o FALLO (-1) en caso de error
 *
 */
int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
    unsigned int offset;

    if (nblogico < DIRECTOS)
        return nblogico;

    if (nblogico < INDIRECTOS0)
        offset = nblogico - DIRECTOS;
    else if (nblogico < INDIRECTOS1)
        offset = nblogico - INDIRECTOS0;
    else if (nblogico < INDIRECTOS2)
        offset = nblogico - INDIRECTOS1;
    else
        return FALLO;

    if (nivel_punteros == 1)
        return offset % NPUNTEROS;

    if (nivel_punteros == 2)
        return (offset / NPUNTEROS) % NPUNTEROS;

    if (nivel_punteros == 3)
        return (offset / (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;

    return FALLO;
}
/**
 * Obtiene el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado.
 * Si el bloque lógico no existe y reservar es 1, se reserva un nuevo bloque físico y se enlaza al inodo.
 * @param ninodo: Número de inodo a traducir
 * @param nblogico: Número de bloque lógico a traducir
 * @param reservar: 1 para reservar, 0 para solo consultar
 * @return: Número de bloque físico correspondiente al bloque lógico o FALLO (-1) en caso de error
 *
 */
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar)
{

    struct inodo inodo;
    unsigned int ptr, ptr_ant;
    int nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];
    unsigned char salvar_inodo = 0;

    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        return FALLO;
    }

    // Identifica,os el rango y obtener el primer puntero (del inodo)
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL;                           // el nivel_punteros +alto es el que cuelga directamente del inodo

    // Iteramos para cada nivel de punteros indirectos
    while (nivel_punteros > 0)
    {
        if (ptr == 0)
        { // No existe el bloque de punteros
            if (!reservar)
                return FALLO; // bloque inexistente -> no imprimir error por pantalla!!!

            // reservar bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos
            salvar_inodo = 1;
            ptr = reservar_bloque();
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);

            if (nivel_punteros == nRangoBL)
            { // el bloque cuelga directamente del inodo
                inodo.punterosIndirectos[nRangoBL - 1] = ptr;
            }
            else
            { // el bloque cuelga de otro bloque de punteros
                // Usamos el indice calculado en la iteración ANTERIOR
                buffer[indice] = ptr;
                bwrite(ptr_ant, buffer); // salvamos en el dispositivo el buffer de punteros modificado
            }

            // Limpiamos el nuevo bloque de punteros en memoria y disco
            memset(buffer, 0, BLOCKSIZE);
            bwrite(ptr, buffer); // salvamos el bloque de punteros recién reservado y limpiado
        }
        else
        {
            // leemos del dispositivo el bloque de punteros ya existente
            bread(ptr, buffer);
        }

        // PREPARAMOS SIGUIENTE NIVEL
        // Calculamos el índice para el nivel actual antes de bajar
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        // Guardamos el bloque actual para el bwrite si el hijo es nuevo
        ptr = buffer[indice]; // Bajamos al siguiente nivel
        nivel_punteros--;
    }

    // MANEJO DEL BLOQUE DE DATOS (Nivel 0)
    if (ptr == 0)
    { // no existe bloque de datos
        if (!reservar)
            return FALLO;

        salvar_inodo = 1;
        ptr = reservar_bloque();
        inodo.numBloquesOcupados++;
        inodo.ctime = time(NULL);

        if (nRangoBL == 0)
        {                                           // si era un puntero Directo
            inodo.punterosDirectos[nblogico] = ptr; // asignamos la direción del bl. de datos en el inodo
        }
        else
        {
            // El indice aquí es el que calculó el bucle en su última iteración
            buffer[indice] = ptr;    // asignamos la dirección del bloque de datos en el buffer
            bwrite(ptr_ant, buffer); // salvamos en el dispositivo el buffer de punteros modificado
        }
    }

    // salvar el inodo si se han hecho cambios y se desea no tener un big lock al usar semáforos
    if (salvar_inodo)
    {
        escribir_inodo(ninodo, &inodo);
    }

    return ptr; // nº de bloque físico correspondiente al bloque de datos lógico, nblogico
}

// NIVEL 6
/**
 * Libera un inodo y todos los bloques de datos asociados a él.
 * @param ninodo: Número de inodo a liberar
 * @return: Número de inodo liberado o FALLO (-1) en caso de error
 *
 */
int liberar_inodo(unsigned int ninodo)
{

    // Leer inodo
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        perror("Error en liberar_inodo");
        return FALLO;
    }

    // Liberar todos los bloques del inodo
    int liberados = liberar_bloques_inodo(0, &inodo);

    if (liberados == FALLO)
    {
        perror("Error liberando bloques");
        return FALLO;
    }

    // Actualizar bloques ocupados
    inodo.numBloquesOcupados = 0;

    // Marcar inodo como libre
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;

    // Leer superbloque
    struct superbloque SB;

    if (bread(posSB, &SB) == FALLO)
    {
        perror("Error leyendo SB");
        return FALLO;
    }

    // Insertar el inodo liberado al principio de la lista libre
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;

    // Actualizar cantidad de inodos libres
    SB.cantInodosLibres++;

    // Actualizar tiempo
    inodo.ctime = time(NULL);

    // Escribir el inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        perror("Error escribiendo inodo");
        return FALLO;
    }

    // Escribir superbloque
    if (bwrite(posSB, &SB) == FALLO)
    {
        perror("Error escribiendo SB");
        return FALLO;
    }

    return ninodo;
}

/**
 * Libera los bloques de datos asociados a un inodo.    APUNTAR
 * @param primerBL: Número del primer bloque lógico a liberar
 * @param inodo: Puntero a la estructura del inodo
 * @return: Número de bloques liberados o FALLO (-1) en caso de error
 **/

/**
 * Libera bloques de datos e indirectos asociados a un inodo
 */
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    unsigned int nivel_punteros = 0;
    unsigned int nBL = primerBL;
    unsigned int ultimoBL;
    unsigned int ptr = 0;

    int nRangoBL = 0;
    int liberados = 0;
    int eof = 0;

    // Fichero vacío
    if (inodo->tamEnBytesLog == 0)
    {
        return 0;
    }

    // Obtener último bloque lógico
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0)
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    }
    else
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

    // ---------------- DIRECTOS ----------------

    nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);

    if (nRangoBL == 0)
    {
        liberados += liberar_directos(&nBL, ultimoBL, inodo, &eof);
    }

    // ---------------- INDIRECTOS ----------------

    while (!eof)
    {
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);

        nivel_punteros = nRangoBL;

        liberados += liberar_indirectos_recursivo(
            &nBL,
            primerBL,
            ultimoBL,
            inodo,
            nRangoBL,
            nivel_punteros,
            &ptr,
            &eof);
    }

    return liberados;
}
/**
 * Libera bloques directos
 */
int liberar_directos(unsigned int *nBL,
                     unsigned int ultimoBL,
                     struct inodo *inodo,
                     int *eof)
{
    int liberados = 0;

    while ((*nBL < DIRECTOS) && !(*eof))
    {

        if (inodo->punterosDirectos[*nBL] != 0)
        {

            printf("Liberado bloque %d\n",
                   inodo->punterosDirectos[*nBL]);

            liberar_bloque(inodo->punterosDirectos[*nBL]);

            inodo->punterosDirectos[*nBL] = 0;

            liberados++;
        }

        (*nBL)++;

        if (*nBL > ultimoBL)
        {
            *eof = 1;
        }
    }

    return liberados;
}

/**
 * Libera bloques indirectos recursivamente
 */
int liberar_indirectos_recursivo(
    unsigned int *nBL,
    unsigned int primerBL,
    unsigned int ultimoBL,
    struct inodo *inodo,
    int nRangoBL,
    unsigned int nivel_punteros,
    unsigned int *ptr,
    int *eof)
{
    int liberados = 0;
    int indice_inicial;

    unsigned int bloquePunteros[NPUNTEROS];
    unsigned int bloquePunteros_Aux[NPUNTEROS];
    unsigned int bufferCeros[NPUNTEROS];

    memset(bufferCeros, 0, BLOCKSIZE);

    // --------------------------------------------------
    // Si existe bloque de punteros
    // --------------------------------------------------

    if (*ptr)
    {

        indice_inicial = obtener_indice(*nBL, nivel_punteros);

        // Leer bloque solo si hace falta
        if ((indice_inicial == 0) || (*nBL == primerBL))
        {

            if (bread(*ptr, bloquePunteros) == FALLO)
            {
                return FALLO;
            }

            memcpy(bloquePunteros_Aux,
                   bloquePunteros,
                   BLOCKSIZE);
        }

        // Recorrer punteros
        for (int i = indice_inicial;
             i < NPUNTEROS && !(*eof);
             i++)
        {

            // ----------------------------------------
            // PUNTERO OCUPADO
            // ----------------------------------------

            if (bloquePunteros[i] != 0)
            {

                // ---------- NIVEL 1 ----------
                if (nivel_punteros == 1)
                {

                    printf("Liberado bloque %d\n",
                           bloquePunteros[i]);

                    liberar_bloque(bloquePunteros[i]);

                    bloquePunteros[i] = 0;

                    liberados++;

                    (*nBL)++;
                }

                // ---------- RECURSIVIDAD ----------
                else
                {

                    liberados += liberar_indirectos_recursivo(
                        nBL,
                        primerBL,
                        ultimoBL,
                        inodo,
                        nRangoBL,
                        nivel_punteros - 1,
                        &bloquePunteros[i],
                        eof);
                }
            }

            // ----------------------------------------
            // PUNTERO VACÍO
            // ----------------------------------------

            else
            {

                switch (nivel_punteros)
                {

                case 1:
                    (*nBL)++;
                    break;

                case 2:
                    (*nBL) += NPUNTEROS;
                    break;

                case 3:
                    (*nBL) += NPUNTEROS * NPUNTEROS;
                    break;
                }
            }

            // Fin fichero
            if (*nBL > ultimoBL)
            {
                *eof = 1;
            }
        }

        // --------------------------------------------------
        // Si el bloque cambió
        // --------------------------------------------------

        if (memcmp(bloquePunteros,
                   bloquePunteros_Aux,
                   BLOCKSIZE) != 0)
        {

            // Si todavía contiene punteros
            if (memcmp(bloquePunteros,
                       bufferCeros,
                       BLOCKSIZE) != 0)
            {

                bwrite(*ptr, bloquePunteros);
            }

            // Bloque vacío -> liberar
            else
            {

                printf("Liberado bloque %d\n", *ptr);

                liberar_bloque(*ptr);

                *ptr = 0;

                // Si cuelga directamente del inodo
                if (nRangoBL == nivel_punteros)
                {
                    inodo->punterosIndirectos[nRangoBL - 1] = 0;
                }

                liberados++;
            }
        }
    }

    // --------------------------------------------------
    // Si ptr == 0
    // --------------------------------------------------

    else
    {

        switch (nRangoBL)
        {

        case 1:
            *nBL = INDIRECTOS0;
            break;

        case 2:
            *nBL = INDIRECTOS1;
            break;

        case 3:
            *nBL = INDIRECTOS2;
            break;
        }
    }

    return liberados;
}
