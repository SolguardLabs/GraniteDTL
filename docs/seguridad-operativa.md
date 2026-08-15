# Seguridad operativa

## Modelo de controles

Los controles se distribuyen entre prevención, detección y recuperación. Una
comprobación contable no sustituye a los límites de política, y una vista de
riesgo no autoriza movimientos por sí sola.

```mermaid
flowchart TB
    subgraph Prevención
        TYPES["Tipos e importes"]
        POLICY["Límites de política"]
        SDK["Proceso sin shell"]
    end
    subgraph Detección
        EVENTS["Journal monotónico"]
        INV["Invariantes"]
        STRESS["Escenarios de estrés"]
    end
    subgraph Recuperación
        INPUT["Entrada versionada"]
        REPLAY["Reejecución determinista"]
        RECON["Conciliación"]
    end
    Prevención --> Detección
    Detección --> Recuperación
```

## Separación de responsabilidades

| Rol            | Puede                         | No debe                             |
| -------------- | ----------------------------- | ----------------------------------- |
| operador       | ejecutar escenarios aprobados | cambiar políticas sin revisión      |
| revisor        | validar journal e invariantes | modificar el informe                |
| mantenedor     | aprobar cambios y versiones   | omitir CI multiplataforma           |
| consumidor SDK | aplicar decisiones propias    | asumir que una vista muta el ledger |

```mermaid
sequenceDiagram
    participant O as Operador
    participant E as Engine
    participant R as Revisor
    participant M as Mantenedor
    O->>E: ejecutar entrada aprobada
    E-->>R: informe + journal
    R->>R: validar conservación y límites
    R-->>M: evidencia y decisión
    M->>M: aprobar o rechazar ciclo
```

## Invariantes de control

- ningún saldo de cuenta o posición puede ser negativo;
- la suma interna debe coincidir con entradas menos retiradas;
- la secuencia de eventos debe ser estrictamente creciente;
- los identificadores de posición deben ser únicos;
- los estados terminales no deben conservar acciones pendientes;
- la deuda próxima nunca puede superar la deuda abierta en la matriz;
- la deuda de la mayor lane nunca puede superar el total abierto.

```mermaid
flowchart TD
    RUN["Ejecución"] --> A{"Aritmética válida"}
    A -- No --> REJECT["Rechazar"]
    A -- Sí --> S{"Estado coherente"}
    S -- No --> HOLD["Retener conciliación"]
    S -- Sí --> J{"Journal monotónico"}
    J -- No --> HOLD
    J -- Sí --> T{"Tesorería dentro de límites"}
    T -- No --> LIMIT["Aplicar límites operativos"]
    T -- Sí --> OK["Aceptar"]
```

## Gestión de entradas

Los archivos `.gdtl` deben almacenarse como texto UTF-8, revisarse por pares y
referenciar un objetivo concreto. Evita nombres ambiguos, cantidades sin unidad
y cambios de política mezclados con operaciones no relacionadas.

## Evidencia

Conserva:

- SHA del commit y versión del compilador;
- hash del archivo de entrada;
- stdout JSON original;
- resultado de CI y plataforma;
- decisión de aceptación o rechazo.

No almacenes credenciales en scripts, eventos o informes. Ante información
sensible, utiliza el canal privado descrito en `SECURITY.md`.
