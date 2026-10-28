# paleta: los 27 colores del CPC a la vez

Ejecutar: `./cpc.sh run paleta`. Teclas **1**, **2** y **3** para elegir pantalla; **Espacio** pasa a la siguiente.

| Pantalla | Modo | Colores "normales" | Lo que se ve |
|---|---|---|---|
| 1 | 1 | 4 | Los 27 colores en cuadrícula, con número y nombre, y el título en degradado |
| 2 | 2 | 2 | Lo mismo en el modo de 2 colores, con el código hardware de cada color |
| 3 | 1 | 4 | Los 27 colores en movimiento: 600 cambios de color por imagen |

Continuación de `rasters/` y `rasters2/`.

## Los 27 colores
Cada color mezcla rojo, verde y azul, y cada componente vale 0, 1 o 2: 3 × 3 × 3 = 27. El número de BASIC es

```
INK n:   n = 9 × VERDE + 3 × ROJO + AZUL
```

Por eso caben en 9 filas × 3 columnas: en cada fila solo cambia el azul.

```
         azul 0       azul 1        azul 2
 G0 R0    0 Negro      1 Azul        2 Azul brillante
 G0 R1    3 Rojo       4 Magenta     5 Malva
 G0 R2    6 Rojo br.   7 Purpura     8 Magenta br.
 G1 R0    9 Verde     10 Cian       11 Azul cielo
 ...
 G2 R2   24 Amarillo br. 25 Amarillo pastel 26 Blanco brillante
```

El Gate Array no usa ese número, sino su propio código hardware (el `&hh` de la pantalla 2). La tabla `hw[]`
de `src/tablas.c` traduce uno a otro.

## La idea: cada línea tiene su propia paleta
Una tabla de 200 entradas (`tabla` en `src/tablas.c`), una por línea de pantalla, guarda 3 colores. Una rutina
en ensamblador la recorre entera en cada imagen. **El título, las etiquetas y la cuadrícula son solo datos de
la tabla**:

```
 línea   pantalla 1 (modo 1)             entrada de la tabla
 -----   ------------------------------  ------------------------------
  8-15   PALETA COMPLETA DEL CPC         degradado (se anima)
 40-47    0 Negro    1 Azul    2 Azul br. blanco, blanco, blanco
 48-55   [pluma 1] [pluma 2] [pluma 3]   color 0, color 1, color 2
 56      (separación)                    blanco
 57-64    3 Rojo     4 Magenta  5 Malva   blanco, blanco, blanco
 65-72   [pluma 1] [pluma 2] [pluma 3]   color 3, color 4, color 5
 ...
```

- **Modo 1** (`rasterTresPlumas`): en cada línea cambian las plumas 1, 2 y 3, en la parte invisible de la línea
  (borde y retrazado).
- **Modo 2** (`rasterTresColumnas`): solo hay una pluma de color, así que cambia **3 veces dentro de la línea**,
  en los bytes 0, 26 y 52 (split raster). Las celdas dejan 2 bytes de hueco para que el corte no se note.

La misma tabla sirve para los dos modos.

## Lo que aprendimos depurando
1. **El borde como cronómetro.** Poner el borde en rojo al empezar un trozo de código y en negro al acabar
   enseña en pantalla cuántas líneas tarda. Así descubrimos que `animaTitulo` tardaba más de 2 ms.
2. **No perder imágenes.** Si el trabajo de C no acaba antes de la sincronización, esa imagen se queda sin raster
   y la pantalla sale en blanco. Al principio usábamos `cpct_waitVSYNC()` y solo quedaban ~3 ms. Ahora un
   manejador cuenta las interrupciones y esperamos a la de la línea ~294: el margen sube a ~6 ms.
3. **Calibrar.** `ESPERA_*` (líneas) y `AJUSTE_*` (µs) se ajustan mirando dónde cae el corte. Una línea
   blanca encima de cada celda significa que el raster llega una línea tarde. Un color que empieza a mitad de
   una celda significa que el corte cae tarde en horizontal (en modo 2, 1 µs son 2 caracteres).

## Para experimentar
- Cambia la cuadrícula en `tablaPaleta`: por ejemplo, filas con `9g + 3r + b` ordenadas de otra forma.
- En la pantalla 3, prueba `n = y % 27` en vez de `(y >> 1) % 27`: un color nuevo en cada línea.
- Quita la separación de 2 bytes entre celdas del modo 2 y verás exactamente dónde cambia el color.
