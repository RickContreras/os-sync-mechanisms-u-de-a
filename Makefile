# Makefile para el Laboratorio de Sincronización en SO
# Autor: Rick Contreras
# Descripción: Sistema de construcción para todas las tareas del laboratorio

# Configuración del compilador
CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -pthread -g
LDFLAGS = -pthread
VALGRIND_FLAGS = --tool=helgrind --read-var-info=yes

# Directorios
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
OUTPUT_DIR = output

# Targets
TARGETS = queue_test pc_test philosophers_test

# Available targets (only build what exists)
AVAILABLE_TARGETS = queue_test

# Check if other source files exist and add to targets
ifneq ($(wildcard $(SRC_DIR)/task2_producer_consumer/producer_consumer.c),)
    AVAILABLE_TARGETS += pc_test
endif

ifneq ($(wildcard $(SRC_DIR)/task3_dining_philosophers/dining_philosophers.c),)
    AVAILABLE_TARGETS += philosophers_test
endif

# Objetivo principal
.PHONY: all clean test valgrind help debug release queue_test pc_test philosophers_test

all: $(AVAILABLE_TARGETS)

# Crear directorio build si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Task 1: Thread-Safe Queue
queue_test: $(BUILD_DIR) $(SRC_DIR)/task1_queue/thread_safe_queue.c $(SRC_DIR)/task1_queue/queue_test.c
	$(CC) $(CFLAGS) $(SRC_DIR)/task1_queue/thread_safe_queue.c $(SRC_DIR)/task1_queue/queue_test.c -o $(BUILD_DIR)/queue_test $(LDFLAGS)
	@echo "✅ queue_test compilado exitosamente"

# Task 2: Producer-Consumer
pc_test: $(BUILD_DIR) $(SRC_DIR)/task2_producer_consumer/producer_consumer.c $(SRC_DIR)/task2_producer_consumer/pc_test.c
	$(CC) $(CFLAGS) $(SRC_DIR)/task2_producer_consumer/producer_consumer.c $(SRC_DIR)/task2_producer_consumer/pc_test.c -o $(BUILD_DIR)/pc_test $(LDFLAGS)
	@echo "✅ pc_test compilado exitosamente"

# Task 3: Dining Philosophers (cuando esté implementado)
philosophers_test: $(BUILD_DIR) $(SRC_DIR)/task3_dining_philosophers/dining_philosophers.c $(SRC_DIR)/task3_dining_philosophers/philosophers_test.c
	$(CC) $(CFLAGS) $(SRC_DIR)/task3_dining_philosophers/dining_philosophers.c $(SRC_DIR)/task3_dining_philosophers/philosophers_test.c -o $(BUILD_DIR)/philosophers_test $(LDFLAGS)
	@echo "✅ philosophers_test compilado exitosamente"

# Compilación con flags de debug
debug: CFLAGS += -DDEBUG -O0
debug: $(AVAILABLE_TARGETS)

# Compilación optimizada para producción
release: CFLAGS += -DNDEBUG -O2
release: $(AVAILABLE_TARGETS)

# Ejecutar todos los tests disponibles
test: $(AVAILABLE_TARGETS) $(OUTPUT_DIR)
	@echo "🧪 Ejecutando todos los tests disponibles..."
	@if [ -f "$(BUILD_DIR)/queue_test" ]; then \
        echo "Ejecutando queue_test..."; \
        ./$(BUILD_DIR)/queue_test | tee $(OUTPUT_DIR)/queue_output.txt; \
	fi
	@if [ -f "$(BUILD_DIR)/pc_test" ]; then \
        echo "Ejecutando pc_test..."; \
        ./$(BUILD_DIR)/pc_test | tee $(OUTPUT_DIR)/pc_output.txt; \
	fi
	@if [ -f "$(BUILD_DIR)/philosophers_test" ]; then \
        echo "Ejecutando philosophers_test..."; \
        ./$(BUILD_DIR)/philosophers_test | tee $(OUTPUT_DIR)/philosophers_output.txt; \
	fi

# Análisis con Valgrind para todos los tests disponibles
valgrind: $(AVAILABLE_TARGETS) $(OUTPUT_DIR)
	@echo "🔍 Ejecutando análisis Valgrind..."
	@if [ -f "$(BUILD_DIR)/queue_test" ]; then \
        echo "Analizando queue_test con Helgrind..."; \
        valgrind $(VALGRIND_FLAGS) ./$(BUILD_DIR)/queue_test 2>&1 | tee $(OUTPUT_DIR)/valgrind_queue.txt; \
	fi
	@if [ -f "$(BUILD_DIR)/pc_test" ]; then \
        echo "Analizando pc_test con Helgrind..."; \
        valgrind $(VALGRIND_FLAGS) ./$(BUILD_DIR)/pc_test 2>&1 | tee $(OUTPUT_DIR)/valgrind_pc.txt; \
	fi
	@if [ -f "$(BUILD_DIR)/philosophers_test" ]; then \
        echo "Analizando philosophers_test con Helgrind..."; \
        valgrind $(VALGRIND_FLAGS) ./$(BUILD_DIR)/philosophers_test 2>&1 | tee $(OUTPUT_DIR)/valgrind_philosophers.txt; \
	fi

# Limpiar archivos compilados
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(OUTPUT_DIR)
	@echo "🧹 Archivos de compilación eliminados"

# Mostrar ayuda
help:
	@echo "Laboratorio de Sincronización en SO - Sistema de Construcción"
	@echo "============================================================="
	@echo ""
	@echo "Targets disponibles:"
	@echo "  all                 - Compilar todos los proyectos disponibles"
	@echo "  queue_test          - Compilar test de cola thread-safe (Task 1)"
	@echo "  pc_test             - Compilar test producer-consumer (Task 2)"
	@echo "  philosophers_test   - Compilar test filósofos cenando (Task 3)"
	@echo "  test               - Ejecutar todos los tests disponibles"
	@echo "  valgrind           - Análisis de race conditions con Valgrind"
	@echo "  debug              - Compilar con flags de debugging"
	@echo "  release            - Compilar optimizado para producción"
	@echo "  clean              - Limpiar archivos compilados"
	@echo "  help               - Mostrar esta ayuda"
	@echo ""
	@echo "Targets actualmente disponibles: $(AVAILABLE_TARGETS)"
	@echo ""
	@echo "Ejemplos de uso:"
	@echo "  make all           # Compilar todo"
	@echo "  make pc_test       # Solo compilar producer-consumer"
	@echo "  make test          # Ejecutar todos los tests"
	@echo "  make valgrind      # Análisis completo con Valgrind"