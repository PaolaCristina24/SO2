CC=gcc
CFLAGS=-c -g -Wall -std=gnu17
LDFLAGS=-pthread


<<<<<<< HEAD
SOURCES=bloques.c mi_mkfs.c ficheros_basico.c ficheros.c leer_sf.c escribir.c leer.c permitir.c truncar.c directorios.c mi_mkdir.c mi_chmod.c mi_stat.c mi_ls.c mi_touch.c mi_escribir.c mi_cat.c mi_rm.c mi_link.c mi_rn.c mi_mv.c simulacion.c #verificacion.c
LIBRARIES=bloques.o semaforo_mutex_posix.o ficheros_basico.o ficheros.o directorios.o 
INCLUDES=bloques.h semaforo_mutex_posix.h ficheros_basico.h ficheros.h directorios.h simulacion.h #verificacion.h
PROGRAMS=mi_mkfs leer_sf escribir leer permitir truncar mi_mkdir mi_chmod mi_stat mi_ls mi_touch mi_escribir mi_cat mi_rm mi_link simulacion mi_rn mi_mv #verificacion
=======
SOURCES=bloques.c mi_mkfs.c ficheros_basico.c ficheros.c leer_sf.c escribir.c leer.c permitir.c truncar.c directorios.c mi_mkdir.c mi_chmod.c mi_stat.c mi_ls.c mi_touch.c mi_escribir.c mi_cat.c mi_rm.c mi_link.c simulacion.c #verificacion.c
LIBRARIES=bloques.o semaforo_mutex_posix.o ficheros_basico.o ficheros.o directorios.o 
INCLUDES=bloques.h semaforo_mutex_posix.h ficheros_basico.h ficheros.h directorios.h simulacion.h #verificacion.h
PROGRAMS=mi_mkfs leer_sf escribir leer permitir truncar mi_mkdir mi_chmod mi_stat mi_ls mi_touch mi_escribir mi_cat mi_rm mi_link simulacion #verificacion
>>>>>>> e0b6e2efe9cd3724ad05310f5b6e8546095aa133
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS) disco* ext*