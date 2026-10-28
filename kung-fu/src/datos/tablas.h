// Fichero GENERADO por gen/genera.py: no editar a mano.
// Para cambiarlo, modifica el generador y ejecuta: python3 gen/genera.py
#ifndef TABLAS_H
#define TABLAS_H
#include <cpctelera.h>

extern const u8 g_paleta[16];          // colores hardware de las 16 tintas
extern const u8 g_tabla_mascara[256];  // byte de sprite -> máscara de transparencia
extern const u8 g_tabla_espejo[256];   // byte de sprite -> byte en espejo y con colores del J2
extern const u8 g_pen_izq[16];         // tinta -> bits del píxel izquierdo
extern const u8 g_pen_der[16];         // tinta -> bits del píxel derecho
extern const u8 g_fuente[413];         // letras 5x7, ASCII 32..90, 7 bytes cada una

#endif
