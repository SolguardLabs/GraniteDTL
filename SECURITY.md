# Security Policy

GraniteDTL es un laboratorio de seguridad con un fallo intencional. La version
publica del reto debe incluir el codigo, tests y documentacion operativa, pero
no debe publicar `vulnerability.md` ni soluciones privadas.

## Alcance

El alcance del CTF incluye:

- logica de collateral y locks temporales;
- expiraciones y penalizaciones;
- ratio de cobertura y liberacion de surplus;
- liquidacion contra vault/reserva;
- parser de scripts `.gdtl`;
- reportes JSON y tests de integracion.

Queda fuera de alcance:

- seguridad de red;
- firmas reales;
- persistencia;
- concurrencia;
- integraciones externas;
- hardening productivo de secretos o credenciales.

## Reporte

Para un CTF privado, reporta:

1. secuencia minima de reproduccion;
2. impacto contable observado;
3. ubicacion del fallo en codigo;
4. razon por la que los tests publicos no lo cubren;
5. parche propuesto;
6. test privado que falle antes del parche y pase despues.

## Reglas

- No dependas de servicios externos.
- No modifiques tests publicos para ocultar el fallo.
- No publiques `vulnerability.md` en repositorios destinados a participantes.
- Mantén cualquier exploit como script reproducible y determinista.
