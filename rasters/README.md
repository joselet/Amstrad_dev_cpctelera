# rasters: más de 4 colores en modo 1

Ejecutar: `./cpc.sh run rasters`

## La idea en una frase
En modo 1 la memoria de vídeo guarda **plumas** (0-3), no colores. Si cambiamos el **color** de una pluma
mientras el haz del monitor va bajando, cada línea puede salir con un color distinto.

```
 línea   pantalla                          color de la pluma 1
 -----   -------------------------------   -------------------
   0     Arriba: paleta normal...          blanco
  ...
  34     <- llega la 2ª interrupción tras el VSYNC
  40     MODO 1 CON MUCHOS MAS...  (pluma 1)  blanco  <- empieza el bucle de 64 µs
  41     MODO 1 CON MUCHOS MAS...  (pluma 1)  amarillo pálido
  42     MODO 1 CON MUCHOS MAS...  (pluma 1)  amarillo
  ...    (un color nuevo en cada línea)
  79     ##########################  (barra)  rosa
  80     Abajo: paleta normal...           blanco  <- se restaura el color normal
```

Cada línea sigue teniendo solo 4 colores; es **la pantalla entera** la que tiene muchos más.

## Las piezas (todas en `src/main.c`)
| Pieza | Qué hace |
|---|---|
| `interrupcion()` | Se ejecuta 300 veces por segundo. Cuenta interrupciones desde el VSYNC y en la 2ª (línea ~34) llama al raster. |
| `rasterTitulo()` | En ensamblador: espera hasta la línea 40 y luego, en un bucle de **exactamente 64 µs** (lo que dura una línea), manda un color nuevo al Gate Array. |
| `degradado[]` | Un color por línea (40 líneas). |
| `while (1) halt` | Hace que la interrupción empiece siempre en el mismo instante, así el degradado no tiembla. |

## Para experimentar
- Cambia colores en `degradado[]`: por ejemplo, todas las filas con el mismo degradado.
- `ESPERA_NOPS`: pon `0` y verás en la barra un **escalón**. Es el punto exacto de la línea en el que cambia el
  color. Con `40` el cambio cae fuera de la pantalla visible (en el retrazado horizontal) y no se nota.
- `ESPERA_LINEAS`: sube o baja el degradado entero (si lo mueves, mueve también el título).
- Cambia también el borde: selecciona la pluma 16 (`ld bc,#0x7F10`) en vez de la 1 y tendrás barras de colores
  en el borde, como en las demos.

## Límites
- El cambio afecta a **toda la línea**: no se pueden poner dos colores distintos de la pluma 1 en la misma línea
  (bueno, se puede, cambiando a mitad, pero el tiempo es aún más crítico).
- Mientras dura el bucle (40 líneas ≈ 2,5 ms por imagen), la CPU no puede hacer otra cosa. En un juego hay que
  tener en cuenta ese tiempo.
- Debe acabar antes de la siguiente interrupción (línea ~86).
