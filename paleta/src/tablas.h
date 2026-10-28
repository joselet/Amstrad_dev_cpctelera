//------------------------------------------------------------------------------
// tablas: los 27 colores del CPC y la tabla "cada línea tiene su paleta"
//------------------------------------------------------------------------------
#ifndef TABLAS_H
#define TABLAS_H

#include <cpctelera.h>

// Líneas visibles de la pantalla: la rutina de raster las recorre todas
#define LINEAS       200

// La animación de la pantalla 3 se repite cada 54 líneas (27 colores x 2)
#define PERIODO      54

// Geometría de la cuadrícula (pantallas 1 y 2): 9 filas de 17 líneas
//   +0..+7   etiquetas ("13 Blanco")   -> las 3 entradas de la tabla, blanco
//   +8..+15  celdas de color           -> colores 3f, 3f+1 y 3f+2
//   +16      separación                -> blanco (sirve para el marco del negro)
#define Y_REJILLA    40
#define ALTO_FILA    17
#define Y_TITULO     8
#define Y_SUBTITULO  24

// Tabla de colores por línea: 4 bytes por línea { c1, c2, c3, libre }, ya con
// el &40 sumado. Tiene LINEAS + PERIODO filas para poder animar desplazando el
// puntero (pantalla 3).
extern u8  tabla[(LINEAS + PERIODO) * 4];
extern u8* pTabla;               // desde dónde la lee la rutina de raster

extern const u8 hw[27];                    // color de firmware -> hardware
extern const char* const nombreCorto[27];  // para el modo 1 (40 columnas)
extern const char* const nombreLargo[27];  // para el modo 2 (80 columnas)

void tablaPaleta(void);          // pantallas 1 y 2
void tablaAnimada(void);         // pantalla 3
void animaTitulo(u8 t);          // degradado del título, desplazado t pasos

#endif
