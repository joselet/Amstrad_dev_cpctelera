// Fichero GENERADO por gen/genera.py: no editar a mano.
// Para cambiarlo, modifica el generador y ejecuta: python3 gen/genera.py
#ifndef SPRITES_H
#define SPRITES_H
#include <cpctelera.h>

// Una postura: datos por tramos + posición respecto al ancla del luchador
typedef struct {
   const u8* datos;  // ancho, alto, y filas (inicio, longitud, bytes)
   i8 dx;            // byte izquierdo relativo al ancla
   i8 oy;            // fila superior relativa al suelo (negativa)
   i8 frente;        // alcance por delante, en bytes
} TPostura;

#define P_GUARDIA 0
#define P_ANDAR1 1
#define P_ANDAR2 2
#define P_PUNYO 3
#define P_PREPARA 4
#define P_PATADA 5
#define P_PAT_ALTA 6
#define P_AGACHADO 7
#define P_BARRIDO 8
#define P_SALTO 9
#define P_VOLADORA 10
#define P_BLOQUEO 11
#define P_BLOQ_AGA 12
#define P_GOLPEADO 13
#define P_CAYENDO 14
#define P_CAIDO 15
#define P_VICTORIA 16
#define NUM_POSTURAS 17

extern const TPostura g_posturas[NUM_POSTURAS];
extern const u8 g_spr_chispa1[];
extern const u8 g_spr_chispa2[];

#endif
