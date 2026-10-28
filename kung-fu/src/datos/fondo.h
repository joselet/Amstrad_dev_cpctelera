// Fichero GENERADO por gen/genera.py: no editar a mano.
// Para cambiarlo, modifica el generador y ejecuta: python3 gen/genera.py
#ifndef FONDO_H
#define FONDO_H
#include <cpctelera.h>

#define Y_CIELO   28   // primera línea del cielo
#define Y_FRANJA  100   // primera línea de la franja de lucha

extern const u8 g_cielo[1391];    // líneas 28-99 comprimidas con RLE
extern const u8 g_franja[8000];  // líneas 100-199 en crudo (80 bytes por línea)

#endif
