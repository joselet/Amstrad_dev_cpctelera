# rasters2: demo de rasters en modo 0, 1 y 2

Ejecutar: `./cpc.sh run rasters2`. Teclas **0**, **1** y **2** para elegir el modo; **Espacio** pasa al siguiente.

Continuación de `rasters/` (léelo primero).

| Modo | Normal | Con rasters |
|---|---|---|
| 0 | 16 colores | degradado de fondo y de letras + las 16 plumas normales |
| 1 | 4 colores | degradado vertical + *split raster*: letras contiguas de colores distintos |
| 2 | 2 colores | texto de 80 columnas con 2 colores distintos en cada línea + *split raster* |

## Una línea de pantalla, en el tiempo
```
 µs:  0                                      40          46            64
      |<----------- zona visible ------------>|<- borde ->|<- retrazado ->|
      |  modo 0: 160 px | modo 1: 320 | modo 2: 640       |               |
                                                    ^^^^^^^^^^^^^^^^
                     rasterVertical: los 4 OUT caen aquí, donde no se ven
      ^         ^         ^         ^
      rasterColumnas: un OUT cada 10 µs, EN la zona visible -> 4 columnas
```
- En 1 µs se pintan 2 bytes: 4 px en modo 0, 8 en modo 1 y 16 en modo 2.
- Un `OUT` tarda 4 µs. Por eso el cambio horizontal más fino posible es cada 4 µs: 2 caracteres en modo 0,
  4 en modo 1 u 8 en modo 2. Carácter a carácter no se puede.
- El **modo** no se puede cambiar a mitad de línea: el Gate Array lo aplica en la línea siguiente.
- El CPC tiene 27 colores: es el máximo en pantalla, en cualquier modo.
- Mientras dura un raster la CPU no hace nada más (en modo 2: 144 líneas ≈ 9 ms de los 20 ms de cada imagen).

## Las piezas (`src/main.c`)
| Pieza | Qué hace |
|---|---|
| Bucle de `main` | Se sincroniza con el VSYNC y con dos `HALT` (línea ~34), desactiva las interrupciones y lanza los efectos. Después lee el teclado y avanza la animación. |
| `rasterVertical` | Las plumas 0 y 1 cambian de color en cada línea. Lee la tabla con `pop` (el **truco de la pila**: 2 bytes en 3 µs). |
| `rasterColumnas` | Una pluma cambia 4 veces dentro de cada línea, una cada 10 µs. |
| `construyeTablas` | Precalcula los colores. Para animar, en cada imagen se empieza a leer una fila más abajo. |
| `src/pantallas.c` | Lo que se dibuja en cada modo (solo plumas). |

## Para experimentar
- `AJUSTE_V`: réstale 22 y verás escalones en los degradados. Es el punto exacto de la línea en el que cambia el
  color, que ahora cae en la zona visible.
- `AJUSTE_C`: mueve las columnas a izquierda o derecha (1 NOP = 1 carácter en modo 1).
- En `rasterColumnas`, pasa los `.rept 6` a `.rept 0`: los cambios irán cada 4 µs, el máximo posible. Tendrás
  4 columnas estrechas a la izquierda (añade NOPs al relleno final para que el bucle siga sumando 64).
- Cambia `brillantes[]` y `oscuros[]`, o la fórmula de `construyeTablas`.
