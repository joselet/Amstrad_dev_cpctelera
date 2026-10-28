//------------------------------------------------------------------------------
// pantallas.c: lo que se dibuja en cada modo.
//
// Recuerda: aquí solo elegimos PLUMAS. Las zonas marcadas como "raster" tienen
// los colores cambiando en cada línea (o dentro de la línea); el resto usa la
// paleta normal del modo.
//
// Coordenadas: X en BYTES (80 por línea en todos los modos), Y en líneas.
//   modo 0: 1 byte = 2 píxeles, un carácter = 4 bytes -> 20 caracteres por fila
//   modo 1: 1 byte = 4 píxeles, un carácter = 2 bytes -> 40 caracteres
//   modo 2: 1 byte = 8 píxeles, un carácter = 1 byte  -> 80 caracteres
//------------------------------------------------------------------------------
#include <cpctelera.h>
#include "pantallas.h"

#define PANT(x, y)  cpct_getScreenPtr(CPCT_VMEM_START, (x), (y))

// Una franja maciza de todo el ancho (80 bytes). drawSolidBox admite como
// mucho 64 bytes de ancho: la hacemos en dos trozos.
static void franja(u8 y, u8 alto, u8 patron) {
   cpct_drawSolidBox(PANT(0, y),  patron, 40, alto);
   cpct_drawSolidBox(PANT(40, y), patron, 40, alto);
}

//------------------------------------------------------------------------------
// MODO 0: 160x200, 16 plumas
//   líneas 40-119: raster vertical (fondo = pluma 0, letras = pluma 1)
//   líneas 128-199: las 16 plumas con su color normal
//------------------------------------------------------------------------------
void dibujaModo0(void) {
   u8 c;

   cpct_setDrawCharM0(2, 0);
   cpct_drawStringM0("MODO 0: 16 COLORES", PANT(4, 8));
   cpct_setDrawCharM0(3, 0);
   cpct_drawStringM0("Teclas 0,1,2,ESPACIO", PANT(0, 24));

   // --- Zona raster (40-119) ---
   cpct_setDrawCharM0(1, 0);
   cpct_drawStringM0("RASTERS en MODO 0", PANT(6, 48));
   cpct_drawStringM0("Fondo  = pluma 0", PANT(8, 64));
   cpct_drawStringM0("Letras = pluma 1", PANT(8, 80));
   cpct_drawStringM0("cambian cada linea", PANT(4, 96));
   franja(108, 8, cpct_px2byteM0(1, 1));

   // --- Zona normal (128-199) ---
   cpct_setDrawCharM0(1, 0);
   cpct_drawStringM0("Las 16 plumas:", PANT(12, 128));
   for (c = 0; c < 16; ++c)
      cpct_drawSolidBox(PANT(c * 5, 144), cpct_px2byteM0(c, c), 5, 40);
   // La caja de la pluma 0 es negro sobre negro: le ponemos un marco
   cpct_drawSolidBox(PANT(0, 142), cpct_px2byteM0(1, 1), 5, 2);
   cpct_drawSolidBox(PANT(0, 184), cpct_px2byteM0(1, 1), 5, 2);
   cpct_setDrawCharM0(2, 0);
   cpct_drawStringM0("+ rasters = 27 !", PANT(8, 190));
}

//------------------------------------------------------------------------------
// MODO 1: 320x200, 4 plumas
//   líneas 40-79:   raster vertical (plumas 0 y 1)
//   líneas 96-135:  split raster (la pluma 2 cambia 4 veces por línea)
//   líneas 144-199: las 4 plumas con su color normal
//------------------------------------------------------------------------------
void dibujaModo1(void) {
   u8 c;

   cpct_setDrawCharM1(2, 0);
   cpct_drawStringM1("MODO 1: 4 colores por linea... o no", PANT(0, 8));
   cpct_setDrawCharM1(3, 0);
   cpct_drawStringM1("Teclas: 0, 1, 2 o ESPACIO", PANT(0, 24));

   // --- Raster vertical (40-79) ---
   cpct_setDrawCharM1(1, 0);
   cpct_drawStringM1(" El fondo (pluma 0) y las letras", PANT(0, 40));
   cpct_drawStringM1(" (pluma 1) cambian de color en", PANT(0, 48));
   cpct_drawStringM1(" cada linea: RASTERS VERTICALES", PANT(0, 56));
   franja(68, 8, cpct_px2byteM1(1, 1, 1, 1));

   cpct_setDrawCharM1(3, 0);
   cpct_drawStringM1("Y en horizontal? (split raster)", PANT(0, 84));

   // --- Split raster (96-135): todo con la pluma 2 ---
   cpct_setDrawCharM1(2, 0);
   cpct_drawStringM1("1234567890123456789012345678901234567890", PANT(0, 96));
   cpct_drawStringM1("Todo esto es la pluma 2, pero su color", PANT(0, 104));
   cpct_drawStringM1("cambia 4 veces en CADA linea: una cada", PANT(0, 112));
   cpct_drawStringM1("10 letras (10 us). Minimo: cada 4.", PANT(0, 120));
   franja(128, 8, cpct_px2byteM1(2, 2, 2, 2));

   // --- Zona normal (144-199) ---
   cpct_setDrawCharM1(1, 0);
   cpct_drawStringM1("Fuera de los rasters: solo 4 colores", PANT(0, 144));
   for (c = 0; c < 4; ++c)
      cpct_drawSolidBox(PANT(4 + c * 19, 160), cpct_px2byteM1(c, c, c, c), 16, 32);
   cpct_drawSolidBox(PANT(4, 158), cpct_px2byteM1(1, 1, 1, 1), 16, 2);
   cpct_drawSolidBox(PANT(4, 192), cpct_px2byteM1(1, 1, 1, 1), 16, 2);
}

//------------------------------------------------------------------------------
// MODO 2: 640x200, 2 plumas
//   líneas 40-159:  raster vertical (plumas 0 y 1): 2 colores por línea,
//                   pero distintos en cada una
//   líneas 168-191: split raster (la pluma 1 cambia 4 veces por línea)
//------------------------------------------------------------------------------
static const char* const texto2[15] = {
   "En modo 2 cada pixel es 1 bit: solo hay 2 plumas, la 0 (fondo) y la 1 (letras).",
   "Pero el Gate Array lee la paleta mientras el haz dibuja la pantalla. Si en cada",
   "linea cambiamos el color de las dos plumas (4 OUT = 16 us, en el borde, donde no",
   "se ve), cada linea tiene SUS 2 colores.",
   "",
   "Fondo: 16 tonos oscuros. Letras: 16 tonos brillantes. Los 27 colores del CPC",
   "pueden estar en pantalla a la vez, y en el modo de mas resolucion.",
   "",
   "Limites:",
   " - Dentro de una linea siguen siendo 2 colores (salvo cambiando a mitad: abajo).",
   " - Un OUT tarda 4 us = 16 pixeles de modo 2: no se puede cambiar cada caracter.",
   " - El MODO no se puede cambiar a mitad de linea: el Gate Array lo aplica en la",
   "   linea siguiente. Por eso no hay modo 0 y modo 2 en la misma linea.",
   " - Mientras dura el efecto, la CPU no hace nada mas: aqui son 144 lineas (9 ms).",
   ""
};

void dibujaModo2(void) {
   u8 i;

   cpct_setDrawCharM2(1, 0);
   cpct_drawStringM2("MODO 2: 640x200 y 2 colores por linea... pero hasta 27 colores en pantalla", PANT(0, 8));
   cpct_drawStringM2("Teclas: 0, 1, 2 o ESPACIO", PANT(0, 24));

   // --- Raster vertical (40-159) ---
   for (i = 0; i < 15; ++i)
      cpct_drawStringM2(texto2[i], PANT(0, 40 + i * 8));

   // --- Split raster (168-191) ---
   cpct_drawStringM2("12345678901234567890123456789012345678901234567890123456789012345678901234567890", PANT(0, 168));
   cpct_drawStringM2("Split raster: la pluma 1 cambia 4 veces por linea, una cada 20 letras (10 us).", PANT(0, 176));
   franja(184, 8, 0xFF);
}
