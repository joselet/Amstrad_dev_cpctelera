//------------------------------------------------------------------------------
// pantallas.c: lo que se dibuja en cada pantalla.
//
// Aquí solo elegimos PLUMAS. Qué color tiene cada pluma en cada línea lo
// decide la tabla de tablas.c. Por ejemplo: en modo 1 las celdas de la fila f
// se dibujan con las plumas 1, 2 y 3, y la tabla dice que en esas líneas
// esas plumas son los colores 3f, 3f+1 y 3f+2.
//
// Coordenadas: X en BYTES (80 por línea), Y en líneas.
//------------------------------------------------------------------------------
#include <cpctelera.h>
#include "pantallas.h"
#include "tablas.h"

#define PANT(x, y)  cpct_getScreenPtr(CPCT_VMEM_START, (x), (y))

// Columnas de la cuadrícula: cada una ocupa 26 bytes; la celda empieza 2 bytes
// dentro y mide 22 (en modo 2 los cortes de color caen en los bytes 0, 26 y 52,
// justo en los huecos entre celdas)
#define X_COL(k)     ((k) * 26 + 2)
#define ANCHO_CELDA  22

// Escribe "nn " en buf (número de 2 cifras, alineado a la derecha)
static char* numero(char* buf, u8 n) {
   *buf++ = (n >= 10) ? '0' + n / 10 : ' ';
   *buf++ = '0' + n % 10;
   *buf++ = ' ';
   return buf;
}

// Copia una cadena y devuelve el final
static char* copia(char* buf, const char* s) {
   while (*s) *buf++ = *s++;
   return buf;
}

// Escribe "&hh" (el byte que recibe el Gate Array: &40 + código hardware)
static char* hexa(char* buf, u8 v) {
   static const char cifras[] = "0123456789ABCDEF";
   *buf++ = '&';
   *buf++ = cifras[v >> 4];
   *buf++ = cifras[v & 15];
   return buf;
}

//------------------------------------------------------------------------------
// PANTALLA 1: la paleta en MODO 1 (4 plumas)
//------------------------------------------------------------------------------
void dibujaPaletaModo1(void) {
   char buf[16];
   u8 f, k, n, y;

   // Título (pluma 1) y subtítulo (pluma 2): su color lo anima la tabla
   cpct_setDrawCharM1(1, 0);
   cpct_drawStringM1("PALETA COMPLETA DEL CPC", PANT(17, Y_TITULO));
   cpct_setDrawCharM1(2, 0);
   cpct_drawStringM1("27 colores en MODO 1   [teclas 1,2,3]", PANT(2, Y_SUBTITULO));

   for (f = 0; f < 9; ++f) {
      y = Y_REJILLA + f * ALTO_FILA;
      for (k = 0; k < 3; ++k) {
         n = f * 3 + k;
         // Etiqueta con la pluma 1 (en estas líneas la tabla la pone blanca)
         *copia(numero(buf, n), nombreCorto[n]) = 0;
         cpct_setDrawCharM1(1, 0);
         cpct_drawStringM1(buf, PANT(X_COL(k), y));
         // Celda con la pluma k+1 (en estas líneas, el color n)
         cpct_drawSolidBox(PANT(X_COL(k), y + 8),
                           cpct_px2byteM1(k + 1, k + 1, k + 1, k + 1), ANCHO_CELDA, 8);
      }
   }
   // El negro sobre negro no se ve: le ponemos un marco arriba y abajo, con la
   // pluma 1 en líneas donde la tabla la tiene en blanco (+7 y +16)
   cpct_drawSolidBox(PANT(X_COL(0), Y_REJILLA + 7),  cpct_px2byteM1(1, 1, 1, 1), ANCHO_CELDA, 1);
   cpct_drawSolidBox(PANT(X_COL(0), Y_REJILLA + 16), cpct_px2byteM1(1, 1, 1, 1), ANCHO_CELDA, 1);
}

//------------------------------------------------------------------------------
// PANTALLA 2: la paleta en MODO 2 (¡2 plumas!). Todo es la pluma 1; su color
// cambia 3 veces en cada línea (una por columna).
//------------------------------------------------------------------------------
void dibujaPaletaModo2(void) {
   char buf[32];
   char* p;
   u8 f, k, n, y;

   cpct_setDrawCharM2(1, 0);
   cpct_drawStringM2("PALETA COMPLETA DEL CPC", PANT(28, Y_TITULO));
   cpct_drawStringM2("27 colores en MODO 2 (2 colores!): la pluma 1 cambia 3 veces por linea [1,2,3]", PANT(0, Y_SUBTITULO));

   for (f = 0; f < 9; ++f) {
      y = Y_REJILLA + f * ALTO_FILA;
      for (k = 0; k < 3; ++k) {
         n = f * 3 + k;
         // "nn Nombre largo &hh"
         p = copia(numero(buf, n), nombreLargo[n]);
         *p++ = ' ';
         *hexa(p, 0x40 | hw[n]) = 0;
         cpct_drawStringM2(buf, PANT(X_COL(k), y));
         // Celda: en modo 2 un byte con todos los bits a 1 = 8 píxeles de pluma 1
         cpct_drawSolidBox(PANT(X_COL(k), y + 8), 0xFF, ANCHO_CELDA, 8);
      }
   }
   cpct_drawSolidBox(PANT(X_COL(0), Y_REJILLA + 7),  0xFF, ANCHO_CELDA, 1);
   cpct_drawSolidBox(PANT(X_COL(0), Y_REJILLA + 16), 0xFF, ANCHO_CELDA, 1);
}

//------------------------------------------------------------------------------
// PANTALLA 3: TODO A LA VEZ (modo 1). Franjas diagonales de las plumas 1, 2 y
// 3 por toda la pantalla; la tabla animada hace el resto.
//------------------------------------------------------------------------------
void dibujaTodoALaVez(void) {
   u8 fila, s, pluma;

   // 25 filas de 8 líneas x 20 franjas de 4 bytes. Cada fila, las plumas se
   // desplazan una franja: sale una escalera en diagonal.
   for (fila = 0; fila < 25; ++fila)
      for (s = 0; s < 20; ++s) {
         pluma = (s + fila) % 3 + 1;
         cpct_drawSolidBox(PANT(s * 4, fila * 8),
                           cpct_px2byteM1(pluma, pluma, pluma, pluma), 4, 8);
      }

   // Un recuadro negro (pluma 0) con texto: las letras también cambian de
   // color en cada línea
   cpct_drawSolidBox(PANT(4, 64), cpct_px2byteM1(0, 0, 0, 0), 36, 72);
   cpct_drawSolidBox(PANT(40, 64), cpct_px2byteM1(0, 0, 0, 0), 36, 72);
   cpct_setDrawCharM1(1, 0);
   cpct_drawStringM1("TODO A LA VEZ: LOS 27 COLORES", PANT(11, 72));
   cpct_setDrawCharM1(2, 0);
   cpct_drawStringM1("3 plumas x 200 lineas =", PANT(17, 88));
   cpct_drawStringM1("600 cambios de color", PANT(20, 96));
   cpct_drawStringM1("en cada imagen, 50 veces/seg.", PANT(11, 104));
   cpct_setDrawCharM1(3, 0);
   cpct_drawStringM1("Teclas 1, 2, 3 o ESPACIO", PANT(16, 120));
}
