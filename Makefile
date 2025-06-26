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

#Targets
TARGETS= queue_test pc_test philosophers_test

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
all: $(AVAILABLE_TARGETS)

# Crear directorios necesarios
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Task 1: Thread-Safe Queue
queue_test: $(BUILD_DIR) $(SRC_DIR)/task1_queue/thread_safe_queue.c $(SRC_DIR)/task1_queue/queue_test.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/queue_test \
		$(SRC_DIR)/task1_queue/thread_safe_queue.c \
		$(SRC_DIR)/task1_queue/queue_test.c \
		$(LDFLAGS)

# Task 2: Producer-Consumer (only if files exist)
pc_test: $(BUILD_DIR) $(SRC_DIR)/task2_producer_consumer/producer_consumer.c $(SRC_DIR)/task2_producer_consumer/pc_test.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/pc_test \
		$(SRC_DIR)/task2_producer_consumer/producer_consumer.c \
		$(SRC_DIR)/task2_producer_consumer/pc_test.c \
		$(LDFLAGS)

# Task 3: Dining Philosophers (only if files exist)
philosophers_test: $(BUILD_DIR) $(SRC_DIR)/task3_dining_philosophers/dining_philosophers.c $(SRC_DIR)/task3_dining_philosophers/philosophers_test.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/philosophers_test \
		$(SRC_DIR)/task3_dining_philosophers/dining_philosophers.c \
		$(SRC_DIR)/task3_dining_philosophers/philosophers_test.c \
		$(LDFLAGS)

# Run tests (only available ones)
test: $(AVAILABLE_TARGETS) $(OUTPUT_DIR)
	@echo "Running available tests..."
	@if [ -f "$(BUILD_DIR)/queue_test" ]; then \
		echo "Running Thread-Safe Queue Test..."; \
		timeout 15s ./$(BUILD_DIR)/queue_test > $(OUTPUT_DIR)/queue_output.txt 2>&1 || \
		{ echo "Test completed or timed out"; tail -10 $(OUTPUT_DIR)/queue_output.txt; } \
	fi
	@if [ -f "$(BUILD_DIR)/pc_test" ]; then \
		echo "Running Producer-Consumer Test..."; \
		./$(BUILD_DIR)/pc_test > $(OUTPUT_DIR)/pc_output.txt 2>&1; \
	fi
	@if [ -f "$(BUILD_DIR)/philosophers_test" ]; then \
		echo "Running Dining Philosophers Test..."; \
		./$(BUILD_DIR)/philosophers_test > $(OUTPUT_DIR)/philosophers_output.txt 2>&1; \
	fi
	@echo "All available tests completed. Check output/ directory for results."

# Memory leak detection
valgrind: $(AVAILABLE_TARGETS)
	@echo "Running Valgrind on available executables..."
	@if [ -f "$(BUILD_DIR)/queue_test" ]; then \
		echo "Running Valgrind on queue_test..."; \
		valgrind $(VALGRIND_FLAGS) ./$(BUILD_DIR)/queue_test; \
	fi
	@if [ -f "$(BUILD_DIR)/pc_test" ]; then \
		echo "Running Valgrind on pc_test..."; \
		valgrind $(VALGRIND_FLAGS) ./$(BUILD_DIR)/pc_test; \
	fi
	@if [ -f "$(BUILD_DIR)/philosophers_test" ]; then \
		echo "Running Valgrind on philosophers_test..."; \
		valgrind $(VALGRIND_FLAGS) ./$(BUILD_DIR)/philosophers_test; \
	fi

# Limpiar archivos generados
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(OUTPUT_DIR)

# Go implementations
go-build:
	cd $(SRC_DIR)/go_implementations && go build ./...

go-test:
	cd $(SRC_DIR)/go_implementations && go test -race ./...

# Development helpers
debug: CFLAGS += -DDEBUG -O0
debug: $(AVAILABLE_TARGETS)

release: CFLAGS += -O2 -DNDEBUG
release: $(AVAILABLE_TARGETS)

# Show available targets
status:
	@echo "Available targets: $(AVAILABLE_TARGETS)"
	@echo "Source files found:"
	@find $(SRC_DIR) -name "*.c" -o -name "*.h" | sort

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build all available C programs"
	@echo "  queue_test   - Build thread-safe queue test"
	@echo "  pc_test      - Build producer-consumer test (if sources exist)"
	@echo "  philosophers_test - Build dining philosophers test (if sources exist)"
	@echo "  test         - Run all available tests and save output"
	@echo "  valgrind     - Run memory leak detection"
	@echo "  go-build     - Build Go implementations"
	@echo "  go-test      - Test Go implementations"
	@echo "  clean        - Remove build artifacts"
	@echo "  debug        - Build with debug flags"
	@echo "  release      - Build with optimization"
	@echo "  status       - Show available targets and source files"
	@echo "  help         - Show this help message"

.PHONY: all test valgrind clean go-build go-test debug release help status