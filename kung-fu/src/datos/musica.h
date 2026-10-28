// Fichero GENERADO por gen/genera.py: no editar a mano.
// Para cambiarlo, modifica el generador y ejecuta: python3 gen/genera.py
#ifndef MUSICA_H
#define MUSICA_H
#include <cpctelera.h>

typedef struct {
   const u8* canal_a;
   const u8* canal_b;
   u8 vol_a, vol_b;   // volumen inicial de cada nota (0-15)
} TCancion;

#define NOTA_PERCUSION 0x7F
#define MUS_BUCLE 0xFF
#define MUS_FIN 0xFE

#define MUS_TITULO 1
#define MUS_COMBATE 2
#define MUS_VICTORIA 3
#define MUS_DERROTA 4
#define NUM_CANCIONES 4

extern const u16 g_periodos[61];
extern const TCancion g_canciones[NUM_CANCIONES];

#endif
