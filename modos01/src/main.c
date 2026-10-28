//------------------------------------------------------------------------------
// modos01: pantalla partida en dos modos de vídeo a la vez
//
//   - Arriba (líneas 0-79):   MODO 0 -> 160x200, 16 colores, píxeles gordos
//   - Franja negra (80-95):   aquí cae el cambio de modo (no se ve)
//   - Abajo (líneas 96-199):  MODO 1 -> 320x200, 4 colores, píxeles finos
//
// LA IDEA
// -------
// El Gate Array (el chip que convierte bytes de vídeo en colores) consulta el
// modo de vídeo al empezar cada línea. Si cambiamos el modo a mitad de imagen,
// de ahí hacia abajo se pinta con el modo nuevo. La memoria de vídeo NO cambia:
// son los mismos bytes, interpretados de otra forma (en modo 0 un byte son 2
// píxeles de 16 colores; en modo 1, 4 píxeles de 4 colores).
//
// ¿Cómo sabemos "por qué línea va" la pantalla? Por las interrupciones: el CPC
// genera 6 por imagen (300 por segundo), una cada 52 líneas, y una de ellas
// coincide con el VSYNC (el retorno del haz arriba). Contándolas desde el VSYNC:
//
//   int. 0 -> línea ~242 (fuera de la zona visible, en el borde de abajo)
//   int. 1 -> línea ~294 (borde de arriba)
//   int. 2 -> línea ~34  (zona visible)
//   int. 3 -> línea ~86  (zona visible)  <-- aquí pasamos a modo 1
//   int. 4 -> línea ~138
//   int. 5 -> línea ~190
//
// Así que: en la interrupción 0 ponemos modo 0 (para la imagen siguiente) y en
// la 3 ponemos modo 1. Esto se repite 50 veces por segundo y el ojo ve las dos
// zonas a la vez.
//------------------------------------------------------------------------------

#include <cpctelera.h>

// Puerto B del PPI (dirección de E/S &F5xx). Su bit 0 vale 1 mientras dura el
// VSYNC. Con esta declaración de SDCC, leer PPI_B hace un "IN A,(C)" con BC=&F500.
__sfr __banked __at(0xF500) PPI_B;

// Número de interrupción dentro de la imagen actual (0..5).
// No le damos valor inicial (CPCtelera no lo respetaría): se pone a 0 solo
// la primera vez que el manejador detecta el VSYNC.
u8 nInterrupcion;

// Paleta: los colores 0-3 los comparten ambos modos (el modo 1 solo usa esos 4).
// El color 0 es negro, así un byte 0 es negro en los dos modos (útil para la franja).
const u8 paleta[16] = {
   HW_BLACK,          HW_BRIGHT_YELLOW, HW_BRIGHT_CYAN,  HW_BRIGHT_RED,
   HW_BLUE,           HW_BRIGHT_BLUE,   HW_RED,          HW_MAGENTA,
   HW_BRIGHT_MAGENTA, HW_GREEN,         HW_BRIGHT_GREEN, HW_CYAN,
   HW_ORANGE,         HW_PINK,          HW_PASTEL_BLUE,  HW_BRIGHT_WHITE
};

//------------------------------------------------------------------------------
// Manejador de interrupciones: se ejecuta solo, 300 veces por segundo.
//------------------------------------------------------------------------------
void cambiaModo(void) {
   if (PPI_B & 1) {
      // Estamos en el VSYNC: empieza una imagen nueva. La parte de arriba, en modo 0.
      nInterrupcion = 0;
      cpct_setVideoMode(0);
   } else {
      ++nInterrupcion;
      // Tercera interrupción tras el VSYNC (~línea 86): el resto, en modo 1.
      if (nInterrupcion == 3)
         cpct_setVideoMode(1);
   }
}

//------------------------------------------------------------------------------
// Zona de arriba: se dibuja pensando en MODO 0 (px2byteM0, drawStringM0).
//------------------------------------------------------------------------------
void dibujaZonaModo0(void) {
   u8 c;

   // Título en modo 0: cada carácter ocupa 4 bytes (8 píxeles gordos)
   cpct_setDrawCharM0(15, 0);
   cpct_drawStringM0("MODO 0: 16 COLORES", cpct_getScreenPtr(CPCT_VMEM_START, 4, 4));

   // 16 franjas verticales de 5 bytes (10 píxeles de modo 0), una por color
   for (c = 0; c < 16; ++c)
      cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START, c * 5, 24),
                        cpct_px2byteM0(c, c), 5, 48);
}

//------------------------------------------------------------------------------
// Zona de abajo: se dibuja pensando en MODO 1 (px2byteM1, drawStringM1).
//------------------------------------------------------------------------------
void dibujaZonaModo1(void) {
   u8* pvm;
   u8 c;

   // Texto en modo 1: cada carácter ocupa 2 bytes (8 píxeles finos), caben 40
   cpct_setDrawCharM1(1, 0);
   pvm = cpct_getScreenPtr(CPCT_VMEM_START, 0, 104);
   cpct_drawStringM1("MODO 1: 320x200 pixeles, 4 colores.", pvm);
   cpct_setDrawCharM1(2, 0);
   cpct_drawStringM1("Mismos bytes de video, leidos de otra", pvm + 0x50 * 2);
   cpct_drawStringM1("forma: el Gate Array cambia de modo", pvm + 0x50 * 3);
   cpct_drawStringM1("en mitad de la pantalla gracias a una", pvm + 0x50 * 4);
   cpct_drawStringM1("interrupcion.", pvm + 0x50 * 5);

   // 4 cajas, una por cada color del modo 1
   for (c = 0; c < 4; ++c)
      cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START, 4 + c * 19, 160),
                        cpct_px2byteM1(c, c, c, c), 16, 32);
   // La caja del color 0 es negra sobre negro: le damos un marco amarillo
   cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START, 4, 160),
                     cpct_px2byteM1(1, 1, 1, 1), 16, 2);
   cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START, 4, 190),
                     cpct_px2byteM1(1, 1, 1, 1), 16, 2);
}

void main(void) {
   // El firmware vuelve a poner "su" modo en sus propias interrupciones y nos
   // estropearía el truco: lo desactivamos.
   cpct_disableFirmware();

   cpct_setPalette(paleta, 16);
   cpct_setBorder(HW_BLACK);
   cpct_clearScreen(0);          // byte 0 = negro en ambos modos

   dibujaZonaModo0();
   dibujaZonaModo1();

   // A partir de aquí, cada interrupción llama a cambiaModo()
   cpct_setInterruptHandler(cambiaModo);

   // El programa principal no hace nada: todo el trabajo lo hace la interrupción
   while (1);
}
