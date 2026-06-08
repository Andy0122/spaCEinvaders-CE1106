# Variables de compilación
CC = gcc

# -I./include le dice a gcc que busque raylib.h ahí
CFLAGS = -Wall -I./include

# -L./lib le dice que busque libraylib.a en esa carpeta
# -lraylib enlaza la biblioteca
# -lgdi32 y -lwinmm son obligatorias en Windows para usar Raylib
# -lws2_32 es la de sockets que ya tenías
LDFLAGS = -L./lib -lraylib -lgdi32 -lwinmm -lws2_32

# Definición de carpetas
SRC_DIR = src
INC_DIR = include
BIN_DIR = .

# Archivos fuente y de objeto
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/red.c $(SRC_DIR)/gui.c $(SRC_DIR)/estructuras_datos.c
OBJS = $(SRCS:.c=.o)
EXEC = $(BIN_DIR)/cliente_spaceinvaders.exe

# Regla principal
all: $(EXEC)

# Enlace (Linking)
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)
	@echo "========================================="
	@echo "Compilacion exitosa: $(EXEC)"
	@echo "========================================="

# Compilación de cada archivo fuente
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar archivos generados
clean:
	del /Q src\*.o
	del /Q cliente_spaceinvaders.exe