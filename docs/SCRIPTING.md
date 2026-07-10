# GraniteDTL Scripting

Los scripts `.gdtl` son lineales, deterministas y sin includes. Las lineas en
blanco y las que empiezan por `#` se ignoran.

## Ejemplo

```text
SCENARIO custom
EMPTY
ACCOUNT alice Alice 2200000
ACCOUNT merchant Merchant 50000
RESERVE 700000 seed
OPEN p1 alice merchant 1700000 1000000 5 500 lane-a
REFRESH p1
RELEASE p1
ADVANCE 2
COMPLETE p1
```

Ejecutar:

```bash
build/granitedtl script examples/healthy.gdtl
```

## Politicas

`POLICY <key> <value>` soporta:

- `min_coverage_bps`
- `target_coverage_bps`
- `default_penalty_bps`
- `daily_penalty_bps`
- `max_penalty_bps`
- `release_fee_bps`
- `grace_period`
- `reserve_floor`
- `max_single_lock`
- `allow_surplus_release`

## Comandos Tolerantes

Cualquier comando puede prefijarse con `TRY_` para registrar el rechazo y
continuar:

```text
TRY_COMPLETE missing-position
TRY_WITHDRAW alice 999999999
```

Esto permite probar controles negativos sin romper el reporte final.
