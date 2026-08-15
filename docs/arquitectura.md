# Arquitectura de GraniteDTL

## Objetivo

GraniteDTL separa el procesamiento de comandos, el estado contable y las vistas
analíticas. El ejecutable es una unidad de despliegue, pero sus componentes
mantienen contratos explícitos para facilitar revisión, pruebas y evolución.

```mermaid
flowchart TB
    subgraph Entrada
        CLI["main.cpp"]
        SCRIPT["ScriptRunner"]
        SCENARIO["Scenario registry"]
    end
    subgraph Dominio
        ENGINE["Engine"]
        LOCKS["LockBook"]
        RISK["RiskBook"]
        AMOUNT["AmountMath"]
    end
    subgraph Lectura
        AUDIT["Audit"]
        PORT["Portfolio"]
        PLAN["Planner"]
        TREASURY["TreasuryStressModel"]
    end
    CLI --> SCRIPT
    CLI --> SCENARIO
    SCRIPT --> ENGINE
    SCENARIO --> ENGINE
    ENGINE --> LOCKS
    ENGINE --> RISK
    ENGINE --> AMOUNT
    ENGINE --> AUDIT
    ENGINE --> PORT
    ENGINE --> PLAN
    ENGINE --> TREASURY
```

## Estado autoritativo

El `Engine` posee cuentas, posiciones, vault, reloj, eventos y flujos externos.
`RiskBook` conserva vistas derivadas; una vista no es una fuente de fondos ni
una autorización de movimiento. `TreasuryStressModel` recibe un agregado y
produce proyecciones puras.

| Estado              | Propietario           | Mutabilidad    | Persistencia            |
| ------------------- | --------------------- | -------------- | ----------------------- |
| cuentas             | `Engine`              | por comando    | informe final           |
| posiciones          | `Engine`              | por transición | informe final           |
| vault               | `Engine`              | por transición | informe final           |
| vistas de cobertura | `RiskBook`            | refrescables   | informe final           |
| matriz de estrés    | `TreasuryStressModel` | ninguna        | calculada al serializar |
| journal             | `Engine`              | append-only    | informe final           |

```mermaid
classDiagram
    class Engine {
        accounts
        positions
        vault
        events
        settleExpired()
        liquidatePosition()
    }
    class LockBook {
        quote()
        quotePenalty()
    }
    class RiskBook {
        observe()
        refresh()
        invalidate()
    }
    class TreasuryStressModel {
        project()
        matrix()
    }
    Engine *-- LockBook
    Engine *-- RiskBook
    Engine ..> TreasuryStressModel: snapshot
```

## Flujo de lectura

La serialización construye primero resúmenes consistentes desde una referencia
constante al engine. Ningún escritor JSON puede mutar el estado. El SDK valida
la forma del objeto y lo congela recursivamente.

```mermaid
sequenceDiagram
    participant E as Engine
    participant A as Analizadores
    participant J as JsonWriter
    participant S as SDK
    A->>E: lectura const de estado y eventos
    E-->>A: snapshot lógico
    A->>J: audit + invariants + portfolio + stress
    J-->>S: JSON determinista
    S->>S: validar secciones
    S->>S: deepFreeze
```

## Decisiones de diseño

- Los importes se representan con `std::int64_t` y utilidades comprobadas.
- Las políticas usan basis points; `10.000` representa el cien por cien.
- Los mapas ordenados estabilizan la serialización de cuentas y posiciones.
- Los eventos reciben una secuencia monotónica dentro de una ejecución.
- Los scripts no contienen lógica económica: traducen comandos al engine.
- El SDK no usa shell y limita tiempo y volumen de salida.

## Extensión

Una nueva transición debe definir validaciones, mutación, evento, actualización
de versión del vault e invariantes. Una nueva vista debe ser pura y añadirse al
informe sin otorgar capacidad de escritura.
