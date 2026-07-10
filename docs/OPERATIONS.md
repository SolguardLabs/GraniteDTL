# GraniteDTL Operations

El motor mantiene todo el estado en memoria y usa un reloj determinista. Cada
comando que mueve fondos emite un evento y el reporte final incluye auditoria,
journal, portfolio, plan e invariantes.

## Ciclo De Vida

1. `ACCOUNT` registra saldos externos iniciales.
2. `RESERVE` capitaliza el vault de garantias.
3. `OPEN` mueve collateral del owner a una posicion timelocked.
4. `ADVANCE` mueve el reloj y marca expiraciones vencidas.
5. `REFRESH` actualiza cobertura observada.
6. `PENALTY` descuenta incumplimiento del collateral.
7. `RELEASE` devuelve surplus si la cobertura lo permite.
8. `SETTLE` procesa una posicion expirada.
9. `LIQUIDATE` paga claim al counterparty y termina la posicion.
10. `WITHDRAW` saca cash del modelo local.

## Invariantes

- `accounting`: fondos internos deben igualar inflows menos withdrawals.
- `reserve_floor`: `vault.reserve_cash` debe quedar sobre el floor.
- `non_negative_balances`: cuentas y posiciones no pueden ser negativas.
- `open_coverage`: posiciones abiertas no deben cargar shortfall.
- `journal_release_sum`: eventos de release deben cuadrar con el journal.

## Campos De Riesgo

- `required_collateral`: `debt * min_coverage_bps / 10000`.
- `surplus`: collateral por encima del requerido.
- `shortfall`: deficit contra el requerido.
- `coverage_bps`: ratio entero `collateral / debt`.
- `penalty_accrued`: collateral transferido a reserva de penalizaciones.

## Recomendacion Para Auditores

Inspecciona secuencias donde `risk_refreshed`, `penalty_accrued` y
`surplus_released` aparecen juntos en una misma posicion. Esas transiciones son
las mas sensibles del modelo.
