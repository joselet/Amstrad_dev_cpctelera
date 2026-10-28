// cpu.c - el rival controlado por el ordenador
//
// La CPU no hace trampas: decide qué "teclas" pulsar (los mismos bits C_xxx
// que el teclado o el joystick) y la lógica del luchador hace el resto.
// El nivel controla lo rápido que reacciona y lo a menudo que acierta a defenderse.
#include "kungfu.h"

static u8 espera;   // ticks hasta la próxima decisión
static u8 accion;   // controles que mantiene pulsados
static u8 dur;      // ticks que le quedan a la acción

void cpu_reinicia(void) {
   espera = 25;
   accion = 0;
   dur = 0;
}

// Probabilidad (sobre 256) que crece con el nivel, con un tope
static u8 prob(u8 base, u8 por_nivel, u8 nivel) {
   u16 p = base + (u16)por_nivel * nivel;
   return (p > 235) ? 235 : (u8)p;
}

u8 cpu_entrada(TLuchador* yo, TLuchador* riv, u8 nivel) {
   u8 ade = C_IZQ, atr = C_DER;        // la CPU es el jugador 2: mira a la izquierda
   u8 dist = yo->x - riv->x;
   u8 r = cpct_rand();
   const TAtaque* a;

   // En el aire: patada voladora si el rival está cerca
   if (yo->estado == E_SALTO) {
      if (yo->ataque == A_NINGUNO && yo->alt > 14 && dist < 18 && r < prob(90, 20, nivel))
         return C_FUE;
      return 0;
   }
   if (yo->estado != E_NORMAL && yo->estado != E_AGACHADO) {
      accion = dur = 0;
      return 0;
   }

   // Defensa: el rival está preparando un golpe y estamos a su alcance
   if (riv->estado == E_ATAQUE && riv->t <= 1) {
      a = &g_ataques[riv->ataque];
      if (dist <= g_posturas[a->p_golpe].frente + 4 && r < prob(40, 22, nivel)) {
         r = cpct_rand();
         if (a->altura == ALT_BAJO)
            accion = (r & 1) ? C_ARR : (C_ABA | atr);
         else if (a->altura == ALT_ALTO)
            accion = (r & 1) ? C_ABA : atr;
         else
            accion = (r & 1) ? atr : (C_ABA | atr);
         dur = 8;
         return accion;
      }
   }

   // El rival salta hacia nosotros: patada alta para recibirle
   if (riv->estado == E_SALTO && riv->ataque == A_VOLADORA && dist < 15 && r < prob(30, 18, nivel)) {
      dur = 0;
      espera = 6;
      return C_FUE | C_ARR;
   }

   if (dur) {
      dur--;
      return accion;
   }
   if (espera) {
      espera--;
      return 0;
   }

   // Nueva decisión
   r = cpct_rand();
   if (riv->estado == E_CAIDO || riv->estado == E_KO) {
      accion = 0;                       // esperar a que se levante
      dur = 4;
   } else if (dist > 13) {
      if (r < 30) {                     // salto hacia delante (quizá con patada)
         accion = C_ARR | ade;
         dur = 1;
      } else {                          // acercarse
         accion = ade;
         dur = 3 + (r & 7);
      }
   } else {
      switch (r & 7) {
         case 0: case 1: accion = C_FUE;          break;   // puñetazo
         case 2: case 3: accion = C_FUE | ade;    break;   // patada
         case 4:         accion = C_FUE | C_ARR;  break;   // patada alta
         case 5: case 6: accion = C_FUE | C_ABA;  break;   // barrido
         default:        accion = atr; dur = 6;   break;   // retroceder
      }
      espera = (nivel < 6 ? 16 - nivel * 2 : 4) + (cpct_rand() & 3);
      return accion;
   }
   return accion;
}
