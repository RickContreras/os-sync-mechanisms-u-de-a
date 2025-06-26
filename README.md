# Laboratorio de Sincronización en SO

Este es el MVP (Producto Mínimo Viable) para el laboratorio de mecanismos de sincronización en sistemas operativos.

## 🚀 Inicio Rápido

### Prerrequisitos
- Docker y VS Code con la extensión DevContainers
- O un sistema Linux con gcc, make, pthread, y valgrind

### Configuración con DevContainer

1. Clona este repositorio
2. Abre VS Code y usa "Reopen in Container"
3. El DevContainer se configurará automáticamente con todas las herramientas necesarias

### Compilación y Ejecución

```bash
# Compilar todos los proyectos
make all

# Ejecutar el test de la cola thread-safe
make queue_test
./build/queue_test

# Ejecutar todos los tests y guardar resultados
make test

# Ejecutar análisis de memoria con Valgrind
make valgrind

# Limpiar archivos de compilación
make clean
```

## 📁 Estructura Actual del MVP

```
os-synchronization-lab/
├── .devcontainer/
│   └── devcontainer.json          # Configuración DevContainer
├── src/
│   └── task1_queue/
│       ├── thread_safe_queue.h    # Header de la cola thread-safe
│       ├── thread_safe_queue.c    # Implementación de la cola
│       └── queue_test.c           # Tests de la cola
├── build/                         # Archivos compilados (generado)
├── output/                        # Resultados de tests (generado)
├── Makefile                       # Sistema de construcción
└── README.md                      # Este archivo
```

## 🎯 Estado Actual del MVP

### ✅ Completado
- **Configuración DevContainer**: Entorno de desarrollo completamente configurado
- **Tarea 1 - Cola Thread-Safe**: Implementación completa con tests
  - Cola circular thread-safe
  - Uso de mutex y variables de condición
  - Operaciones bloqueantes y no-bloqueantes
  - Tests unitarios y multi-threading

### 🚧 Por Implementar
- **Tarea 2**: Productor-Consumidor con semáforos
- **Tarea 3**: Filósofos Cenando
- **Implementaciones en Go**

## 🧪 Testing

El MVP incluye tests completos para la cola thread-safe:

### Tests Incluidos
1. **Tests Básicos**: Operaciones fundamentales (enqueue/dequeue)
2. **Tests Multi-threading**: 3 productores y 2 consumidores simultáneos
3. **Tests de Sincronización**: Verificación de thread-safety
4. **Análisis de Memoria**: Detección automática de leaks y race conditions

### Ejecutar Tests
```bash
# Compilar y ejecutar tests
make test

# Ver resultados
cat output/queue_output.txt

# Análisis completo con Valgrind (recomendado)
./tools/valgrind_check.sh

# O análisis manual
make valgrind
```

## 🔧 Desarrollo

### Comandos Útiles
```bash
# Compilación con debug
make debug

# Compilación optimizada
make release

# Análisis completo de memoria y race conditions
./tools/valgrind_check.sh

# Ver todas las opciones
make help
```

### Herramientas Incluidas
- **GCC**: Compilador con flags de threading
- **Valgrind**: Detección automática de memory leaks y race conditions
- **GDB**: Debugging de programas multi-threaded
- **Script automatizado**: `tools/valgrind_check.sh` para análisis completo

## 📊 Ejemplo de Salida

```
Programa de Prueba de Cola Thread-Safe
=====================================

=== Probando Operaciones Básicas ===
Prueba de operaciones básicas: EXITOSA

=== Probando Operaciones Multi-threaded ===
Iniciando 3 productores y 2 consumidores
Cada productor producirá 10 elementos
Productor 0 iniciado
Productor 1 iniciado
Productor 2 iniciado
Consumidor 0 iniciado
Consumidor 1 iniciado
...
Estadísticas Finales:
Total producido: 30
Total consumido: 30
Esperado: 30
Prueba multi-threaded: EXITOSA

¡Todas las pruebas completadas!
```

## 🐛 Debugging

### Análisis Automático de Problemas
```bash
# Script completo para análisis de memoria y race conditions
./tools/valgrind_check.sh

# Salida esperada:
# ✅ Verificación de memoria: EXITOSA (No se detectaron leaks)
# ✅ Verificación de race conditions: EXITOSA (No se detectaron race conditions)
```

### Análisis Manual de Race Conditions
```bash
# Helgrind para detectar race conditions
valgrind --tool=helgrind ./build/queue_test

# ThreadSanitizer (si está disponible)
gcc -fsanitize=thread -g -pthread -o test_tsan queue_test.c
./test_tsan
```

### Debugging con GDB
```bash
# Compilar con debug symbols
make debug

# Iniciar GDB
gdb ./build/queue_test

# Comandos útiles en GDB
(gdb) set scheduler-locking off
(gdb) thread apply all bt
(gdb) info threads
```

## 🔄 Próximos Pasos

### Implementar Task 2 - Producer-Consumer
1. Crear `src/task2_producer_consumer/`
2. Implementar con semáforos (`sem_t`)
3. Manejar buffer de tamaño fijo
4. Tests con múltiples productores/consumidores

### Implementar Task 3 - Dining Philosophers
1. Crear `src/task3_dining_philosophers/`
2. Prevenir deadlocks y starvation
3. Usar diferentes estrategias de solución
4. Tests con 5 filósofos

### Mejoras Adicionales
- Implementaciones en Go
- Benchmarks de rendimiento
- Documentación detallada
- CI/CD con GitHub Actions

## 🎓 Conceptos Demostrados en el MVP

### Sincronización
- **Mutex**: `pthread_mutex_t` para exclusión mutua
- **Variables de Condición**: `pthread_cond_t` para señalización
- **Patrón Productor-Consumidor**: Con cola acotada

### Seguridad de Threads
- Secciones críticas protegidas
- Prevención de race conditions
- Manejo correcto de recursos compartidos

### Testing Multi-threading
- Múltiples threads concurrentes
- Verificación de correctitud
- Estadísticas de sincronización

## 📚 Referencias

- Programación POSIX Threads: https://computing.llnl.gov/tutorials/pthreads/
- Manual de Usuario Valgrind: https://valgrind.org/docs/manual/
- Manual GDB: https://sourceware.org/gdb/documentation/

## 🤝 Contribución

Para extender este MVP:
1. Seguir la estructura de directorios establecida
2. Mantener el estilo de documentación
3. Incluir tests para nuevas funcionalidades
4. Actualizar el Makefile según sea necesario