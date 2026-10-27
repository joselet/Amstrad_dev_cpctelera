# READY, STEADY, GO! — un duelo del oeste

Juego muy sencillo para Amstrad CPC hecho en C con CPCtelera. Dos vaqueros frente a frente: tras unos tiempos aleatorios aparecen **READY**, **STEADY** y **GO!**.

- Si pulsas **ESPACIO** antes del GO, has disparado antes de tiempo y pierdes.
- Después del GO gana el que dispare primero, tú o el ordenador.
- Cada vez que ganas subes de nivel, y el ordenador dispara más rápido.
- Al morir se muestra "HAS LLEGADO AL NIVEL X".

```bash
./cpc.sh run claudegame      # desde la raíz del repositorio, en WSL
```

## Ficheros

| Fichero | Qué es |
|---|---|
| `src/main.c` | Todo el juego (unas 200 líneas con comentarios). |
| `img/sprites.py` | Script que dibuja los PNG de los vaqueros a partir de dibujos en texto. |
| `img/*.png` | Los 6 sprites: `jug` (jugador) y `ene` (enemigo), cada uno `q` (quieto), `d` (dispara) y `m` (muerto). |
| `cfg/image_conversion.mk` | Le dice a CPCtelera cómo convertir los PNG a C y qué paleta usar. |
| `src/jugq.c/.h`, `src/g_paleta.c/.h`... | **Generados** por CPCtelera al compilar. No se editan a mano. |

El resto (`Makefile`, otros ficheros de `cfg/`, `obj/`, `.dsk`...) no se versiona: `cpc.sh` lo regenera con `cpct_mkproject`.

## Cómo se ha hecho

### 1. Los gráficos: de texto a PNG y de PNG a C

El modo 1 del CPC tiene 320x200 píxeles y **4 colores**. Elegimos la paleta (números de color del firmware):

| Índice | Color | Firmware | Uso |
|---|---|---|---|
| 0 | negro | 0 | fondo |
| 1 | naranja | 15 | piel, suelo |
| 2 | rojo | 6 | camisa, fogonazo |
| 3 | blanco | 26 | sombrero, pantalón, pistola |

Los sprites se dibujan en `img/sprites.py` como texto, un carácter por píxel (`.` negro, `o` naranja, `r` rojo, `w` blanco):

```
.....wwwww......
.....wwwww......
...wwwwwwwww....
......ooo.......
```

Cada dibujo mide 16x20 y el script lo amplía al doble: 32x40 píxeles. Los del enemigo son **el espejo** de los del jugador con los colores rojo y blanco intercambiados, así que basta con dibujar 3 posturas para tener 6 sprites. El script escribe los PNG sin librerías externas (el formato PNG son unas cabeceras y los píxeles comprimidos con `zlib`).

Para cambiar un vaquero: edita el dibujo en `sprites.py`, ejecuta `python3 sprites.py` dentro de `img/` y recompila. También puedes retocar los PNG con cualquier editor de imágenes, siempre usando solo esos 4 colores exactos.

Al hacer `make`, CPCtelera lee `cfg/image_conversion.mk` y convierte cada PNG en un array de C con los bytes tal como los espera la memoria de vídeo:

```make
PALETA=0 15 6 26
$(eval $(call IMG2SP, SET_MODE        , 1                  ))
$(eval $(call IMG2SP, SET_FOLDER      , src/               ))
$(eval $(call IMG2SP, SET_PALETTE_FW  , $(PALETA)          ))
$(eval $(call IMG2SP, CONVERT_PALETTE , $(PALETA), g_paleta ))
$(eval $(call IMG2SP, CONVERT         , img/jugq.png , 32, 40, g_jugq ))
...
```

Resultado: `src/jugq.h` declara `g_jugq` (el sprite) y `G_JUGQ_W` / `G_JUGQ_H` (8 bytes de ancho, 40 líneas). En modo 1 **cada byte guarda 4 píxeles**, por eso 32 píxeles son 8 bytes. `g_paleta` contiene los 4 colores ya traducidos a los valores que entiende el hardware.

### 2. Arrancar: firmware, modo y paleta

```c
cpct_disableFirmware();          // si no, el firmware nos deshace modo y paleta
cpct_setVideoMode(1);
cpct_setPalette(g_paleta, 4);
```

### 3. El reloj del juego: los cuadros de pantalla

El CPC dibuja la pantalla **50 veces por segundo**. `cpct_waitVSYNCStart()` espera al inicio del siguiente cuadro, así que cada vuelta de bucle dura exactamente 1/50 s. Todos los tiempos del juego se cuentan en cuadros (50 = 1 segundo).

La función clave es `esperar()`: espera N cuadros mirando el teclado en cada uno, y avisa en cuanto se pulsa el espacio:

```c
u8 esperar(u8 frames) {
   while (frames--) {
      cpct_waitVSYNCStart();
      if (espacio())
         return 1;     // ¡ha disparado!
   }
   return 0;           // se acabó el tiempo sin disparar
}
```

Con ella se resuelve todo el duelo:

- Esperas antes de READY, STEADY y GO: si `esperar()` devuelve 1, el jugador ha disparado antes de tiempo.
- Después del GO, `esperar(reaccion)`: si devuelve 1 el jugador ha sido más rápido; si devuelve 0 el ordenador ha disparado primero.

### 4. Tiempos aleatorios y dificultad

`cpct_rand()` da números pseudoaleatorios. Para que no salga la misma secuencia en cada partida hace falta una **semilla** distinta: mientras está la pantalla de título se cuentan vueltas de bucle hasta que el jugador pulsa espacio. Ese número nunca es igual, y se pasa a `cpct_srand()`.

Esperas (en cuadros): antes de READY 50–125 (1–2,5 s), antes de STEADY 25–75, antes de GO 25–100.

Reacción del ordenador tras el GO:

```c
if (nivel < 9) reaccion = 45 - nivel * 4;   // nivel 1: 41 cuadros (0,8 s)
else           reaccion = 10;               // tope: 0,2 s, un reflejo humano muy bueno
reaccion += aleatorio(0, 8);                // un poco de azar
```

Para hacerlo más fácil o más difícil, cambia esos números.

### 5. Pintar en pantalla

- `cpct_getScreenPtr(CPCT_VMEM_START, x, y)` calcula la dirección de memoria de vídeo de una posición (x en bytes, 0–79; y en líneas, 0–199).
- `cpct_drawSprite(sprite, posicion, 8, 40)` copia el sprite. Como todos los sprites miden igual y son opacos, para "cambiar de postura" basta con pintar el nuevo encima.
- `cpct_drawStringM1(texto, posicion)` escribe con la fuente de la ROM; cada letra ocupa 2 bytes de ancho, por eso para centrar se usa `x = 40 - largo`.

## Lección aprendida: variables globales inicializadas

La primera versión tenía los textos con número así, como globales:

```c
char txt_nivel[] = "NIVEL 00";    // global: ¡MAL en CPCtelera!
```

El juego se volvía loco (textos rotos, pantalla corrupta). El motivo: en CPCtelera **las variables globales con valor inicial no se inicializan al arrancar** (el compilador SDCC deja esa copia a un código de arranque que CPCtelera no incluye). La cadena estaba vacía, y la función que escribe el número, al buscar el final de la cadena, escribía *fuera* del array y machacaba memoria.

Soluciones:
- Hacerla **local** dentro de la función (es lo que hace el juego): las locales se rellenan cada vez que se entra en la función.
- O, si es un dato que no cambia, declararla `const` (va a la memoria del programa y sí tiene su valor).

## Ideas para seguir

- Sonido de disparo (CPCtelera incluye el reproductor Arkos).
- Medir y mostrar el tiempo de reacción del jugador en centésimas.
- Letras grandes para READY / STEADY / GO dibujadas como sprites.
- Un fogonazo animado unos cuadros antes de la postura de "muerto".
