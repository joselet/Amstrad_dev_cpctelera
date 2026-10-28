//------------------------------------------------------------------------------
// overscan: dibujar también en el borde de la pantalla
//
// LA IDEA
// -------
// El "borde" no es una zona especial de la pantalla: es simplemente la parte de
// la imagen donde el CRTC (el chip que genera la imagen) ha dejado de leer la
// memoria de vídeo y el Gate Array pinta el color de borde. Quien decide cuánto
// mide la zona que SÍ lee memoria es el CRTC, con sus registros:
//
//   Registro                        Normal    Overscan
//   R1  ancho visible (en palabras)   40        48     -> 80 bytes  -> 96 bytes
//   R2  posición del sincronismo H    46        50     -> centra lo nuevo
//   R6  alto visible (filas de 8 l.)  25        34     -> 200 lín.  -> 272 lín.
//   R7  posición del sincronismo V    30        35     -> centra lo nuevo
//   R12 dirección de inicio (alta)   &30       &2C     -> &C000 16K -> &8000 32K
//   R13 dirección de inicio (baja)   &00       &00
//
// Si ensanchamos/alargamos la zona visible (R1, R6), la imagen empieza antes y
// termina después; moviendo los sincronismos (R2, R7) la recolocamos para que
// quede centrada en el monitor. Resultado: la imagen "se come" el borde.
//
// ¿Y LOS MODOS 0, 1 Y 2?
// ----------------------
// Funciona igual en los tres. El modo lo decide el Gate Array (cuántos píxeles
// saca de cada byte) y el tamaño lo decide el CRTC (cuántos bytes lee). Con 96
// bytes por línea: modo 0 -> 192 píxeles, modo 1 -> 384, modo 2 -> 768. Pulsa
// 0, 1 o 2 para comprobarlo: los bytes son los mismos, solo cambia el modo.
//
// EL PRECIO: 32 KB DE MEMORIA DE VÍDEO
// ------------------------------------
// 96 bytes x 272 líneas = 26112 bytes, que no caben en los 16 KB de siempre.
// Con R12=&2C le decimos al CRTC que use 32 KB, de &8000 a &FFFF:
//   - bits 5-4 = 10 -> empezar en el banco &8000
//   - bits 3-2 = 11 -> cuando el contador interno pasa de 1023 "acarrea" al
//                      banco siguiente (&C000) en vez de dar la vuelta.
// Pero en &A600-&BFFF vive el firmware y la pila del programa: hay que apagar el
// firmware y mover la pila antes de usar esa memoria como pantalla.
//
// Además las direcciones de cada línea ya no son las de siempre, así que
// cpct_getScreenPtr no sirve: las calculamos con dirPantalla() (ver abajo).
//------------------------------------------------------------------------------

#include <cpctelera.h>

#define ANCHO  96    // bytes por línea (R1 x 2)
#define ALTO   272   // líneas (R6 x 8)

// Dónde caería la pantalla normal de 80x200 bytes dentro de la de overscan.
// Horizontal: el sincronismo se ha movido 4 palabras (50-46) -> 8 bytes antes.
// Vertical: la imagen empieza 5 filas antes (en ambos casos hay 39 filas en
// total; normal: 39-30=9 filas tras el VSYNC; overscan: 39-35=4) -> 40 líneas.
#define NORMAL_X   8
#define NORMAL_Y   40
#define NORMAL_AN  80
#define NORMAL_AL  200

// Paleta pensada para los tres modos:
//   - modo 2 usa las tintas 0-1, modo 1 las 0-3, modo 0 las 0-15.
//   - un byte &FF es "todo tinta 15" en modo 0, "todo tinta 3" en modo 1 y
//     "todo tinta 1" en modo 2: por eso 1, 3 y 15 son blanco, y así el marco
//     (dibujado con &FF) se ve blanco en los tres modos.
const u8 paleta[16] = {
   HW_BLACK,       HW_BRIGHT_WHITE,  HW_BRIGHT_RED,   HW_BRIGHT_WHITE,
   HW_BLUE,        HW_BRIGHT_BLUE,   HW_RED,          HW_MAGENTA,
   HW_BRIGHT_YELLOW, HW_GREEN,       HW_BRIGHT_GREEN, HW_CYAN,
   HW_ORANGE,      HW_PINK,          HW_PASTEL_BLUE,  HW_BRIGHT_WHITE
};

//------------------------------------------------------------------------------
// Dirección de memoria del byte x (0-95) de la línea y (0-271).
//
// El CRTC lleva un contador de "palabras" (2 bytes) que avanza 48 por fila de
// caracteres (8 líneas). Para cada línea de pantalla forma la dirección así:
//   - banco:        &8000 o &C000 según si el contador ha pasado de 1023
//   - línea en fila: (y % 8) * &800      (igual que en la pantalla normal)
//   - posición:     (contador % 1024) * 2 + (x par o impar)
//------------------------------------------------------------------------------
u8* dirPantalla(u8 x, u16 y) {
   u16 palabra = (y >> 3) * (ANCHO / 2) + (x >> 1);   // contador del CRTC
   return (u8*)(0x8000
              + ((palabra >> 10) << 14)       // banco: 0 -> &8000, 1 -> &C000
              + ((y & 7) << 11)               // línea dentro de la fila
              + ((palabra & 0x3FF) << 1)      // posición dentro del banco
              + (x & 1));                     // byte par/impar de la palabra
}

//------------------------------------------------------------------------------
// Rellena un rectángulo (en bytes x líneas) con un valor. Va byte a byte porque
// una línea puede cruzar del banco &8000 al &C000 a mitad (lento, pero claro).
//------------------------------------------------------------------------------
void rellena(u8 x, u16 y, u8 an, u16 al, u8 valor) {
   u16 j;
   u8 i;
   for (j = 0; j < al; ++j)
      for (i = 0; i < an; ++i)
         *dirPantalla(x + i, y + j) = valor;
}

// Marco de 1 byte de ancho y 2 líneas de alto, con el valor &FF (blanco).
void marco(u8 x, u16 y, u8 an, u16 al) {
   rellena(x,          y,          an, 2,  0xFF);
   rellena(x,          y + al - 2, an, 2,  0xFF);
   rellena(x,          y,          1,  al, 0xFF);
   rellena(x + an - 1, y,          1,  al, 0xFF);
}

//------------------------------------------------------------------------------
// Pone el CRTC en modo overscan (ver tabla de la cabecera).
//------------------------------------------------------------------------------
void activaOverscan(void) {
   cpct_setCRTCReg(1,  48);
   cpct_setCRTCReg(2,  50);
   cpct_setCRTCReg(6,  34);
   cpct_setCRTCReg(7,  35);
   cpct_setCRTCReg(12, 0x2C);
   cpct_setCRTCReg(13, 0x00);
}

void main(void) {
   u8 c;

   // 1) Apagar el firmware (vive en &A600-&BFFF, que va a ser pantalla) y
   //    llevar la pila a &8000 (crece hacia abajo, lejos de la pantalla).
   cpct_disableFirmware();
   cpct_setStackLocation((void*)0x8000);

   // 2) Borrar los 32 KB de vídeo, poner paleta, borde y modo, y activar el overscan.
   cpct_memset((void*)0x8000, 0, 0x8000);
   cpct_setPalette(paleta, 16);
   cpct_setBorder(HW_BLACK);
   cpct_setVideoMode(1);
   activaOverscan();

   // 3) Fondo: 16 franjas verticales de colores de arriba abajo (6 bytes cada una).
   //    Se dibujan como colores de modo 0; en modo 1 y 2 los mismos bytes se ven
   //    como mezclas de píxeles, pero ocupan exactamente el mismo sitio.
   for (c = 0; c < 16; ++c)
      rellena(c * 6, 0, 6, ALTO, cpct_px2byteM0(c, c));

   // 4) Donde estaría la pantalla normal: negro con marco blanco. Todo lo que
   //    queda fuera de este rectángulo es lo que hemos ganado al borde.
   rellena(NORMAL_X, NORMAL_Y, NORMAL_AN, NORMAL_AL, 0);
   marco(NORMAL_X, NORMAL_Y, NORMAL_AN, NORMAL_AL);

   // 5) Marco en el límite exterior del overscan, para ver hasta dónde llega.
   marco(0, 0, ANCHO, ALTO);

   // 6) Teclas 0, 1, 2: cambiar de modo de vídeo. El overscan se mantiene.
   while (1) {
      cpct_scanKeyboard_f();
      if (cpct_isKeyPressed(Key_0)) cpct_setVideoMode(0);
      if (cpct_isKeyPressed(Key_1)) cpct_setVideoMode(1);
      if (cpct_isKeyPressed(Key_2)) cpct_setVideoMode(2);
   }
}
