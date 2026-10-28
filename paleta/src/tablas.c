//------------------------------------------------------------------------------
// tablas.c: los 27 colores del CPC y el relleno de la tabla de líneas.
//
// LOS 27 COLORES
// --------------
// Cada color mezcla rojo, verde y azul, y cada componente solo puede valer
// 0 (apagado), 1 (medio) o 2 (a tope): 3 x 3 x 3 = 27 colores.
// El número de color de BASIC (INK n) es:
//
//      n = 9 x VERDE + 3 x ROJO + AZUL
//
// Por eso la paleta cabe en una cuadrícula de 9 filas x 3 columnas: en cada
// fila solo cambia el azul (0, 1, 2) y la fila dice el verde y el rojo.
//
// Pero el Gate Array no usa ese número, sino su propio código "de hardware"
// (0..31, desordenado). La tabla hw[] traduce uno a otro; los HW_* de
// CPCtelera están definidos justamente en el orden de BASIC.
//------------------------------------------------------------------------------
#include "tablas.h"

#define C(color)  (0x40 | (color))      // &40 + color = "pon este color"

u8  tabla[(LINEAS + PERIODO) * 4];
u8* pTabla;

const u8 hw[27] = {
   HW_BLACK,        HW_BLUE,           HW_BRIGHT_BLUE,      //  0  1  2
   HW_RED,          HW_MAGENTA,        HW_MAUVE,            //  3  4  5
   HW_BRIGHT_RED,   HW_PURPLE,         HW_BRIGHT_MAGENTA,   //  6  7  8
   HW_GREEN,        HW_CYAN,           HW_SKY_BLUE,         //  9 10 11
   HW_YELLOW,       HW_WHITE,          HW_PASTEL_BLUE,      // 12 13 14
   HW_ORANGE,       HW_PINK,           HW_PASTEL_MAGENTA,   // 15 16 17
   HW_BRIGHT_GREEN, HW_SEA_GREEN,      HW_BRIGHT_CYAN,      // 18 19 20
   HW_LIME,         HW_PASTEL_GREEN,   HW_PASTEL_CYAN,      // 21 22 23
   HW_BRIGHT_YELLOW,HW_PASTEL_YELLOW,  HW_BRIGHT_WHITE      // 24 25 26
};

const char* const nombreCorto[27] = {
   "Negro",     "Azul",      "Azul br.",
   "Rojo",      "Magenta",   "Malva",
   "Rojo br.",  "Purpura",   "Magen.br.",
   "Verde",     "Cian",      "Az.cielo",
   "Amarillo",  "Blanco",    "Az.pastel",
   "Naranja",   "Rosa",      "Mag.past.",
   "Verde br.", "Verde mar", "Cian br.",
   "Lima",      "Verde pas", "Cian pas.",
   "Amar.br.",  "Amar.pas.", "Blanco br."
};

const char* const nombreLargo[27] = {
   "Negro",            "Azul",               "Azul brillante",
   "Rojo",             "Magenta",            "Malva",
   "Rojo brillante",   "Purpura",            "Magenta brillante",
   "Verde",            "Cian",               "Azul cielo",
   "Amarillo",         "Blanco",             "Azul pastel",
   "Naranja",          "Rosa",               "Magenta pastel",
   "Verde brillante",  "Verde mar",          "Cian brillante",
   "Lima",             "Verde pastel",       "Cian pastel",
   "Amarillo brillante","Amarillo pastel",   "Blanco brillante"
};

// Degradados del título y del subtítulo (números de color de BASIC)
static const u8 fuego[16] = { 26,25,24,24,15,15, 6, 6, 3, 6, 6,15,15,24,24,25 };
static const u8 hielo[16] = { 26,23,20,20,11,11, 2, 2, 1, 2, 2,11,11,20,20,23 };

// Pone los 3 colores de una línea
static void linea(u8 y, u8 c1, u8 c2, u8 c3) {
   u8* p = tabla + y * 4;
   p[0] = C(hw[c1]);
   p[1] = C(hw[c2]);
   p[2] = C(hw[c3]);
   p[3] = 0;                     // libre (la rutina lo lee pero no lo usa)
}

//------------------------------------------------------------------------------
// Pantallas 1 y 2. Sirve la misma tabla para los dos modos:
//   - modo 1: c1, c2 y c3 son los colores de las plumas 1, 2 y 3
//   - modo 2: c1, c2 y c3 son los colores de la pluma 1 en las columnas 0, 1, 2
// En las celdas eso son los colores 3f, 3f+1, 3f+2. En el resto (etiquetas,
// título) ponemos el mismo color en las tres entradas y sirve para ambos.
//------------------------------------------------------------------------------
void tablaPaleta(void) {
   u8 y, f, k;

   for (y = 0; y < LINEAS; ++y)
      linea(y, 26, 26, 26);      // por defecto: blanco brillante

   for (f = 0; f < 9; ++f)
      for (k = 0; k < 8; ++k)    // las 8 líneas de las celdas de la fila f
         linea(Y_REJILLA + f * ALTO_FILA + 8 + k, f * 3, f * 3 + 1, f * 3 + 2);

   animaTitulo(0);
   pTabla = tabla;
}

// Desplaza los degradados del título y del subtítulo. Se llama en cada imagen
// y hay poco tiempo, así que escribe directamente en la tabla con punteros en
// vez de llamar a linea() (con linea() tardaba más de 2 ms: lo vimos con el
// truco del borde rojo).
void animaTitulo(u8 t) {
   u8 k, c;
   u8* p1 = tabla + Y_TITULO * 4;
   u8* p2 = tabla + Y_SUBTITULO * 4;

   for (k = 0; k < 8; ++k) {
      c = C(hw[fuego[(k + t) & 15]]);
      *p1++ = c;  *p1++ = c;  *p1++ = c;  ++p1;
      c = C(hw[hielo[(k + t) & 15]]);
      *p2++ = c;  *p2++ = c;  *p2++ = c;  ++p2;
   }
}

//------------------------------------------------------------------------------
// Pantalla 3: cada línea con los colores n, n+9 y n+18 (los 3 niveles de
// verde); n avanza un color cada 2 líneas. Para animar no se recalcula nada:
// en cada imagen la rutina empieza a leer una fila más abajo.
//------------------------------------------------------------------------------
void tablaAnimada(void) {
   u8 y, n;

   for (y = 0; y < LINEAS + PERIODO; ++y) {
      n = (y >> 1) % 27;
      linea(y, n, (n + 9) % 27, (n + 18) % 27);
   }
   pTabla = tabla;
}
