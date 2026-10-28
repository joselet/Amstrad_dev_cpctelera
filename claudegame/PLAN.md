# PLAN — juego de vaqueros "Ready, Steady, Go!"

Registro de la sesión en la que se creó este juego con Claude: qué se pidió, qué se planificó y qué salió al final (con los cambios sobre el plan y por qué).

## 1. La petición

> He creado el proyecto de cpctelera claudegame. ¿Podríamos generar un juego facilito? Quiero un juego de vaqueros súper fácil. Ready, steady, go.
>
> Un contador decide cuándo empieza el duelo (cuándo hay que disparar). En un tiempo aleatorio aparecerá READY, otro tiempo aleatorio STEADY, y otro tiempo aleatorio GO. Que estos tiempos no duren demasiados segundos, que no se aburra el jugador.
>
> - Si el jugador aprieta el espacio antes de que haya aparecido GO, ha perdido.
> - Si aprieta el espacio una vez ha aparecido GO, consigue disparar a su adversario.
> - El adversario (el algoritmo del juego) disparará también, y será cada vez más preciso a medida que aumenten los niveles.
> - Si el ordenador dispara antes que el jugador después del GO, el jugador muere mostrando "has llegado al nivel X".
> - Si el jugador dispara antes que el ordenador, se pasa a otro nivel donde el ordenador es más preciso.
>
> Código lo más sencillo posible. Pocos sprites: uno para el jugador, otro para el enemigo, y los mismos representando al que dispara y al que muere.

Decisiones durante la sesión:

- **Sprites:** se eligió PNG + conversión automática de CPCtelera (frente a arrays de C a mano o rectángulos), porque es la forma habitual de trabajar con CPCtelera y los gráficos son fáciles de retocar.
- El usuario no puede dibujar los PNG, así que **Claude los genera**.
- Se pidió también un `README.md` que explique cómo se ha desarrollado el juego.

## 2. El plan

**Ficheros previstos**

- `src/main.c`: todo el juego en un solo fichero, comentado en español.
- `img/*.png`: 6 sprites de 16x32 px. Jugador y enemigo, cada uno en tres posturas: quieto, disparando y muerto. El enemigo es el espejo del jugador con otros colores.
- `cfg/image_conversion.mk`: modo 1, paleta de 4 colores y conversión de cada PNG a un array de C (salida en `src/sprites/`).
- `.gitignore`: admitir los PNG y `cfg/image_conversion.mk`, que la lista blanca no incluía.

**Paleta (modo 1, colores del firmware):** negro, naranja/amarillo, rojo y blanco.

**Estructura de `main.c`**

- Inicialización: apagar el firmware, modo 1, paleta y borde.
- `esperar(frames)`: el reloj del juego es la pantalla (50 cuadros por segundo con `cpct_waitVSYNC`). Espera N cuadros y devuelve 1 si se pulsa espacio. Con esto se detecta tanto el disparo antes de tiempo como quién dispara primero tras el GO.
- Números aleatorios con `cpct_rand()`. La semilla es lo que tarda el jugador en pulsar en la pantalla de título.
- Textos con `cpct_drawStringM1` y una función mínima para escribir el número de nivel (sin `sprintf`, que ocupa mucho).
- Bucle: título → duelo (esperas aleatorias de ≈1–2,5 s, 0,5–1,5 s y 0,5–2 s) → tras el GO el ordenador dispara a los `max(10, 45 − nivel·4) + azar(0..8)` cuadros → gana uno u otro.

**Verificación prevista:** compilar con `./cpc.sh build`, lanzar en Caprice32, capturar la pantalla, simular el espacio con `keybd_event` y comprobar los tres casos: disparo antes de tiempo, derrota y victoria.

## 3. El resultado

El juego funciona y se probaron en el emulador los cuatro casos: pantalla de título, disparo antes de tiempo, derrota cuando el ordenador dispara primero, y victoria con paso al nivel 2. Para probar la victoria, un script vigilaba la pantalla y pulsaba el espacio al aparecer el "GO!" rojo.

**Ficheros finales**

| Fichero | Contenido |
|---|---|
| `src/main.c` | El juego completo (~200 líneas). |
| `img/sprites.py` | Genera los 6 PNG a partir de dibujos en texto (un carácter = un píxel). |
| `img/*.png` | `jugq/jugd/jugm` (jugador) y `eneq/ened/enem` (enemigo): quieto, dispara, muerto. |
| `cfg/image_conversion.mk` | Paleta `0 15 6 26` (negro, naranja, rojo, blanco) y conversión de los PNG. |
| `src/*.c/.h` de los sprites y `g_paleta` | Generados por CPCtelera al compilar. |
| `README.md` | Explicación de cómo se ha hecho el juego. |

**Cambios respecto al plan, y por qué**

1. **Sprites de 32x40 en vez de 16x32.** Se dibujan a 16x20 y el script los amplía al doble. Con 16 px de ancho no cabía el brazo extendido con la pistola, ni el vaquero tumbado.
2. **Un script genera los PNG** (`img/sprites.py`, Python sin librerías externas). Así el usuario puede cambiar los dibujos editando texto, aunque no tenga editor de imágenes.
3. **Los sprites convertidos van a `src/` y no a `src/sprites/`.** CPCtelera exige que la carpeta de salida exista, y `src/sprites/` no se versionaba. Por eso, en un clon nuevo no existiría y la compilación fallaría. Efecto secundario: los `.c` generados también se guardan en git.
4. **`cpct_waitVSYNCStart()` en vez de `cpct_waitVSYNC()`.** La señal VSYNC dura varias líneas de pantalla, y llamar a `waitVSYNC` dos veces seguidas podría contar el mismo cuadro dos veces. `waitVSYNCStart` espera al *inicio* de cada cuadro.
5. **Fallo encontrado y corregido: las variables globales inicializadas.** La primera versión tenía `char txt_nivel[] = "NIVEL 00";` como global y el juego se corrompía. En CPCtelera las globales con valor inicial no se inicializan al arrancar: la cadena estaba vacía y la función del número escribía fuera del array. Se pasaron a variables locales (más detalles en `README.md`).
6. **`cpc.sh` modificado** (no estaba en el plan). Al regenerar la plantilla de un proyecto, antes se saltaba la carpeta `cfg/` entera si ya existía. Con `cfg/image_conversion.mk` versionado, un clon tendría `cfg/` sin el resto de ficheros y no compilaría. Ahora copia lo que falta sin pisar nada (`cp -rn`). Se probó con una copia que solo tenía `src/`, `img/` y `image_conversion.mk`, y compiló.
7. **`.gitignore`:** admite `/*/img/*` y `/*/cfg/image_conversion.mk`.
8. **`CLAUDE.md`:** actualizado con la regla de `img/` y el aviso sobre las variables globales.

**Para usarlo en otro ordenador:** después del `git pull`, basta con `./cpc.sh run claudegame`. El script regenera solo el `Makefile` y el resto de la plantilla; no hace falta llamar a `cpct_mkproject` a mano.
