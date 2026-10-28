# KUNG-FU — Petición y plan

## La petición (copia literal)

> He creado un cpct_mkproject kung-fu
>
> Ahora montame un juego de kung fu, a ver de lo que eres capaz. Me gustan los juegos tipo street fighter o tipo karateka o barbarian.
> Quiero poder jugar a dos jugadores cada jugador con una zona de teclado, pero también compatible con joystick, jugador 1, parte izquierda de teclado o joystick. jugador 2 parte derecha del teclado o joystic.
> Genera tú los sprites necesarios y los efectos de sonido y musica que necesites.
>
> Si ves que eres capaz, también una posibilidad de 1 jugador contra cpu puede resultar interesante.
>
> A ver de lo que eres capaz. Dale caña!
>
> Creame un README.md donde pueda ver cómo lo has hecho todo.
>
> Hazme una copia de mi petición y del plan en un fichero PLAN.md

## El plan

### Decisiones técnicas

| Tema | Decisión | Por qué |
|---|---|---|
| Modo de vídeo | **Modo 0** (160x200, 16 colores) | Un juego de lucha luce con color: dos karatekas de distinto color y un fondo de atardecer. |
| Parpadeo | **Doble búfer por hardware**: pantallas en `0xC000` y `0x8000` | Se dibuja en la pantalla oculta y se intercambian: cero parpadeo aunque los luchadores se solapen. |
| Memoria | Código cargado en `0x0100`, pila en `0x8000`, tablas en `0x7C00` | Con dos pantallas solo quedan 32 KB; hay que aprovechar la memoria baja. |
| Fondo | Imagen generada por Python; la franja donde se mueven los luchadores (líneas 100-199) se guarda **en crudo** en memoria | Para borrar un luchador basta copiar ese trozo de fondo: rápido y sin cálculos. |
| Sprites | Generados por Python a partir de un **esqueleto** (ángulos de cadera, rodillas, hombros...) | 16 posturas coherentes sin dibujar píxel a píxel. Se guardan **por tramos** (solo los bytes con dibujo de cada fila). |
| Jugador 2 | Mismos sprites **en espejo y con otros colores**, calculado al dibujar con una tabla | Ahorra la mitad de memoria de sprites. |
| Rutinas críticas | Dibujar y restaurar sprites en **ensamblador** dentro del C | En C serían demasiado lentas para 25 imágenes por segundo. |
| Sonido | Motor propio en C llamado desde la **interrupción** (50 veces/s): música en canales A y B, efectos en C | Así el ritmo de la música no depende de lo que tarde el juego en dibujar. |
| Texto | Fuente propia 5x7 generada por Python | La fuente de la ROM no se puede usar: con el código en memoria baja, activar la ROM taparía nuestro propio programa. |

### Controles

| | Mover | Golpear |
|---|---|---|
| Jugador 1 | `W` `A` `S` `D` o joystick 1 | `ESPACIO` o fuego |
| Jugador 2 | cursores o joystick 2 | `RETURN`, `ENTER` o `COPY`, o fuego |

Los golpes salen de combinar fuego + dirección (como en *International Karate* o *Barbarian*), así funcionan igual con teclado que con joystick de un botón:

- Fuego solo: puñetazo (alto). Fuego + adelante: patada (media). Fuego + arriba: patada alta. Fuego + abajo: barrido (bajo, derriba). En el aire: patada voladora.
- Atrás: retroceder cubriéndose (bloquea golpes altos y medios). Abajo: agacharse (esquiva los altos). Abajo + atrás: bloqueo agachado. Arriba: salto (esquiva barridos).

### Reglas

- Combate a 2 asaltos ganados, 60 segundos por asalto. Si se acaba el tiempo gana quien tenga más energía.
- Modo 1 jugador contra la CPU: cada combate ganado sube el nivel y la CPU reacciona antes y ataca más.

### Pasos

1. Generador en Python (`gen/`): paleta, sprites de esqueleto, fondo, fuente y música → ficheros C en `src/datos/` + PNG de muestra en `img/`.
2. Rutinas de gráficos: dibujar sprites con transparencia (normal y espejo), restaurar fondo, texto, marcador.
3. Motor de sonido por interrupción y efectos.
4. Lógica de luchadores: estados, golpes, bloqueos, impactos, energía.
5. Bucle del juego: asaltos, tiempo, K.O., victoria; pantalla de título con menú.
6. CPU rival con niveles.
7. Probar en el emulador con capturas, ajustar, y escribir el `README.md`.
