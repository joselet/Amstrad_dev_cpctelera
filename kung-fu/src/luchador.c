// luchador.c - lo que hace cada luchador en cada tick (1/25 s)
#include "kungfu.h"

TLuchador g_luch[2];
u8 g_chispa_t, g_chispa_x, g_chispa_y;

// Los ataques: preparación -> golpe (hace daño) -> recuperación
const TAtaque g_ataques[5] = {
   //  preparación     golpe            recup  altura     daño
   { P_GUARDIA,  2,  P_PUNYO,     3,   3,  ALT_ALTO,   6 },   // A_PUNYO
   { P_PREPARA,  3,  P_PATADA,    4,   3,  ALT_MEDIO,  8 },   // A_PATADA
   { P_PREPARA,  4,  P_PAT_ALTA,  4,   4,  ALT_ALTO,  10 },   // A_ALTA
   { P_AGACHADO, 3,  P_BARRIDO,   5,   3,  ALT_BAJO,   7 },   // A_BARRIDO
   { P_SALTO,    0,  P_VOLADORA,  0,   0,  ALT_MEDIO, 10 },   // A_VOLADORA
};

// Altura del salto en cada tick (una parábola)
#define T_SALTO 15
static const u8 altura_salto[T_SALTO] = { 7, 13, 18, 22, 25, 27, 28, 28, 27, 25, 22, 18, 13, 7, 0 };

// Animación de andar
static const u8 ciclo_andar[4] = { P_ANDAR1, P_GUARDIA, P_ANDAR2, P_GUARDIA };

void luchador_inicia(TLuchador* l, u8 x, u8 espejo) {
   l->x = x;
   l->alt = 0;
   l->estado = E_ESPERA;
   l->t = 0;
   l->ataque = A_NINGUNO;
   l->golpeo = 0;
   l->postura = P_GUARDIA;
   l->vx = 0;
   l->vida = VIDA_MAX;
   l->entrada = l->entrada_ant = 0;
   l->espejo = espejo;
   l->paso = 0;
}

// Mueve hacia delante (d > 0) o hacia atrás (d < 0), sin salirse de la pantalla
static void mueve(TLuchador* l, i8 d) {
   i16 x = l->x;
   x += l->espejo ? -d : d;
   if (x < X_MIN) x = X_MIN;
   if (x > X_MAX) x = X_MAX;
   l->x = (u8)x;
}

static void cambia_estado(TLuchador* l, u8 e) {
   l->estado = e;
   l->t = 0;
}

static void inicia_ataque(TLuchador* l, u8 a) {
   cambia_estado(l, E_ATAQUE);
   l->ataque = a;
   l->golpeo = 0;
   l->postura = g_ataques[a].p_prep;
   efecto(SFX_AIRE);
}

// ¿Está pulsando "atrás"? (para bloquear)
u8 bloqueando(TLuchador* l) {
   return l->entrada & (l->espejo ? C_DER : C_IZQ);
}

void luchador_actualiza(TLuchador* l) {
   u8 ent = l->entrada;
   u8 ade = l->espejo ? C_IZQ : C_DER;
   u8 atr = l->espejo ? C_DER : C_IZQ;
   u8 fuego = (ent & C_FUE) && !(l->entrada_ant & C_FUE);   // recién pulsado
   const TAtaque* a;

   l->t++;
   switch (l->estado) {
   case E_NORMAL:
   case E_AGACHADO:
      if (fuego) {
         if (ent & C_ABA)      inicia_ataque(l, A_BARRIDO);
         else if (ent & C_ARR) inicia_ataque(l, A_ALTA);
         else if (ent & ade)   inicia_ataque(l, A_PATADA);
         else                  inicia_ataque(l, A_PUNYO);
      } else if (ent & C_ARR) {
         cambia_estado(l, E_SALTO);
         l->ataque = A_NINGUNO;
         l->vx = (ent & ade) ? 1 : (ent & atr) ? -1 : 0;
         l->postura = P_SALTO;
         efecto(SFX_SALTO);
      } else if (ent & C_ABA) {
         l->estado = E_AGACHADO;
         l->postura = (ent & atr) ? P_BLOQ_AGA : P_AGACHADO;
      } else {
         l->estado = E_NORMAL;
         if (ent & atr) {                 // retrocede cubriéndose (a media velocidad)
            l->postura = P_BLOQUEO;
            if (l->t & 1)
               mueve(l, -1);
         } else if (ent & ade) {
            mueve(l, 1);
            l->paso++;
            l->postura = ciclo_andar[(l->paso >> 1) & 3];
         } else {
            l->postura = P_GUARDIA;
         }
      }
      break;

   case E_SALTO:
      if (l->t >= T_SALTO) {               // aterriza
         l->alt = 0;
         l->ataque = A_NINGUNO;
         l->postura = P_GUARDIA;
         cambia_estado(l, E_NORMAL);
         break;
      }
      l->alt = altura_salto[l->t];
      mueve(l, l->vx);
      if (fuego && l->ataque == A_NINGUNO) {
         l->ataque = A_VOLADORA;
         l->golpeo = 0;
         efecto(SFX_AIRE);
      }
      l->postura = (l->ataque == A_VOLADORA) ? P_VOLADORA : P_SALTO;
      break;

   case E_ATAQUE:
      a = &g_ataques[l->ataque];
      if (l->t < a->t_prep)
         l->postura = a->p_prep;
      else if (l->t < a->t_prep + a->t_golpe)
         l->postura = a->p_golpe;
      else if (l->t < a->t_prep + a->t_golpe + a->t_recup)
         l->postura = a->p_prep;
      else {
         l->ataque = A_NINGUNO;
         l->postura = P_GUARDIA;
         cambia_estado(l, E_NORMAL);
      }
      break;

   case E_GOLPEADO:
      l->postura = P_GOLPEADO;
      l->alt = 0;                         // si le dieron saltando cerca del suelo
      if (l->t < 4)
         mueve(l, -1);
      if (l->t >= 7) {
         l->postura = P_GUARDIA;
         cambia_estado(l, E_NORMAL);
      }
      break;

   case E_CAIDO:     // derribado: cae, se queda en el suelo y se levanta
   case E_KO:        // fuera de combate: cae y ya no se levanta
      if (l->alt)
         l->alt = (l->alt > 5) ? l->alt - 5 : 0;
      if (l->t < 5) {
         l->postura = P_CAYENDO;
         mueve(l, -1);
      } else if (l->t == 5) {
         l->postura = P_CAIDO;
         l->alt = 0;
         efecto(SFX_CAIDA);
      } else if (l->estado == E_CAIDO) {
         if (l->t == 22)
            l->postura = P_AGACHADO;      // se levanta
         else if (l->t >= 27) {
            l->postura = P_GUARDIA;
            cambia_estado(l, E_NORMAL);
         }
      }
      break;

   case E_VICTORIA:
      l->postura = P_VICTORIA;
      break;

   default:   // E_ESPERA
      l->postura = P_GUARDIA;
      break;
   }
   l->entrada_ant = ent;
}

// ---------------------------------------------------------------------------
// Golpes. Devuelve qué pasaría si el atacante "a" golpea ahora al defensor "d":
//   0 = nada (fuera de alcance, no está golpeando o lo esquiva)
//   1 = bloqueado   2 = impacto
// ---------------------------------------------------------------------------
#define G_NADA 0
#define G_BLOQ 1
#define G_IMPACTO 2

static const TAtaque* ataque_activo(TLuchador* l) {
   const TAtaque* a;
   if (l->golpeo)
      return 0;
   if (l->estado == E_SALTO && l->ataque == A_VOLADORA)
      return &g_ataques[A_VOLADORA];
   if (l->estado != E_ATAQUE)
      return 0;
   a = &g_ataques[l->ataque];
   if (l->t < a->t_prep || l->t >= a->t_prep + a->t_golpe)
      return 0;
   return a;
}

static u8 evalua(TLuchador* a, TLuchador* d) {
   const TAtaque* at = ataque_activo(a);
   u8 dist, agachado, en_aire, bloq;

   if (!at || d->estado >= E_GOLPEADO)
      return G_NADA;
   dist = (a->x > d->x) ? a->x - d->x : d->x - a->x;
   if (dist > g_posturas[at->p_golpe].frente + 3)
      return G_NADA;

   a->golpeo = 1;   // este ataque ya no puede golpear más (acierte o falle)
   agachado = (d->estado == E_AGACHADO) || (d->estado == E_ATAQUE && d->ataque == A_BARRIDO);
   en_aire = d->alt > 6;
   bloq = bloqueando(d) && (d->estado == E_NORMAL || d->estado == E_AGACHADO);

   switch (at->altura) {
   case ALT_ALTO:     // puñetazo y patada alta: pasan por encima del agachado
      if (agachado && !en_aire) return G_NADA;
      if (bloq) return G_BLOQ;
      break;
   case ALT_MEDIO:    // patada y voladora: se bloquean de pie o agachado
      if (bloq) return G_BLOQ;
      break;
   default:           // barrido: se esquiva saltando, se bloquea agachado
      if (en_aire) return G_NADA;
      if (bloq && d->estado == E_AGACHADO) return G_BLOQ;
      break;
   }
   return G_IMPACTO;
}

static void aplica(TLuchador* a, TLuchador* d, u8 r) {
   const TAtaque* at;
   if (r == G_NADA)
      return;
   if (r == G_BLOQ) {
      efecto(SFX_BLOQUEO);
      d->vx = 0;
      mueve(d, -2);
      return;
   }
   at = &g_ataques[a->ataque];
   // chispa en el punto del impacto
   g_chispa_t = 6;
   g_chispa_x = a->espejo ? a->x - g_posturas[at->p_golpe].frente + 1
                          : a->x + g_posturas[at->p_golpe].frente - 1;
   g_chispa_y = SUELO - d->alt - (at->altura == ALT_BAJO ? 10 : at->altura == ALT_MEDIO ? 30 : 42);

   d->vida = (d->vida > at->danyo) ? d->vida - at->danyo : 0;
   d->ataque = A_NINGUNO;
   if (!d->vida) {
      cambia_estado(d, E_KO);
      efecto(SFX_KO);
   } else if (at->altura == ALT_BAJO || d->alt > 6) {
      cambia_estado(d, E_CAIDO);      // barrido o golpe en el aire: al suelo
      efecto(SFX_GOLPE);
   } else {
      cambia_estado(d, E_GOLPEADO);
      efecto(SFX_GOLPE);
   }
}

// Se evalúan los dos golpes antes de aplicarlos: si los dos aciertan a la vez,
// los dos reciben (si no, el jugador 1 siempre tendría ventaja).
void luchadores_golpes(void) {
   u8 r0 = evalua(&g_luch[0], &g_luch[1]);
   u8 r1 = evalua(&g_luch[1], &g_luch[0]);
   aplica(&g_luch[0], &g_luch[1], r0);
   aplica(&g_luch[1], &g_luch[0], r1);
}

// Los luchadores no pueden atravesarse: si están demasiado cerca se separan
void luchadores_separa(void) {
   TLuchador* l0 = &g_luch[0];
   TLuchador* l1 = &g_luch[1];
   while (l1->x < l0->x + DIST_MIN) {
      if (l1->x < X_MAX && (l0->x == X_MIN || (l1->x & 1)))
         l1->x++;
      else
         l0->x--;
   }
}
