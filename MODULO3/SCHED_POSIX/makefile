# Variables de compilación
CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lpthread

# Nombre del ejecutable y archivos fuente
TARGET = scheduler_rt_2026
SRCS = scheduler_rt_2026.c

# Regla por defecto: compilar el ejecutable
all: $(TARGET)

# Enlace del binario final
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

# Regla conveniente para ejecutar con los privilegios RT necesarios (sudo)
run: $(TARGET)
	sudo ./$(TARGET)

# Limpieza de archivos generados
clean:
	rm -f $(TARGET)

.PHONY: all run clean
