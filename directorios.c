/**
 * @author Juana Luna
 * @author Paola Chacín
 * @author Yassin EL Gharsa
 */



#include "directorios.h"

/**
 * Extrae el primer componente del camino, el tipo (directorio o fichero) y el resto del camino
 * @param camino El camino a analizar
 * @param inicial El primer componente del camino (sin el '/')
 * @param final El resto del camino (incluyendo el '/')
 * @param tipo 'd' si el primer componente es un directorio, 'f' si es un fichero
 * @return EXITO si se ha extraído correctamente, o un código de error negativo
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{

    // El camino debe empezar por '/'
    if (camino[0] != '/')
    {
        return ERROR_CAMINO_INCORRECTO;
    }

    // Busca el siguiente '/' después del primer carácter
    char *p = strchr(camino + 1, '/');

    // Si se encuentra un '/', entonces el primer componente es un directorio, sino es un fichero
    if (p != NULL)
    {
        // Hay otro '/', por tanto inicial es directorio
        int len = p - (camino + 1);

        strncpy(inicial, camino + 1, len); // Copia el primer componente (entre el primer '/' y el siguiente '/')
        inicial[len] = '\0';               // Asegura que la cadena inicial esté terminada en null

        strcpy(final, p); // copiamos desde ese '/'

        *tipo = 'd';
    }
    else
    {
        // No hay más '/', es fichero final

        strcpy(inicial, camino + 1);
        strcpy(final, "");

        *tipo = 'f';
    }

    return EXITO;
}

// Usamos mi read y no bread porqeu un directorio es un fichero cuyos datos son estructuras entrada, así que se accede como fichero lógico.

/**
 * Busca una entrada en un directorio dado su camino parcial, y opcionalmente la reserva si no existe
 * @param camino_parcial El camino parcial a buscar
 * @param p_inodo_dir Puntero al inodo del directorio actual
 * @param p_inodo Puntero al inodo de la entrada encontrada o reservada (relativo al array de inodos)
 * @param p_entrada Puntero al número de entrada dentro del directorio donde se encuentra la entrada encontrada o reservada
 * @param reservar Si es 0, solo se busca la entrada. Si es 1, se reserva una nueva entrada si no existe.
 * @param permisos Permisos a asignar a la nueva entrada si se reserva (solo se tiene en cuenta si reservar es 1)
 * @return EXITO si se ha encontrado o reservado la entrada correctamente, o un código de error negativo
 */


int buscar_entrada(const char *camino_parcial,
                   unsigned int *p_inodo_dir,
                   unsigned int *p_inodo,
                   unsigned int *p_entrada,
                   char reservar,
                   unsigned char permisos)
{
    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;

    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial) + 1];
    char tipo;

    int cant_entradas_inodo;
    int num_entrada_inodo = 0;
    int encontrada = 0;

    memset(inicial, 0, sizeof(inicial));
    memset(final, 0, sizeof(final));
    memset(&entrada, 0, sizeof(struct entrada));

    // =========================
    // CASO RAÍZ "/"
    // =========================

    if (strcmp(camino_parcial, "/") == 0)
    {
        if (bread(posSB, &SB) == FALLO)
        {
            perror("Error leyendo superbloque");
            return FALLO;
        }

        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;

        return EXITO;
    }

    // =========================
    // EXTRAER CAMINO
    // =========================

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO)
    {
        return ERROR_CAMINO_INCORRECTO;
    }

#if DEBUG1
    fprintf(stderr,
            "[buscar_entrada() → inicial: %s, final: %s, reservar: %d]\n",
            inicial,
            final,
            reservar);
#endif

    // =========================
    // LEER INODO DIRECTORIO
    // =========================

    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO)
    {
        return FALLO;
    }

    // permiso lectura directorio
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }

    // =========================
    // BUSCAR ENTRADA
    // =========================

    cant_entradas_inodo =
        inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    while (num_entrada_inodo < cant_entradas_inodo)
    {
        if (mi_read_f(*p_inodo_dir,
                      &entrada,
                      num_entrada_inodo * sizeof(struct entrada),
                      sizeof(struct entrada)) < 0)
        {
            return FALLO;
        }

        if (strcmp(inicial, entrada.nombre) == 0)
        {
            encontrada = 1;
            break;
        }

        num_entrada_inodo++;
    }

    // =========================
    // SI NO EXISTE
    // =========================

    if (!encontrada)
    {
        // modo consulta
        if (reservar == 0)
        {
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        }

        // no se puede crear dentro de fichero
        if (inodo_dir.tipo == 'f')
        {
            return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
        }

        // permiso escritura
        if ((inodo_dir.permisos & 2) != 2)
        {
            return ERROR_PERMISO_ESCRITURA;
        }

        // directorios intermedios
        if (strcmp(final, "") != 0 &&
            strcmp(final, "/") != 0 &&
            tipo == 'd')
        {
            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
        }

        // =========================
        // CREAR NUEVA ENTRADA
        // =========================

        strcpy(entrada.nombre, inicial);

        if (tipo == 'd')
        {
            if (strcmp(final, "/") != 0)
            {
                return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
            }

            entrada.ninodo = reservar_inodo('d', permisos);
        }
        else
        {
            entrada.ninodo = reservar_inodo('f', permisos);
        }

        if (entrada.ninodo == FALLO)
        {
            return FALLO;
        }

#if DEBUG1
        fprintf(stderr,
                "[buscar_entrada() → reservado inodo %d tipo %c permisos %d para %s]\n",
                entrada.ninodo,
                tipo,
                permisos,
                inicial);
#endif

        if (mi_write_f(*p_inodo_dir,
                       &entrada,
                       num_entrada_inodo * sizeof(struct entrada),
                       sizeof(struct entrada)) == FALLO)
        {
            liberar_inodo(entrada.ninodo);
            return FALLO;
        }

#if DEBUG1
        fprintf(stderr,
                "[buscar_entrada() → creada entrada %s, %d]\n",
                entrada.nombre,
                entrada.ninodo);
#endif
    }

    // =========================
    // FINAL DEL CAMINO
    // =========================

    if (strcmp(final, "") == 0 ||
        strcmp(final, "/") == 0)
    {
        // si ya existe y queremos reservar
        if (encontrada && reservar == 1)
        {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;

        return EXITO;
    }

    // =========================
    // RECURSIVIDAD
    // =========================

    *p_inodo_dir = entrada.ninodo;

    return buscar_entrada(final,
                           p_inodo_dir,
                           p_inodo,
                           p_entrada,
                           reservar,
                           permisos);
}



void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {
    case -2:
        fprintf(stderr, RED "Error: Camino incorrecto.\n");
        break;
    case -3:
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n");
        break;
    case -4:
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n");
        break;
    case -5:
        fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n");
        break;
    case -6:
        fprintf(stderr, RED "Error: Permiso denegado de escritura.\n");
        break;
    case -7:
        fprintf(stderr, RED "Error: El archivo ya existe.\n");
        break;
    case -8:
        fprintf(stderr, RED "Error: No es un directorio.\n");
        break;
    }
    fprintf(stderr, WHITE);
}

/**
 * Crea un nuevo fichero o directorio según el camino especificado, con los permisos indicados
 * @param camino El camino del nuevo fichero o directorio a crear
 * @param permisos Los permisos a asignar al nuevo fichero o directorio (bit 4 para lectura, bit 2 para escritura, bit 1 para ejecución)
 * @return EXITO si se ha creado correctamente, o un código de error negativo si no se ha podido crear
 */
int mi_creat(const char *camino, unsigned char permisos)
{
    // Comprobamos que el modo de permisos es válido (entre 0 y 7)
    if (permisos > 7 || permisos < 0)
    {
        perror("Permisos inválidos");
        return FALLO;
    }

    unsigned int p_inodo_dir = 0; // raíz
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos); // reservar = 1 para crear si no existe
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error); // Mostrar mensaje de error específico
        return FALLO;
    }

    return EXITO;
}
/** Lista el contenido de un directorio dado su camino, guardando los nombres de las entradas en el buffer proporcionado
 * @param camino El camino del directorio a listar
 * @param buffer El buffer donde se guardarán los nombres de las entradas
 * @return El número de entradas listadas o un código de error negativo si no se ha podido listar
 */
int mi_dir(const char *camino, char *buffer, char tipo, char flag)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    struct inodo inodo;
    struct entrada entrada;
    char tmp[512];

    buffer[0] = '\0';

    // 1. Buscar la entrada correspondiente al camino
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (error < 0) return error;

    // 2. Leer el inodo del elemento solicitado
    if (leer_inodo(p_inodo, &inodo) == FALLO) return FALLO;

    // 3. Validación de sintaxis: Comprobar que el tipo de inodo coincide con la ruta
    if (inodo.tipo != tipo) {
        return -10; // Error de concordancia de sintaxis capturado en mi_ls.c
    }

    // 4. Comprobar permisos de lectura
    if ((inodo.permisos & 4) != 4) {
        return ERROR_PERMISO_LECTURA;
    }

    // Definición de colores ANSI (¡Solo afectarán al nombre!)
    #define COLOR_DIR   "\033[33m"  // Naranja / Amarillo para directorios
    #define COLOR_FIC   "\033[36m"  // Cian para ficheros
    #define COLOR_RESET "\033[0m"   // Blanco estándar

    // =========================================================================
    // CASO A: LA RUTA ES UN FICHERO INDIVIDUAL
    // =========================================================================
    if (inodo.tipo == 'f') {
        const char *nombre_fichero = strrchr(camino, '/') ? strrchr(camino, '/') + 1 : camino;

        if (flag == 'l') { // Modo extendido
            struct tm *tm;
            tm = localtime(&inodo.mtime);
            
            // Construimos los metadatos base
            sprintf(buffer, "%c\t", inodo.tipo);
            strcat(buffer, (inodo.permisos & 4) ? "r" : "-");
            strcat(buffer, (inodo.permisos & 2) ? "w" : "-");
            strcat(buffer, (inodo.permisos & 1) ? "x" : "-");
            
            // Construcción del tiempo manual solicitada en el guion
            sprintf(tmp, "\t%d-%02d-%02d %02d:%02d:%02d", 
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, 
                    tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);

            // Tamaño y Nombre con color
            sprintf(tmp, "\t%d\t%s%s%s\n", inodo.tamEnBytesLog, COLOR_FIC, nombre_fichero, COLOR_RESET);
            strcat(buffer, tmp);
        } else {
            // Modo simple: solo nombre coloreado
            sprintf(buffer, "%s%s%s\n", COLOR_FIC, nombre_fichero, COLOR_RESET);
        }
        return 1;
    }

    // =========================================================================
    // CASO B: LA RUTA ES UN DIRECTORIO (Recorrer entradas)
    // =========================================================================
    int nentradas = inodo.tamEnBytesLog / sizeof(struct entrada);

    for (int i = 0; i < nentradas; i++)
    {
        if (mi_read_f(p_inodo, &entrada, i * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) {
            return FALLO;
        }
        
        struct inodo inodo_entrada;
        if (leer_inodo(entrada.ninodo, &inodo_entrada) == FALLO) {
            return FALLO;
        }

        // Asignamos el color dependiendo de lo que sea la entrada hija
        char color_nombre[16];
        if (inodo_entrada.tipo == 'd') {
            strcpy(color_nombre, COLOR_DIR);
        } else {
            strcpy(color_nombre, COLOR_FIC);
        }

        if (flag == 'l') { // Modo extendido para directorios
            struct tm *tm;
            tm = localtime(&inodo_entrada.mtime);
            
            sprintf(tmp, "%c\t", inodo_entrada.tipo);
            strcat(buffer, tmp);
            
            strcat(buffer, (inodo_entrada.permisos & 4) ? "r" : "-");
            strcat(buffer, (inodo_entrada.permisos & 2) ? "w" : "-");
            strcat(buffer, (inodo_entrada.permisos & 1) ? "x" : "-");
            
            sprintf(tmp, "\t%d-%02d-%02d %02d:%02d:%02d", 
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, 
                    tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);

            sprintf(tmp, "\t%d\t%s%s%s\n", inodo_entrada.tamEnBytesLog, color_nombre, entrada.nombre, COLOR_RESET);
            strcat(buffer, tmp);
        } else {
            // Modo simple para directorios: exclusivamente los nombres coloreados con su salto de línea
            sprintf(tmp, "%s%s%s\n", color_nombre, entrada.nombre, COLOR_RESET);
            strcat(buffer, tmp);
        }
    }

    return nentradas;
}

/**  Cambia los permisos de un fichero o directorio dado su camino
 * @param camino El camino del fichero o directorio al que se le quieren cambiar los permisos
 * @param permisos Los nuevos permisos a asignar (bit 4 para lectura, bit 2 para escritura, bit 1 para ejecución)
 * @return EXITO si se han cambiado los permisos correctamente, o un código de error negativo si no se han podido cambiar
 */

int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    // Buscar ruta
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);

    if (error < 0) {
        return error;
    }

    // Cambiar permisos del inodo encontrado
    return mi_chmod_f(p_inodo, permisos);
}

/** Obtiene los datos del inodo asociado a una ruta
 * @param camino El camino del fichero o directorio del que se quieren obtener los datos
 * @param p_stat Puntero a la estructura donde se almacenarán los datos
 * @return EXITO si se han obtenido los datos correctamente, o un código de error negativo si no se han podido obtener
 */
int mi_stat(const char *camino, struct STAT *p_stat)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    // Buscar la ruta
    int error = buscar_entrada(camino,
                               &p_inodo_dir,
                               &p_inodo,
                               &p_entrada,
                               0,
                               0);

    if (error < 0)
    {
        return error;
    }

    // Obtener metadata del inodo
    if (mi_stat_f(p_inodo, p_stat) == FALLO)
    {
        return FALLO;
    }
  //RETURN NUMERO INODO
    return (int)p_inodo;
}

//###############NIVEL 9 ################################

//Función para escribir contenido en un fichero, se busca la entrada dela camino con buscar entrada
#if (USARCACHE == 2 || USARCACHE == 3)
    static struct UltimaEntrada UltimasEntradas[CACHE_SIZE];
#elif (USARCACHE == 1)
    static struct UltimaEntrada UltimaEntradaEscritura;
#endif

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int encontrado = -1;

    // --- Se busca la entrada del camino ---
#if (USARCACHE > 0)
    #if (USARCACHE == 1)
        if (strcmp(UltimaEntradaEscritura.camino, camino) == 0) {
            p_inodo = UltimaEntradaEscritura.p_inodo;
            encontrado = 0;
        }
    #elif (USARCACHE == 2 || USARCACHE == 3)
        for (int i = 0; i < CACHE_SIZE; i++) {
            if (strcmp(UltimasEntradas[i].camino, camino) == 0) {
                p_inodo = UltimasEntradas[i].p_inodo;
                encontrado = i;
                #if (USARCACHE == 3) // Actualizar sello de tiempo para LRU
                    gettimeofday(&UltimasEntradas[i].ultima_consulta, NULL);
                #endif
                break;
            }
        }
    #endif
#endif

    // --- Si no está se busca ---
    if (encontrado == -1) {
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
        if (error < 0) {
            mostrar_error_buscar_entrada(error);
            return error;
        }

        // --- Actualiza el cache ---
#if (USARCACHE == 1)
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
#elif (USARCACHE == 3)
        // Buscar el hueco libre o la entrada más antigua (LRU)
        int indice_lru = 0;
        struct timeval min_time;
        gettimeofday(&min_time, NULL); // Tiempo actual

        for (int i = 0; i < CACHE_SIZE; i++) {
            // Si hay un hueco vacío (camino vacío)
            if (UltimasEntradas[i].camino[0] == '\0') {
                indice_lru = i;
                break;
            }
            // Si no, buscamos el que tenga el tiempo menor
            if (UltimasEntradas[i].ultima_consulta.tv_sec < min_time.tv_sec ||
               (UltimasEntradas[i].ultima_consulta.tv_sec == min_time.tv_sec && 
                UltimasEntradas[i].ultima_consulta.tv_usec < min_time.tv_usec)) {
                min_time = UltimasEntradas[i].ultima_consulta;
                indice_lru = i;
            }
        }
        // Reemplazar la entrada más antigua
        strcpy(UltimasEntradas[indice_lru].camino, camino);
        UltimasEntradas[indice_lru].p_inodo = p_inodo;
        gettimeofday(&UltimasEntradas[indice_lru].ultima_consulta, NULL);
#endif
    }

    // --- ESCRITURA FINAL ---
    return mi_write_f(p_inodo, buf, offset, nbytes);
}

int mi_read(const char *camino,
            void *buf,
            unsigned int offset,
            unsigned int nbytes)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int encontrado = 0;

    // =========================
    // BUSCAR EN CACHÉ
    // =========================

#if (USARCACHE == 1)

    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaEscritura.p_inodo;
        encontrado = 1;
    }

#endif

    // =========================
    // SI NO ESTÁ EN CACHÉ
    // =========================

    if (!encontrado)
    {
        int error = buscar_entrada(camino,
                                   &p_inodo_dir,
                                   &p_inodo,
                                   &p_entrada,
                                   0,
                                   0);

        if (error < 0)
        {
            return error;
        }

#if (USARCACHE == 1)

        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;

#endif
    }

    // =========================
    // LEER FICHERO
    // =========================

    int bytes_leidos = mi_read_f(p_inodo,
                                 buf,
                                 offset,
                                 nbytes);

    if (bytes_leidos < 0)
    {
        return FALLO;
    }

    return bytes_leidos;
}

//nivel 10

int mi_link(const char *camino1,
            const char *camino2)
{
    unsigned int p_inodo_dir1 = 0;
    unsigned int p_inodo1 = 0;
    unsigned int p_entrada1 = 0;

    struct inodo inodo1;

    // =========================
    // BUSCAR camino1
    // =========================

    int error = buscar_entrada(camino1,
                               &p_inodo_dir1,
                               &p_inodo1,
                               &p_entrada1,
                               0,
                               0);

    if (error < 0)
    {
        return error;
    }

    // =========================
    // LEER INODO ORIGINAL
    // =========================

    if (leer_inodo(p_inodo1, &inodo1) == FALLO)
    {
        return FALLO;
    }

    // =========================
    // COMPROBAR QUE ES FICHERO
    // =========================

    if (inodo1.tipo != 'f')
    {
        return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
    }

    // =========================
    // COMPROBAR PERMISO LECTURA
    // =========================

    if ((inodo1.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }

    // =========================
    // CREAR camino2
    // =========================

    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;

    error = buscar_entrada(camino2,
                           &p_inodo_dir2,
                           &p_inodo2,
                           &p_entrada2,
                           1,
                           6);

    if (error < 0)
    {
        return error;
    }

    // =========================
    // LEER ENTRADA CREADA
    // =========================

    struct entrada entrada2;

    if (mi_read_f(p_inodo_dir2,
                  &entrada2,
                  p_entrada2 * sizeof(struct entrada),
                  sizeof(struct entrada)) == FALLO)
    {
        return FALLO;
    }

    // =========================
    // HACER LINK AL MISMO INODO
    // =========================

    entrada2.ninodo = p_inodo1;

    if (mi_write_f(p_inodo_dir2,
                   &entrada2,
                   p_entrada2 * sizeof(struct entrada),
                   sizeof(struct entrada)) == FALLO)
    {
        return FALLO;
    }

    // =========================
    // LIBERAR INODO RESERVADO
    // =========================

    if (liberar_inodo(p_inodo2) == FALLO)
    {
        return FALLO;
    }

    // =========================
    // ACTUALIZAR NLINKS
    // =========================

    inodo1.nlinks++;

    inodo1.ctime = time(NULL);

    if (escribir_inodo(p_inodo1, &inodo1) == FALLO)
    {
        return FALLO;
    }

    return EXITO;
}


int mi_unlink(const char *camino)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    struct inodo inodo;
    struct entrada ultimaEntrada;

    // =========================
    // BUSCAR ENTRADA
    // =========================

    int error = buscar_entrada(camino,
                               &p_inodo_dir,
                               &p_inodo,
                               &p_entrada,
                               0,
                               0);

    if (error < 0)
    {
        return error;
    }

    // =========================
    // LEER INODO
    // =========================

    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        return FALLO;
    }

    // =========================
    // SI ES DIRECTORIO:
    // COMPROBAR QUE ESTÁ VACÍO
    // =========================

    if (inodo.tipo == 'd' &&
        inodo.tamEnBytesLog > 0)
    {
        fprintf(stderr,
                RED "El directorio no está vacío\n" RESET);

        return FALLO;
    }

    // =========================
    // LEER INODO DEL DIRECTORIO PADRE
    // =========================

    struct inodo inodo_dir;

    if (leer_inodo(p_inodo_dir, &inodo_dir) == FALLO)
    {
        return FALLO;
    }

    // Número total de entradas
    int num_entradas =
        inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    // =========================
    // SI NO ES LA ÚLTIMA ENTRADA
    // =========================

    if (p_entrada != (num_entradas - 1))
    {
        // Leer última entrada
        if (mi_read_f(p_inodo_dir,
                      &ultimaEntrada,
                      (num_entradas - 1) * sizeof(struct entrada),
                      sizeof(struct entrada)) == FALLO)
        {
            return FALLO;
        }

        // Escribirla en la posición borrada
        if (mi_write_f(p_inodo_dir,
                       &ultimaEntrada,
                       p_entrada * sizeof(struct entrada),
                       sizeof(struct entrada)) == FALLO)
        {
            return FALLO;
        }
    }

    // =========================
    // TRUNCAR DIRECTORIO PADRE
    // =========================

    if (mi_truncar_f(p_inodo_dir,
                     (num_entradas - 1) * sizeof(struct entrada)) == FALLO)
    {
        return FALLO;
    }

    // =========================
    // ACTUALIZAR NLINKS
    // =========================

    inodo.nlinks--;

    if (inodo.nlinks == 0)
    {
        // Liberar inodo
        if (liberar_inodo(p_inodo) == FALLO)
        {
            return FALLO;
        }
    }
    else
    {
        // Actualizar ctime
        inodo.ctime = time(NULL);

        if (escribir_inodo(p_inodo, &inodo) == FALLO)
        {
            return FALLO;
        }
    }

    return EXITO;
} 

    // =========================
    // MEJORAS NIVEL 10
    // =========================
int mi_rn(const char *camino_antiguo, const char *nombre_nuevo) {
    // Validación de la misma sintaxis
    int len_antiguo = strlen(camino_antiguo);
    int len_nuevo = strlen(nombre_nuevo);
    
    int antiguo_es_dir = (camino_antiguo[len_antiguo - 1] == '/');
    int nuevo_es_dir = (nombre_nuevo[len_nuevo - 1] == '/');
    
    if (antiguo_es_dir != nuevo_es_dir) {
        fprintf(stderr, "Error: antiguo y nuevo han de ser del mismo tipo (ambos ficheros o ambos directorios).\n");//
        return FALLO;
    }

    // Aisla la ruta del camino padre y el nuevo nombre
    char camino_padre[512];
    char nombre_antiguo[60];
    memset(camino_padre, 0, sizeof(camino_padre));
    memset(nombre_antiguo, 0, sizeof(nombre_antiguo));

    // Busca la última barra que separa el directorio padre de la entrada
    // Si es un directorio la / no importa y se busca la penúltima
    char *ultima_barra = strrchr(camino_antiguo, '/');
    if (antiguo_es_dir) {
        // Temporalmente truncamos la barra final para encontrar la barra que separa al padre
        char copia_camino[512];
        strcpy(copia_camino, camino_antiguo);
        copia_camino[len_antiguo - 1] = '\0';
        ultima_barra = strrchr(copia_camino, '/');
        
        strncpy(camino_padre, camino_antiguo, (ultima_barra - copia_camino) + 1);
        strcpy(nombre_antiguo, ultima_barra + 1);
        // Le devolvemos la barra al nombre antiguo porque es un directorio
        strcat(nombre_antiguo, "/");
    } else {
        strncpy(camino_padre, camino_antiguo, (ultima_barra - camino_antiguo) + 1);
        strcpy(nombre_antiguo, ultima_barra + 1);
    }

    // Se construye el camino completo nuevo para verificar que NO exista ya 
    //En esta parte tuve ayuda de gemini porque no se me ocurrio verificarlo y los test me daban error
    char camino_nuevo_completo[512];
    sprintf(camino_nuevo_completo, "%s%s", camino_padre, nombre_nuevo);

    unsigned int p_inodo_dir_tmp = 0, p_inodo_tmp = 0, p_entrada_tmp = 0;
    // Se busca con reservar = 0. Si buscar_entrada devuelve >= 0, significa que el nombre NUEVO ya existe.
    if (buscar_entrada(camino_nuevo_completo, &p_inodo_dir_tmp, &p_inodo_tmp, &p_entrada_tmp, 0, 0) >= 0) {
        fprintf(stderr, "Error: El nombre nuevo '%s' ya existe en este directorio.\n", nombre_nuevo);
        return FALLO; 
    }

    // Para proteger la modificación usé la exclusión mutua
    mi_waitSem();

    // Buscamos la entrada del antiguo para obtener el inodo del padre (p_inodo_dir) y el nº de entrada (p_entrada)
    unsigned int p_inodo_dir = 0, p_inodo_antiguo = 0, p_entrada_antiguo = 0;
    int error = buscar_entrada(camino_antiguo, &p_inodo_dir, &p_inodo_antiguo, &p_entrada_antiguo, 0, 0);
    if (error < 0) {
        mi_signalSem();
        return error; // Error: No existe el fichero/directorio antiguo
    }

    // Se la entrada directamente desde el inodo del padre
    struct entrada entrada_padre;
    int offset = p_entrada_antiguo * sizeof(struct entrada);
    
    if (mi_read_f(p_inodo_dir, &entrada_padre, offset, sizeof(struct entrada)) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    // Limpiamos el nombre antiguo y copiamos el nuevo (sin /'si es un directorio, se almacena solo el texto plano)
    memset(entrada_padre.nombre, 0, sizeof(entrada_padre.nombre));
    
    if (nuevo_es_dir) {
        // Se copia quitando la / final 
        strncpy(entrada_padre.nombre, nombre_nuevo, len_nuevo - 1);
    } else {
        strcpy(entrada_padre.nombre, nombre_nuevo);
    }

    //Escribir la entrada modificada de vuelta al disco
    if (mi_write_f(p_inodo_dir, &entrada_padre, offset, sizeof(struct entrada)) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    //Liberar el semáforo
    mi_signalSem();
    return EXITO;
}    
int mi_mv(const char *camino_origen, const char *camino_destino)
{
    unsigned int p_inodo_dir_orig = 0;
    unsigned int p_inodo_orig = 0;
    unsigned int p_entrada_orig = 0;

    struct inodo inodo_orig;

    // =========================================================================
    // COMPROBAR SINTAXIS DEL DESTINO (Debe ser un directorio terminado en '/')
    // =========================================================================

    int len_des = strlen(camino_destino);
    if (camino_destino[len_des - 1] != '/')
    {
        fprintf(stderr, "Error: El camino de destino debe ser un directorio (acabado en '/').\n");
        return FALLO;
    }

    // =========================================================================
    // BUSCAR ELEMENTO EN ORIGEN
    // =========================================================================

    int error = buscar_entrada(camino_origen,
                               &p_inodo_dir_orig,
                               &p_inodo_orig,
                               &p_entrada_orig,
                               0,
                               0);

    if (error < 0)
    {
        return error; // Error: No existe el origen
    }

    // =========================================================================
    // LEER INODO ORIGINAL Y COMPROBAR PERMISOS
    // =========================================================================

    if (leer_inodo(p_inodo_orig, &inodo_orig) == FALLO)
    {
        return FALLO;
    }

    if ((inodo_orig.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }

    // =========================================================================
    // EXTRAER EL NOMBRE LIMPIO DEL ELEMENTO A MOVER
    // =========================================================================

    char nombre_elemento[60];
    memset(nombre_elemento, 0, sizeof(nombre_elemento));

    int len_ori = strlen(camino_origen);
    int origen_es_dir = (camino_origen[len_ori - 1] == '/');

    char *ultima_barra = strrchr(camino_origen, '/');
    if (origen_es_dir)
    {
        // Si es directorio, ignoramos la barra final para extraer el nombre
        char copia_origen[512];
        strcpy(copia_origen, camino_origen);
        copia_origen[len_ori - 1] = '\0';
        ultima_barra = strrchr(copia_origen, '/');
        strcpy(nombre_elemento, ultima_barra + 1);
        strcat(nombre_elemento, "/"); // Le devolvemos la barra de formato
    }
    else
    {
        strcpy(nombre_elemento, ultima_barra + 1);
    }

    // =========================================================================
    // CONSTRUIR RUTA COMPLETA EN DESTINO Y COMPROBAR QUE NO EXISTA YA
    // =========================================================================

    char camino_destino_completo[512];
    sprintf(camino_destino_completo, "%s%s", camino_destino, nombre_elemento);

    unsigned int p_inodo_dir_tmp = 0;
    unsigned int p_inodo_tmp = 0;
    unsigned int p_entrada_tmp = 0;

    // Buscamos con reservar = 0. Si devuelve >= 0, es que ya existe un duplicado en destino
    if (buscar_entrada(camino_destino_completo,
                       &p_inodo_dir_tmp,
                       &p_inodo_tmp,
                       &p_entrada_tmp,
                       0,
                       0) >= 0)
    {
        fprintf(stderr, "Error: El elemento '%s' ya existe en el directorio destino.\n", nombre_elemento);
        return FALLO;
    }

    // =========================================================================
    //  CREA ENTRADA EN DESTINO Y ELIMINAR EN ORIGEN
    // =========================================================================

    mi_waitSem();

    // 1. Crear la nueva entrada en el directorio de destino (reservar = 1)
    unsigned int p_inodo_dir_dest = 0;
    unsigned int p_inodo_dest_nuevo = 0;
    unsigned int p_entrada_dest_nueva = 0;

    error = buscar_entrada(camino_destino_completo,
                           &p_inodo_dir_dest,
                           &p_inodo_dest_nuevo,
                           &p_entrada_dest_nueva,
                           1, // Reservamos la entrada
                           inodo_orig.permisos);

    if (error < 0)
    {
        mi_signalSem();
        return error;
    }

    // 2. Leer la entrada recién creada en el destino
    struct entrada entrada_dest;
    if (mi_read_f(p_inodo_dir_dest,
                  &entrada_dest,
                  p_entrada_dest_nueva * sizeof(struct entrada),
                  sizeof(struct entrada)) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    // 3. Asociar la entrada del destino al inodo original (como en mi_link)
    entrada_dest.ninodo = p_inodo_orig;
    if (mi_write_f(p_inodo_dir_dest,
                   &entrada_dest,
                   p_entrada_dest_nueva * sizeof(struct entrada),
                   sizeof(struct entrada)) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    // 4. Liberar el inodo ficticio que creó buscar_entrada automáticamente
    if (liberar_inodo(p_inodo_dest_nuevo) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    // 5. Eliminar la entrada del directorio de origen
    // Para ello, leemos la última entrada del directorio origen para sustituirla por la que borramos
    struct inodo inodo_dir_orig;
    if (leer_inodo(p_inodo_dir_orig, &inodo_dir_orig) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    int num_entradas_orig = inodo_dir_orig.tamEnBytesLog / sizeof(struct entrada);

    // Si la entrada a borrar no es la última, movemos la última a su posición
    if (p_entrada_orig != num_entradas_orig - 1)
    {
        struct entrada ultima_entrada;
        // Leer la última entrada
        if (mi_read_f(p_inodo_dir_orig,
                      &ultima_entrada,
                      (num_entradas_orig - 1) * sizeof(struct entrada),
                      sizeof(struct entrada)) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
        // Escribirla en la posición de la entrada que estamos moviendo/borrando
        if (mi_write_f(p_inodo_dir_orig,
                       &ultima_entrada,
                       p_entrada_orig * sizeof(struct entrada),
                       sizeof(struct entrada)) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
    }

    // 6. Truncar el directorio origen para eliminar el registro sobrante del final
    if (mi_truncar_f(p_inodo_dir_orig, (num_entradas_orig - 1) * sizeof(struct entrada)) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();

    return EXITO;
}