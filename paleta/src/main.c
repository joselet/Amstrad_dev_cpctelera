//------------------------------------------------------------------------------
// paleta: LOS 27 COLORES DEL CPC A LA VEZ, en modos de pocos colores
//
//   Tecla 1: la paleta completa en MODO 1 (4 colores)
//   Tecla 2: la paleta completa en MODO 2 (2 colores)
//   Tecla 3: todo a la vez, animado
//   ESPACIO: siguiente pantalla
//
// Continuación de rasters/ y rasters2/.
//
// LA IDEA: "CADA LÍNEA TIENE SU PROPIA PALETA"
// --------------------------------------------
// Una tabla de 200 entradas (tablas.c), una por línea de pantalla, dice qué 3
// colores tiene esa línea. Una sola rutina de ensamblador la recorre entera en
// cada imagen, de la línea 0 a la 199. El título en degradado, las etiquetas
// blancas y la cuadrícula de colores son solo datos en esa tabla.
//
// Hay dos rutinas, según el modo:
//
//   rasterTresPlumas (modo 1): en cada línea cambian las PLUMAS 1, 2 y 3. Los
//     cambios se hacen en el borde y el retrazado (la parte de la línea que no
//     se ve), así que cada línea entera tiene sus 3 colores, más el negro de la
//     pluma 0.
//
//   rasterTresColumnas (modo 2): solo hay pluma 1 (y la 0 de fondo), así que
//     cambiamos su color 3 VECES DENTRO de cada línea, en plena zona visible
//     (split raster). Cada línea tiene 3 columnas de colores distintos.
//
// SINCRONIZACIÓN
// --------------
// El raster empieza en la interrupción de la línea ~294 (en el borde de arriba,
// ~18 líneas antes de la línea 0) y dura hasta la 200. Para saber cuál es esa
// interrupción, un manejador las cuenta (como en rasters/): la que llega
// durante el VSYNC (línea ~242) es la 0, y la siguiente (~294) es la 1.
//
//   1. halt hasta que nInterrupcion valga 1   (línea ~294)
//   2. DI, raster de las 200 líneas (12,8 ms), EI
//   3. Animar y leer el teclado: tenemos desde la línea 200 hasta la 294 de
//      la imagen siguiente (~6 ms)
//
// (Primero lo hicimos con cpct_waitVSYNC(), como en rasters2/, pero así solo
// quedaban ~3 ms hasta el VSYNC: si nos pasábamos, esa imagen se quedaba sin
// raster y la pantalla salía en blanco. Lo descubrimos poniendo el borde en
// rojo mientras trabajaba C: el rojo llegaba hasta abajo del todo.)
//------------------------------------------------------------------------------

#include <cpctelera.h>
#include "tablas.h"
#include "pantallas.h"

// Puerto B del PPI (&F5xx): su bit 0 vale 1 mientras dura el VSYNC.
__sfr __banked __at(0xF500) PPI_B;

//------------------------------------------------------------------------------
// Ajustes de tiempo (calibrados a ojo en Caprice32). Desde la interrupción de
// la línea ~294 esperamos ESPERA líneas completas (64 µs) y AJUSTE NOPs
// (1 NOP = 1 µs = 8 píxeles de modo 1 o 16 de modo 2).
//------------------------------------------------------------------------------
#define ESPERA_P    15          // rasterTresPlumas
#define AJUSTE_P    59
#define ESPERA_C    15          // rasterTresColumnas
#define AJUSTE_C    59

u16 guardaSP;                   // para recuperar la pila tras el truco del pop

// Número de interrupción desde el VSYNC (0 = la del VSYNC, 1 = línea ~294).
// volatile: la cambia el manejador "por detrás" del bucle principal, así que
// el compilador debe leerla de memoria cada vez.
volatile u8 nInterrupcion;

void cuentaInterrupciones(void) {
   if (PPI_B & 1)
      nInterrupcion = 0;        // estamos en el VSYNC
   else
      ++nInterrupcion;
}

//------------------------------------------------------------------------------
// rasterTresPlumas: las plumas 1, 2 y 3 cambian de color en cada línea.
//
// Para elegir pluma hay que mandar su número (OUT &7F, pluma). Como son la 1,
// la 2 y la 3 seguidas, usamos C: vale 1, se incrementa y luego vuelve a 1.
// "out (c),c" manda el propio C. Así no gastamos registros ni tiempo en cargar
// los números de pluma.
//------------------------------------------------------------------------------
void rasterTresPlumas(void) __naked {
   __asm
      ld    (_guardaSP), sp

      ;--- Esperar hasta justo antes de la línea 0 -------------------------------
      ld    a, #ESPERA_P
   00001$:                    ; 64 µs por vuelta = 1 línea
      .rept 60
         nop
      .endm
      dec   a
      jr    nz, 00001$
      .rept AJUSTE_P
         nop
      .endm

      ld    sp, (_pTabla)     ; SP -> tabla (pop lee 2 bytes en 3 µs)
      ld    bc, #0x7F01       ; B = &7F (Gate Array), C = 1 (pluma 1)
      ld    a, #LINEAS        ; 200 líneas

      ;--- Una vuelta por línea: 64 µs exactos -----------------------------------
   00002$:
      pop   de                ;  3   E = color pluma 1, D = color pluma 2
      pop   hl                ;  3   L = color pluma 3, H = libre
      out   (c), c            ;  4   elegir pluma 1 (C = 1)
      out   (c), e            ;  4   ---> color de la pluma 1
      inc   c                 ;  1   C = 2
      out   (c), c            ;  4   elegir pluma 2
      out   (c), d            ;  4   ---> color de la pluma 2 (9 µs después)
      inc   c                 ;  1   C = 3
      out   (c), c            ;  4   elegir pluma 3
      out   (c), l            ;  4   ---> color de la pluma 3 (9 µs después)
      ld    c, #1             ;  2   C = 1 para la línea siguiente
      .rept 26
         nop                  ; 26   (aquí el haz pinta la zona visible)
      .endm
      dec   a                 ;  1
      jr    nz, 00002$        ;  3   -> 6 + 30 + 2 + 26 + 1 + 3 = 64

      ld    sp, (_guardaSP)
      ret
   __endasm;
}

//------------------------------------------------------------------------------
// rasterTresColumnas: la pluma 1 cambia 3 veces dentro de cada línea, una cada
// 13 µs (26 bytes = 26 caracteres de modo 2). Los OUT caen en la zona visible:
// el color cambia justo donde está el haz en ese momento.
//------------------------------------------------------------------------------
void rasterTresColumnas(void) __naked {
   __asm
      ld    (_guardaSP), sp

      ld    a, #ESPERA_C
   00003$:
      .rept 60
         nop
      .endm
      dec   a
      jr    nz, 00003$
      .rept AJUSTE_C
         nop
      .endm

      ld    bc, #0x7F01
      out   (c), c            ; elegimos la pluma 1 una sola vez
      ld    sp, (_pTabla)
      ld    a, #LINEAS

   00004$:                    ;                      cae en el byte:
      pop   de                ;  3   E = col. 0, D = col. 1
      pop   hl                ;  3   L = col. 2, H = libre
      out   (c), e            ;  4   ---> columna 0       (byte 0)
      .rept 9
         nop                  ;  9
      .endm
      out   (c), d            ;  4   ---> columna 1       (byte 26)
      .rept 9
         nop                  ;  9
      .endm
      out   (c), l            ;  4   ---> columna 2       (byte 52)
      .rept 24
         nop                  ; 24
      .endm
      dec   a                 ;  1
      jr    nz, 00004$        ;  3   -> 6 + 13 + 13 + 4 + 24 + 4 = 64

      ld    sp, (_guardaSP)
      ret
   __endasm;
}

//------------------------------------------------------------------------------
// Cambio de pantalla
//------------------------------------------------------------------------------
u8 pantalla;                    // 0, 1, 2 (teclas 1, 2, 3)

void cambiaPantalla(u8 p) {
   u8 i;

   pantalla = p;
   for (i = 0; i < 16; ++i)     // todo negro mientras dibujamos
      cpct_setPALColour(i, HW_BLACK);

   if (p == 0) {
      cpct_setVideoMode(1);
      cpct_clearScreen(0);
      tablaPaleta();
      dibujaPaletaModo1();
   } else if (p == 1) {
      cpct_setVideoMode(2);
      cpct_clearScreen(0);
      tablaPaleta();
      dibujaPaletaModo2();
   } else {
      cpct_setVideoMode(1);
      cpct_clearScreen(0);
      tablaAnimada();
      dibujaTodoALaVez();
   }
   // La pluma 0 (fondo) es negra siempre; las demás las pone el raster
}

void main(void) {
   u8 t = 0, fase = 0, espacioAntes = 0, espacio, p;

   cpct_disableFirmware();
   cpct_setBorder(HW_BLACK);
   cpct_setInterruptHandler(cuentaInterrupciones);
   cambiaPantalla(0);

   while (1) {
      // --- Sincronización: dormir hasta la interrupción 1 (línea ~294) ---
      do {
         __asm
            halt
         __endasm;
      } while (nInterrupcion != 1);

      // --- Las 200 líneas ---
      __asm
         di
      __endasm;
      if (pantalla == 1)
         rasterTresColumnas();
      else
         rasterTresPlumas();
      __asm
         ei
      __endasm;
      // Durante el raster llegaron interrupciones que se quedaron esperando;
      // la primera entra justo tras el EI. Invalidamos el contador: el VSYNC
      // lo pondrá a 0 y la siguiente a 1.
      nInterrupcion = 99;

      // --- Animación y teclado (hasta la línea ~294 de la imagen siguiente) ---
      ++t;
      if (pantalla == 2) {
         if (++fase == PERIODO) fase = 0;
         pTabla = tabla + fase * 4;          // una línea más abajo cada imagen
      } else {
         animaTitulo(t >> 1);
      }

      // --- Teclado ---
      cpct_scanKeyboard_f();
      p = pantalla;
      if (cpct_isKeyPressed(Key_1)) p = 0;
      if (cpct_isKeyPressed(Key_2)) p = 1;
      if (cpct_isKeyPressed(Key_3)) p = 2;
      espacio = cpct_isKeyPressed(Key_Space) ? 1 : 0;
      if (espacio && !espacioAntes)
         p = (pantalla == 2) ? 0 : pantalla + 1;
      espacioAntes = espacio;
      if (p != pantalla)
         cambiaPantalla(p);
   }
}
