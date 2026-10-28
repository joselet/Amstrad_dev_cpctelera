//------------------------------------------------------------------------------
// rasters: MODO 1 con muchos más de 4 colores en pantalla
//
//   - Líneas 0-39:    zona normal (4 colores)
//   - Líneas 40-79:   el título. Todo lo de esta franja se dibuja con la PLUMA 1,
//                     pero el COLOR de la pluma 1 cambia en cada línea -> degradado
//   - Líneas 80-199:  zona normal otra vez (los mismos 4 colores de siempre)
//
// LA IDEA
// -------
// En modo 1 cada píxel ocupa 2 bits, así que solo puede valer 0, 1, 2 o 3: es
// el número de PLUMA (PEN). La memoria de vídeo no guarda colores, guarda
// plumas. El color real de cada pluma (INK) lo decide el Gate Array, el chip que
// convierte los bytes de vídeo en señal para el monitor, mirando su paleta.
//
// El truco: el Gate Array consulta la paleta EN EL MOMENTO de pintar cada píxel.
// Si cambiamos el color de la pluma 1 mientras el haz del monitor va bajando, lo
// que ya se ha pintado se queda como estaba y lo que falta sale con el color
// nuevo. Cambiándolo una vez por línea, cada línea tiene su propio color.
// Por eso cada línea sigue teniendo solo 4 colores, pero la pantalla entera
// puede tener los 27 del CPC. A esto se le llama "rasters".
//
// CÓMO SE CAMBIA UN COLOR
// -----------------------
// El Gate Array se programa con OUT al puerto &7Fxx (B = &7F), en dos pasos:
//   OUT &7F, &00 + pluma    -> "voy a cambiar esta pluma" (0..15, 16 = borde)
//   OUT &7F, &40 + color    -> "ponle este color" (color de hardware 0..31)
// La pluma elegida se recuerda, así que para cambiar muchas veces el color de
// la misma pluma basta con elegirla una vez y luego solo mandar colores.
// (cpct_setPALColour hace exactamente esto, pero es demasiado lenta para
// llamarla en cada línea con el tiempo exacto.)
//
// EL TIEMPO LO ES TODO
// --------------------
// Una línea de pantalla tarda 64 microsegundos. En el CPC todas las
// instrucciones del Z80 duran un número entero de microsegundos ("NOPs": un
// NOP dura 1 µs), así que podemos escribir un bucle que tarde EXACTAMENTE 64 µs
// por vuelta: cada vuelta cambia el color justo una línea después que la
// anterior. Esa parte va en ensamblador porque en C no controlamos cuánto
// tarda cada cosa.
//
// ¿Y cómo sabemos por qué línea va el haz? Igual que en modos01: el CPC lanza
// 6 interrupciones por imagen (una cada 52 líneas) y la del VSYNC se reconoce
// leyendo el PPI. La 2ª tras el VSYNC llega hacia la línea 34; desde ahí
// esperamos unas líneas más y empezamos el degradado en la 40.
//------------------------------------------------------------------------------

#include <cpctelera.h>

// Puerto B del PPI (&F5xx): su bit 0 vale 1 mientras dura el VSYNC.
__sfr __banked __at(0xF500) PPI_B;

// Número de interrupción dentro de la imagen actual (0..5). Sin valor inicial:
// CPCtelera no lo respetaría (se pone a 0 en el primer VSYNC).
u8 nInterrupcion;

// Color "normal" de la pluma 1, el que tiene fuera de la franja del título.
#define COLOR_PLUMA1   HW_BRIGHT_WHITE

// Ajuste fino del momento en que empieza el degradado (se calibra a ojo):
//   ESPERA_LINEAS: líneas completas (64 µs) que esperamos tras la interrupción
//   ESPERA_NOPS:   microsegundos extra, para mover el corte en horizontal y
//                  que caiga en el borde izquierdo y no en mitad de las letras
#define ESPERA_LINEAS  4
#define ESPERA_NOPS    40

// Líneas que dura el degradado (una entrada de la tabla por línea)
#define LINEAS_RASTER  40

// Un color por línea. Se guardan ya con el &40 sumado (C(olor) = &40 + color),
// para que el bucle de ensamblador solo tenga que leerlo y mandarlo.
#define C(color)  (0x40 | (color))
const u8 degradado[LINEAS_RASTER] = {
   // Líneas 40-47: primera fila de texto, "fuego"
   C(HW_BRIGHT_WHITE),   C(HW_PASTEL_YELLOW),  C(HW_BRIGHT_YELLOW), C(HW_YELLOW),
   C(HW_ORANGE),         C(HW_BRIGHT_RED),     C(HW_RED),           C(HW_MAGENTA),
   // Líneas 48-55: segunda fila, "mar"
   C(HW_BRIGHT_WHITE),   C(HW_PASTEL_CYAN),    C(HW_BRIGHT_CYAN),   C(HW_SKY_BLUE),
   C(HW_CYAN),           C(HW_BRIGHT_BLUE),    C(HW_PASTEL_BLUE),   C(HW_BLUE),
   // Líneas 56-63: tercera fila, "bosque"
   C(HW_PASTEL_YELLOW),  C(HW_LIME),           C(HW_PASTEL_GREEN),  C(HW_BRIGHT_GREEN),
   C(HW_SEA_GREEN),      C(HW_GREEN),          C(HW_CYAN),          C(HW_GREEN),
   // Líneas 64-71: cuarta fila, "atardecer"
   C(HW_PINK),           C(HW_PASTEL_MAGENTA), C(HW_BRIGHT_MAGENTA),C(HW_MAUVE),
   C(HW_MAGENTA),        C(HW_PURPLE),         C(HW_WHITE),         C(HW_PURPLE),
   // Líneas 72-79: la barra maciza, un arco iris
   C(HW_BRIGHT_RED),     C(HW_ORANGE),         C(HW_BRIGHT_YELLOW), C(HW_BRIGHT_GREEN),
   C(HW_BRIGHT_CYAN),    C(HW_BRIGHT_BLUE),    C(HW_BRIGHT_MAGENTA),C(HW_PINK)
};

// Paleta "normal" de las 4 plumas del modo 1
const u8 paleta[4] = { HW_BLACK, COLOR_PLUMA1, HW_BRIGHT_YELLOW, HW_BRIGHT_CYAN };

//------------------------------------------------------------------------------
// El degradado. Se llama desde la interrupción 2 (línea ~34).
//
// __naked: SDCC no añade nada antes ni después; solo nuestro código, así
// sabemos exactamente cuánto tarda. Los números de la derecha son los µs
// (NOPs) de cada instrucción en el CPC.
//------------------------------------------------------------------------------
void rasterTitulo(void) __naked {
   __asm
      ld    bc, #0x7F01       ; B = &7F (puerto del Gate Array), C = 1 (pluma 1)
      out   (c), c            ; seleccionamos la pluma 1: a partir de ahora
                              ;   cada OUT (C),A cambia SU color

      ;--- 1) Esperar hasta la línea 40 ----------------------------------------
      ld    d, #ESPERA_LINEAS
   00001$:                    ; cada vuelta = 64 µs = una línea
      .rept 60
         nop                  ; 60 x 1 = 60
      .endm
      dec   d                 ;  1
      jr    nz, 00001$        ;  3   -> 60 + 1 + 3 = 64

      .rept ESPERA_NOPS       ; ajuste fino en horizontal
         nop
      .endm

      ;--- 2) Un color nuevo en cada línea -------------------------------------
      ld    hl, #_degradado   ; HL apunta al color de la primera línea
      ld    e, #LINEAS_RASTER ; E = líneas que faltan
   00002$:                    ; cada vuelta = 64 µs = una línea
      ld    a, (hl)           ;  2   A = color de esta línea (ya con el &40)
      out   (c), a            ;  4   ¡cambia el color de la pluma 1 YA!
      inc   hl                ;  2   siguiente color de la tabla
      .rept 52
         nop                  ; 52   no hacemos nada: esperamos a la línea siguiente
      .endm
      dec   e                 ;  1
      jr    nz, 00002$        ;  3   -> 2+4+2+52+1+3 = 64

      ;--- 3) Devolver a la pluma 1 su color normal ----------------------------
      ld    a, (_paleta + 1)  ; color normal de la pluma 1 (paleta[1])
      or    #0x40             ; le sumamos el &40 de "poner color"
      out   (c), a            ; el resto de la pantalla, con la paleta de siempre
      ret
   __endasm;
}

//------------------------------------------------------------------------------
// Manejador de interrupciones: se ejecuta solo, 300 veces por segundo.
//------------------------------------------------------------------------------
void interrupcion(void) {
   if (PPI_B & 1) {
      // VSYNC: empieza una imagen nueva
      nInterrupcion = 0;
   } else if (++nInterrupcion == 2) {
      // 2ª interrupción tras el VSYNC: el haz va por la línea ~34
      rasterTitulo();
   }
}

//------------------------------------------------------------------------------
// Dibujo. Ojo: aquí solo elegimos PLUMAS; los colores los pone la paleta (y
// los rasters).
//------------------------------------------------------------------------------
void dibujaPantalla(void) {
   u8* pvm;
   u8 c;

   // Arriba (líneas 8-31): zona normal
   cpct_setDrawCharM1(2, 0);
   cpct_drawStringM1("Arriba: paleta normal, 4 colores", cpct_getScreenPtr(CPCT_VMEM_START, 4, 8));
   cpct_setDrawCharM1(1, 0);
   cpct_drawStringM1("Esta linea usa la pluma 1 (blanco)", cpct_getScreenPtr(CPCT_VMEM_START, 4, 24));

   // Franja del título (líneas 40-79): TODO con la pluma 1
   cpct_setDrawCharM1(1, 0);
   pvm = cpct_getScreenPtr(CPCT_VMEM_START, 0, 40);
   cpct_drawStringM1("  MODO 1 CON MUCHOS MAS DE 4 COLORES", pvm);
   cpct_drawStringM1("  Todo esto se dibuja con la PLUMA 1,", pvm + 0x50 * 1);
   cpct_drawStringM1("  pero su color cambia en cada linea", pvm + 0x50 * 2);
   cpct_drawStringM1("  del monitor: son los R A S T E R S", pvm + 0x50 * 3);
   // Barra maciza de pluma 1 (líneas 72-79): deja ver un color por línea
   cpct_drawSolidBox(pvm + 0x50 * 4, cpct_px2byteM1(1, 1, 1, 1), 64, 8);
   cpct_drawSolidBox(pvm + 0x50 * 4 + 64, cpct_px2byteM1(1, 1, 1, 1), 16, 8);

   // Abajo (líneas 96-199): zona normal, con texto y gráficos
   cpct_setDrawCharM1(2, 0);
   pvm = cpct_getScreenPtr(CPCT_VMEM_START, 0, 96);
   cpct_drawStringM1("Abajo: paleta normal otra vez. Cada", pvm);
   cpct_drawStringM1("linea del CPC solo tiene 4 colores,", pvm + 0x50 * 1);
   cpct_drawStringM1("pero la pantalla entera puede tener", pvm + 0x50 * 2);
   cpct_drawStringM1("los 27: basta con cambiar la paleta", pvm + 0x50 * 3);
   cpct_drawStringM1("mientras el haz va bajando.", pvm + 0x50 * 4);

   // Las 4 plumas de esta zona: una caja por pluma
   cpct_setDrawCharM1(3, 0);
   cpct_drawStringM1("Las 4 plumas de aqui abajo:", cpct_getScreenPtr(CPCT_VMEM_START, 0, 144));
   for (c = 0; c < 4; ++c)
      cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START, 4 + c * 19, 160),
                        cpct_px2byteM1(c, c, c, c), 16, 32);
   // La caja de la pluma 0 es negro sobre negro: le ponemos un marco
   cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START, 4, 160), cpct_px2byteM1(2, 2, 2, 2), 16, 2);
   cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START, 4, 190), cpct_px2byteM1(2, 2, 2, 2), 16, 2);
}

void main(void) {
   // El firmware vuelve a poner "su" paleta en sus interrupciones: fuera.
   cpct_disableFirmware();

   cpct_setVideoMode(1);
   cpct_setPalette(paleta, 4);
   cpct_setBorder(HW_BLACK);
   cpct_clearScreen(0);

   dibujaPantalla();

   // A partir de aquí, cada interrupción llama a interrupcion()
   cpct_setInterruptHandler(interrupcion);

   // HALT detiene el Z80 hasta la siguiente interrupción. Así la interrupción
   // siempre empieza en el mismo instante (no a mitad de una instrucción más o
   // menos larga) y el degradado no tiembla.
   while (1) {
      __asm
         halt
      __endasm;
   }
}
