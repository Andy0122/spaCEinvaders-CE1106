# --- CONFIGURACIÓN DEL COMPILADOR ---
CC = gcc
# Banderas de compilación: -Wall muestra advertencias, -I indica dónde están los .h
CFLAGS = -Wall -I include
# Banderas de enlazado (LDFLAGS): IMPORTANTE PARA WINDOWS
# Si usas Windows, descomenta la siguiente línea para que los Sockets funcionen:
# LDFLAGS = -lws2_32

# --- CARPETAS ---
SRC_DIR = src
BUILD_DIR = build

# --- ARCHIVOS ---
# Busca todos los .c en src/ y genera los nombres de los .o
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Nombre del ejecutable final
TARGET = $(BUILD_DIR)/cliente_juego

# --- REGLAS PRINCIPALES ---
all: $(TARGET)

# Cómo crear el ejecutable final
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "¡Compilación exitosa! Ejecutable creado en $(TARGET)"

# Cómo compilar cada .c individualmente
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar los compilados (útil cuando algo se traba)
clean:
	rm -rf $(BUILD_DIR)/*
	@echo "Archivos compilados eliminados."