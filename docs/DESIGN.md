# Diseño del Laboratorio de Sincronización

## Visión General

Este laboratorio implementa tres problemas clásicos de sincronización en sistemas operativos:

1. **Cola Thread-Safe** - Estructura de datos sincronizada
2. **Productor-Consumidor** - Problema de sincronización con semáforos
3. **Filósofos Cenando** - Problema de asignación de recursos

## Arquitectura

### Tarea 1: Cola Thread-Safe

```
cola_thread_safe.h
├── Estructura ThreadSafeCola
├── Funciones de inicialización/destrucción
├── Operaciones enqueue/dequeue
└── Funciones de estado (llena/vacía)

cola_thread_safe.c
├── Implementación con mutex
├── Variables de condición para señalización
├── Buffer circular para eficiencia
└── Manejo de errores

test_cola.c
├── Tests unitarios básicos
├── Tests de concurrencia
├── Estadísticas de rendimiento
└── Validación de thread-safety
```

### Sincronización

- **Mutex**: Protege el acceso a la estructura de datos
- **Variables de Condición**: 
  - `cola_no_llena`: Señala cuando hay espacio
  - `cola_no_vacia`: Señala cuando hay datos
- **Operaciones Atómicas**: Todas las operaciones son thread-safe

## Principios de Diseño

1. **Simplicidad**: Interfaz clara y fácil de usar
2. **Robustez**: Manejo completo de errores
3. **Eficiencia**: Uso óptimo de primitivas de sincronización
4. **Testabilidad**: Tests exhaustivos incluidos

### Task 2: Producer-Consumer (TODO)

### Task 3: Dining Philosophers (TODO)

## Performance Considerations

- Memory efficiency with circular buffer
- Minimal locking overhead
- Scalable design for multiple threads

## Testing Strategy

- Unit tests for basic operations
- Multi-threaded stress tests
- Memory leak detection with Valgrind
- Race condition analysis
