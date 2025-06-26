# Estrategia de Pruebas

## Tipos de Pruebas

### 1. Pruebas Unitarias
- Operaciones básicas de la cola
- Casos límite (cola llena/vacía)
- Manejo de errores

### 2. Pruebas de Concurrencia
- Múltiples productores y consumidores
- Verificación de orden FIFO
- Estadísticas de sincronización

### 3. Pruebas de Memoria
- Detección de memory leaks
- Verificación de inicialización
- Limpieza de recursos

### 4. Pruebas de Race Conditions
- Análisis con Helgrind
- Verificación de exclusión mutua
- Detección de deadlocks

## Ejecutar Pruebas

```bash
# Todas las pruebas
make test

# Prueba específica
./build/queue_test

# Análisis de memoria
make valgrind
```

## Herramientas de Análisis

### Valgrind - Análisis de Memoria
```bash
# Análisis completo de memoria
valgrind --leak-check=full ./build/queue_test

# Resultado esperado exitoso:
# HEAP SUMMARY:
#     in use at exit: 0 bytes in 0 blocks
#   total heap usage: 8 allocs, 8 frees, 2,416 bytes allocated
# All heap blocks were freed -- no leaks are possible
# ERROR SUMMARY: 0 errors from 0 contexts
```

### Helgrind - Análisis de Race Conditions
```bash
# Análisis de race conditions
valgrind --tool=helgrind ./build/queue_test

# Resultado esperado exitoso:
# ERROR SUMMARY: 0 errors from 0 contexts (suppressed: X from Y)
# (Los errores suprimidos son normales en bibliotecas del sistema)
```

### GDB - Depuración Multi-threaded
```bash
# Depuración interactiva
gdb ./build/queue_test

# Comandos útiles durante la ejecución:
(gdb) run                        # Ejecutar el programa
(gdb) break main                 # Punto de ruptura en main
(gdb) break thread_function      # Punto de ruptura en función de thread
(gdb) info threads              # Listar todos los threads
(gdb) thread 2                  # Cambiar al thread 2
(gdb) bt                        # Backtrace del thread actual
(gdb) thread apply all bt       # Backtrace de todos los threads
(gdb) continue                  # Continuar ejecución
```

## Comandos de Depuración Avanzada

### Para Deadlocks
```bash
# Si sospechas de deadlocks
gdb ./build/queue_test
(gdb) run
# Cuando se cuelgue, presiona Ctrl+C
(gdb) info threads
(gdb) thread apply all bt
```

### Para Análisis de Rendimiento
```bash
# Timing de ejecución
time ./build/queue_test

# Profiling con gprof (requiere compilación especial)
gcc -pg -pthread src/task1_queue/*.c -o build/queue_test_prof
./build/queue_test_prof
gprof build/queue_test_prof gmon.out > analisis_rendimiento.txt
```

### Para Verificación de Thread Safety
```bash
# Ejecutar múltiples veces para detectar race conditions intermitentes
for i in {1..10}; do
    echo "Ejecución $i:"
    ./build/queue_test
    echo "---"
done
```

## Notas de Desarrollo

### Flags de Compilación Recomendados
```bash
# Para desarrollo
gcc -Wall -Wextra -g -pthread -DDEBUG

# Para debugging de threads
gcc -Wall -Wextra -g -pthread -fsanitize=thread

# Para producción
gcc -Wall -Wextra -O2 -pthread -DNDEBUG
```