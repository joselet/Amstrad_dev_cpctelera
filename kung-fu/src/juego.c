// juego.c - asaltos, marcador y dibujado de cada imagen
// Quita las barras del principio para ver en el borde cuánto tarda cada parte
// (rojo: lógica, verde: borrar, azul: dibujar, blanco: marcador, negro: espera)
// y los controles que lee cada jugador.
// #define PERFIL
#include "kungfu.h"

// --- Lo que hay dibujado en cada una de las 2 pantallas ---------------------
// Con doble búfer, cada pantalla se redibuja cada 2 imágenes. Para borrar lo
// que había hay que recordar qué se dibujó en ESA pantalla.
typedef struct {
   const u8* spr;     // 0 = nada
   u8 x, y;           // esquina superior izquierda (x en bytes)
   u8 esp;            // dibujado en espejo
} TObjeto;

#define OBJ_L0     0
#define OBJ_L1     1
#define OBJ_CHISPA 2
static TObjeto hecho[2][3];   // lo que hay en cada pantalla
static TObjeto nuevo[3];      // lo que debería haber ahora

// Marcador dibujado en cada pantalla (0xFF = hay que redibujarlo)
static u8 hud_vida[2][2], hud_rondas[2][2], hud_tiempo[2];
static const char* hud_msg[2];

static const char* mensaje;   // mensaje grande en el cielo (0 = ninguno)
static u8 tiempo;             // segundos que quedan del asalto
static u8 nivel;              // nivel de la CPU (modo 1 jugador)
static u8 jugadores;

#define Y_MSG    48           // zona de mensajes en el cielo
#define Y_MSG_FIN 72

// --- Leer controles ------------------------------------------------------------
static u8 lee_j1(void) {
   u8 c = 0;
   if (cpct_isKeyPressed(Key_W) || cpct_isKeyPressed(Joy0_Up))    c |= C_ARR;
   if (cpct_isKeyPressed(Key_S) || cpct_isKeyPressed(Joy0_Down))  c |= C_ABA;
   if (cpct_isKeyPressed(Key_A) || cpct_isKeyPressed(Joy0_Left))  c |= C_IZQ;
   if (cpct_isKeyPressed(Key_D) || cpct_isKeyPressed(Joy0_Right)) c |= C_DER;
   if (cpct_isKeyPressed(Key_Space) || cpct_isKeyPressed(Joy0_Fire1) || cpct_isKeyPressed(Joy0_Fire2))
      c |= C_FUE;
   return c;
}

static u8 lee_j2(void) {
   u8 c = 0;
   if (cpct_isKeyPressed(Key_CursorUp) || cpct_isKeyPressed(Joy1_Up))       c |= C_ARR;
   if (cpct_isKeyPressed(Key_CursorDown) || cpct_isKeyPressed(Joy1_Down))   c |= C_ABA;
   if (cpct_isKeyPressed(Key_CursorLeft) || cpct_isKeyPressed(Joy1_Left))   c |= C_IZQ;
   if (cpct_isKeyPressed(Key_CursorRight) || cpct_isKeyPressed(Joy1_Right)) c |= C_DER;
   if (cpct_isKeyPressed(Key_Return) || cpct_isKeyPressed(Key_Enter) || cpct_isKeyPressed(Key_Copy)
       || cpct_isKeyPressed(Joy1_Fire1) || cpct_isKeyPressed(Joy1_Fire2))
      c |= C_FUE;
   return c;
}

// --- Marcador --------------------------------------------------------------------
#define BARRA_ANCHO 32         // bytes (VIDA_MAX / 2)

static void hud_barra(u8* pant, u8 j, u8 vida) {
   u8 llena = vida >> 1;
   if (j == 0) {
      caja(pant, 2, 12, llena, 6, 9);
      caja(pant, 2 + llena, 12, BARRA_ANCHO - llena, 6, 7);
   } else {
      caja(pant, 46, 12, BARRA_ANCHO - llena, 6, 7);
      caja(pant, 78 - llena, 12, llena, 6, 9);
   }
}

static void hud_marcas(u8* pant, u8 j, u8 rondas) {
   u8 x0 = (j == 0) ? 2 : 74;
   caja(pant, x0, 21, 4, 4, rondas >= 1 ? 9 : 11);
   caja(pant, (j == 0) ? x0 + 6 : x0 - 6, 21, 4, 4, rondas >= 2 ? 9 : 11);
}

static void hud_reloj(u8* pant, u8 t) {
   char txt[3];
   txt[0] = '0' + t / 10;
   txt[1] = '0' + t % 10;
   txt[2] = 0;
   caja(pant, 34, 2, 12, 14, 0);
   texto(pant, 68, 2, txt, t <= 10 ? 7 : 12, 2, 2);
}

// Dibuja lo fijo (fondo, nombres, marcos) en una pantalla y olvida lo que tenía
static void escena_completa(u8* pant, u8 b) {
   u8 j;
   borra_pantalla(pant);
   pinta_cielo(pant, Y_FRANJA);
   pinta_franja(pant);
   texto(pant, 4, 2, "J1", 12, 1, 1);
   texto(pant, jugadores == 1 ? 138 : 144, 2, jugadores == 1 ? "CPU" : "J2", 12, 1, 1);
   caja(pant, 1, 11, 34, 1, 11);
   caja(pant, 1, 18, 34, 1, 11);
   caja(pant, 45, 11, 34, 1, 11);
   caja(pant, 45, 18, 34, 1, 11);
   for (j = 0; j < 3; j++)
      hecho[b][j].spr = 0;
   hud_vida[b][0] = hud_vida[b][1] = 0xFF;
   hud_rondas[b][0] = hud_rondas[b][1] = 0xFF;
   hud_tiempo[b] = 0xFF;
   hud_msg[b] = 0;
}

// Actualiza en la pantalla oculta solo lo que ha cambiado
static void hud_actualiza(u8* pant, u8 b) {
   u8 j;
   for (j = 0; j < 2; j++) {
      if (hud_vida[b][j] != g_luch[j].vida) {
         hud_vida[b][j] = g_luch[j].vida;
         hud_barra(pant, j, g_luch[j].vida);
      }
      if (hud_rondas[b][j] != g_luch[j].rondas) {
         hud_rondas[b][j] = g_luch[j].rondas;
         hud_marcas(pant, j, g_luch[j].rondas);
      }
   }
   if (hud_tiempo[b] != tiempo) {
      hud_tiempo[b] = tiempo;
      hud_reloj(pant, tiempo);
   }
   if (hud_msg[b] != mensaje) {
      if (hud_msg[b])
         pinta_cielo(pant, Y_MSG_FIN);          // borra el mensaje anterior
      hud_msg[b] = mensaje;
      if (mensaje) {
         texto_centrado(pant, Y_MSG + 1, mensaje, 1, 2, 3);   // sombra
         texto_centrado(pant, Y_MSG, mensaje, 12, 2, 3);
      }
   }
}

// --- Dibujar los luchadores --------------------------------------------------------
// Calcula qué hay que dibujar ahora (sprite y posición de cada objeto)
static void calcula_objetos(void) {
   const TPostura* p;
   TLuchador* l;
   TObjeto* o;
   const u8* spr;
   i16 x;
   u8 j, w;

   for (j = 0; j < 2; j++) {
      l = &g_luch[j];
      o = &nuevo[j];
      p = &g_posturas[l->postura];
      w = p->datos[0];
      x = l->espejo ? (i16)l->x - p->dx - w + 1 : (i16)l->x + p->dx;
      if (x < 0) x = 0;
      if (x > 80 - w) x = 80 - w;
      o->spr = p->datos;
      o->x = (u8)x;
      o->y = SUELO - l->alt + p->oy;
      o->esp = l->espejo;
   }
   o = &nuevo[OBJ_CHISPA];
   if (g_chispa_t) {
      spr = (g_chispa_t & 2) ? g_spr_chispa1 : g_spr_chispa2;
      o->spr = spr;
      o->x = g_chispa_x - (spr[0] >> 1);
      o->y = g_chispa_y - (spr[1] >> 1);
      o->esp = 0;
   } else {
      o->spr = 0;
   }
}

static u8 iguales(TObjeto* a, TObjeto* b) {
   return a->spr == b->spr && a->x == b->x && a->y == b->y && a->esp == b->esp;
}

// ¿Se tocan los rectángulos de dos objetos?
static u8 solapan(TObjeto* a, TObjeto* b) {
   if (!a->spr || !b->spr)
      return 0;
   return a->x < b->x + b->spr[0] && b->x < a->x + a->spr[0] &&
          a->y < b->y + b->spr[1] && b->y < a->y + a->spr[1];
}

// Prepara las variables de las rutinas en ensamblador para el objeto o
static void prepara(u8* pant, TObjeto* o) {
   u8 fila = o->y - Y_FRANJA;
   g_spr = o->spr;
   g_pant = cpct_getScreenPtr(pant, o->x, o->y);
   g_fon = g_franja + ((u16)fila << 6) + ((u16)fila << 4) + o->x;   // fila*80 + x
   g_esp = o->esp;
}

static void dibuja_imagen(void) {
   u8* pant = g_oculta;
   u8 b = graf_indice();
   u8 i, j, k, pasada;
   u8 cambia[3], orden[3];
   TObjeto* o;

   calcula_objetos();

   // 1) ¿Qué objetos han cambiado respecto a lo que hay en ESTA pantalla?
   for (j = 0; j < 3; j++)
      cambia[j] = !iguales(&hecho[b][j], &nuevo[j]);
   // Un objeto que no cambia también hay que repintarlo si otro que sí cambia
   // lo toca (al borrar o al dibujar al otro se estropearía)
   for (pasada = 0; pasada < 2; pasada++)
      for (i = 0; i < 3; i++)
         if (!cambia[i])
            for (j = 0; j < 3; j++)
               if (j != i && cambia[j] && (solapan(&hecho[b][j], &hecho[b][i]) ||
                                           solapan(&nuevo[j], &hecho[b][i])))
                  cambia[i] = 1;

   // 2) Borrar lo que cambia (copiando el fondo encima)
   for (j = 0; j < 3; j++) {
      o = &hecho[b][j];
      if (cambia[j] && o->spr) {
         prepara(pant, o);
         restaura_spr();
      }
   }
#ifdef PERFIL
   cpct_setPALColour(16, 0x15);
#endif

   // 3) Dibujar lo que cambia. El que está atacando va encima del otro.
   orden[0] = (g_luch[0].estado == E_ATAQUE) ? 1 : 0;
   orden[1] = orden[0] ^ 1;
   orden[2] = OBJ_CHISPA;
   for (k = 0; k < 3; k++) {
      j = orden[k];
      if (!cambia[j])
         continue;
      o = &hecho[b][j];
      o->spr = nuevo[j].spr;
      o->x = nuevo[j].x;
      o->y = nuevo[j].y;
      o->esp = nuevo[j].esp;
      if (o->spr) {
         prepara(pant, o);
         dibuja_spr();
      }
   }
#ifdef PERFIL
   cpct_setPALColour(16, 0x0B);
#endif

   // 4) Marcador
   hud_actualiza(pant, b);
#ifdef PERFIL
   {  // depuración: bits de control del J1 y del J2
      char t[6];
      t[0] = '0' + (g_luch[0].entrada >> 4); t[1] = '0' + (g_luch[0].entrada & 15);
      t[2] = ' ';
      t[3] = '0' + (g_luch[1].entrada >> 4); t[4] = '0' + (g_luch[1].entrada & 15);
      t[5] = 0;
      caja(pant, 12, 2, 18, 7, 0);
      texto(pant, 24, 2, t, 9, 1, 1);
   }
#endif
}

// --- Un asalto -------------------------------------------------------------------
#define RES_J1     0
#define RES_J2     1
#define RES_EMPATE 2
#define RES_SALIR  3

static const char* const txt_asalto[5] = { "ASALTO 1", "ASALTO 2", "ASALTO 3", "ASALTO 4", "ASALTO 5" };

static u8 asalto(u8 numero) {
   u8 t = 0, fase = 0, ticks_seg = 0, res = RES_EMPATE, j;

   luchador_inicia(&g_luch[0], 22, 0);
   luchador_inicia(&g_luch[1], 57, 1);
   g_chispa_t = 0;
   tiempo = 60;
   mensaje = txt_asalto[numero];
   cpu_reinicia();
   escena_completa(PANT_A, 0);
   escena_completa(PANT_B, 1);
   musica(0);

   // fase 0: presentación  1: combate  2: final del asalto
   while (1) {
      cpct_scanKeyboard_f();
      if (cpct_isKeyPressed(Key_Esc))
         return RES_SALIR;
      t++;

      if (fase == 0) {
         if (t == 40) {
            mensaje = "LUCHA!";
            efecto(SFX_GONG);
         } else if (t == 65) {
            mensaje = 0;
            fase = 1;
            t = 0;
            g_luch[0].estado = g_luch[1].estado = E_NORMAL;
            musica(MUS_COMBATE);
         }
      } else if (fase == 1) {
         g_luch[0].entrada = lee_j1();
         g_luch[1].entrada = (jugadores == 1) ? cpu_entrada(&g_luch[1], &g_luch[0], nivel) : lee_j2();
         // fin del asalto por K.O. o por tiempo
         if (g_luch[0].estado == E_KO || g_luch[1].estado == E_KO) {
            res = (g_luch[0].estado == E_KO) ? RES_J2 : RES_J1;
            mensaje = "K.O.";
            fase = 2;
            t = 0;
            musica(0);
         } else if (++ticks_seg == 25) {
            ticks_seg = 0;
            if (--tiempo == 0) {
               mensaje = "TIEMPO";
               res = (g_luch[0].vida > g_luch[1].vida) ? RES_J1 :
                     (g_luch[1].vida > g_luch[0].vida) ? RES_J2 : RES_EMPATE;
               fase = 2;
               t = 0;
               musica(0);
               efecto(SFX_GONG);
            }
         }
      } else {
         // fase 2: nadie toca nada; el ganador celebra
         for (j = 0; j < 2; j++)
            g_luch[j].entrada = 0;
         if (t == 30 && res != RES_EMPATE) {
            TLuchador* g = &g_luch[res];
            if (g->estado <= E_ATAQUE || g->estado == E_GOLPEADO) {
               g->alt = 0;
               g->estado = E_VICTORIA;
            }
         }
         if (t == 30 && res == RES_EMPATE)
            mensaje = "EMPATE";
         if (t >= 75 && (res == RES_EMPATE || g_luch[res].estado == E_VICTORIA))
            return res;
         if (t >= 120)
            return res;
      }

      luchador_actualiza(&g_luch[0]);
      luchador_actualiza(&g_luch[1]);
      if (fase == 1)
         luchadores_golpes();
      luchadores_separa();
      if (g_chispa_t)
         g_chispa_t--;

#ifdef PERFIL
      cpct_setPALColour(16, 0x12);     // borde verde: dibujando
#endif
      dibuja_imagen();
#ifdef PERFIL
      cpct_setPALColour(16, 0x14);     // borde negro: esperando
#endif
      graf_cambia();
#ifdef PERFIL
      cpct_setPALColour(16, 0x0C);     // borde rojo: lógica del juego
#endif
   }
}

// Espera unos ticks redibujando (para mensajes entre asaltos). Devuelve 1 si ESC.
static u8 pausa(u8 ticks) {
   while (ticks--) {
      cpct_scanKeyboard_f();
      if (cpct_isKeyPressed(Key_Esc))
         return 1;
      luchador_actualiza(&g_luch[0]);
      luchador_actualiza(&g_luch[1]);
      dibuja_imagen();
      graf_cambia();
   }
   return 0;
}

// --- Una partida completa -----------------------------------------------------------
void partida(u8 njug) {
   u8 r, numero, ganador, ronda0, ronda1;
   char txt_nivel[9];

   jugadores = njug;
   nivel = 1;
   while (1) {
      ronda0 = ronda1 = 0;
      numero = 0;
      ganador = RES_EMPATE;
      // asaltos hasta que alguien gane 2 (como mucho 5)
      while (numero < 5) {
         g_luch[0].rondas = ronda0;     // el marcador lee las rondas de aquí
         g_luch[1].rondas = ronda1;
         r = asalto(numero);
         if (r == RES_SALIR) {
            musica(0);
            return;
         }
         if (r == RES_J1) ronda0++;
         if (r == RES_J2) ronda1++;
         g_luch[0].rondas = ronda0;
         g_luch[1].rondas = ronda1;
         numero++;
         if (ronda0 == 2 || ronda1 == 2)
            break;
      }
      if (ronda0 > ronda1) ganador = RES_J1;
      else if (ronda1 > ronda0) ganador = RES_J2;

      // Resultado del combate
      if (jugadores == 2) {
         mensaje = (ganador == RES_J1) ? "GANA J1" : (ganador == RES_J2) ? "GANA J2" : "EMPATE";
         musica(MUS_VICTORIA);
         pausa(100);
         musica(0);
         return;
      }
      if (ganador == RES_J1) {
         mensaje = "HAS GANADO!";
         musica(MUS_VICTORIA);
         if (pausa(70)) return;
         nivel++;
         txt_nivel[0] = 'N'; txt_nivel[1] = 'I'; txt_nivel[2] = 'V'; txt_nivel[3] = 'E';
         txt_nivel[4] = 'L'; txt_nivel[5] = ' ';
         txt_nivel[6] = (nivel >= 10) ? '0' + nivel / 10 : '0' + nivel;
         txt_nivel[7] = (nivel >= 10) ? '0' + nivel % 10 : 0;
         txt_nivel[8] = 0;
         mensaje = txt_nivel;
         if (pausa(50)) return;
      } else {
         mensaje = "FIN DEL JUEGO";
         musica(MUS_DERROTA);
         pausa(100);
         musica(0);
         return;
      }
   }
}
