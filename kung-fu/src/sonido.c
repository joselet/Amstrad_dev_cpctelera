// sonido.c - motor de música y efectos para el AY-3-8912
//
// Todo el sonido se calcula dentro de la interrupción, una vez por cuadro
// (50 veces por segundo). El programa principal solo pide cosas cambiando
// una variable (g_pide_musica, g_pide_efecto), así nunca se pisan.
//
// Canal A: melodía. Canal B: bajo y percusión. Canal C: efectos.
#include "kungfu.h"

volatile u8 g_cuadros;

// Copia de los registros 0-10 del AY. Se calculan en C y se envían de golpe.
//  0-1 periodo A, 2-3 periodo B, 4-5 periodo C, 6 ruido, 7 mezclador,
//  8-10 volumen de A, B y C
u8 g_psg[11];

// --- Efectos -----------------------------------------------------------------
typedef struct {
   u16 tono;      // periodo inicial (0 = sin tono)
   i8  dtono;     // cuánto cambia el periodo cada cuadro (+ = más grave)
   u8  ruido;     // periodo de ruido inicial (0 = sin ruido)
   i8  druido;
   u8  vol;       // volumen inicial
   u8  caida;     // cada cuántos cuadros baja 1 el volumen
   u8  dur;       // duración en cuadros
} TEfecto;

static const TEfecto efectos[] = {
   {      0,   0,  0,  0,  0, 1,  0 },  // 0: nada
   {      0,   0,  3,  2, 11, 1,  8 },  // SFX_AIRE: soplido de ruido
   { 0x0300,  60, 12,  1, 15, 1, 14 },  // SFX_GOLPE: ruido + tono que cae
   { 0x0048,   2,  0,  0, 13, 2,  8 },  // SFX_BLOQUEO: "clac" agudo
   { 0x0180, -16,  0,  0, 11, 2, 10 },  // SFX_SALTO: tono que sube
   { 0x0120,  20, 18,  0, 15, 4, 50 },  // SFX_KO: caída larga
   { 0x0238,   0,  0,  0, 15, 6, 90 },  // SFX_GONG: nota grave que se apaga despacio
   { 0x0070,   0,  0,  0, 12, 1,  6 },  // SFX_MENU: pitido
   { 0x0700,  20, 28,  0, 14, 1, 14 },  // SFX_CAIDA: golpe sordo contra el suelo
};

typedef struct {
   const u8* ini;   // principio de la partitura (para repetir)
   const u8* pos;   // siguiente nota (0 = canal parado)
   u8 dur;          // cuadros que le quedan a la nota actual
   u8 nota;
   u8 vol, vol0;    // volumen actual e inicial
   u8 cuenta;       // contador para bajar el volumen
} TCanal;

static TCanal canal[2];
static u16 sfx_tono;
static u8 sfx_ruido, sfx_vol, sfx_cuenta, sfx_dur;
static const TEfecto* sfx;

volatile u8 g_pide_musica;   // 0xFF = nada pendiente
volatile u8 g_pide_efecto;   // 0 = nada pendiente

// Envía g_psg[0..10] al chip de sonido. El AY no está conectado directamente
// al Z80: se le habla a través del PPI 8255 (puerto A = dato, puerto C = orden).
void psg_vuelca(void) __naked {
   __asm
   ld hl, #_g_psg
   ld e, #0                ; registro del AY
pv_bucle:
   ld b, #0xF4             ; PPI puerto A <- número de registro
   out (c), e
   ld bc, #0xF6C0          ; PPI puerto C <- "seleccionar registro"
   out (c), c
   ld bc, #0xF600          ; PPI puerto C <- "inactivo"
   out (c), c
   ld b, #0xF4             ; PPI puerto A <- valor
   ld a, (hl)
   out (c), a
   ld bc, #0xF680          ; PPI puerto C <- "escribir"
   out (c), c
   ld bc, #0xF600
   out (c), c
   inc hl
   inc e
   ld a, e
   cp #11
   jr nz, pv_bucle
   ret
   __endasm;
}

static void canal_empieza(TCanal* c, const u8* datos, u8 vol) {
   c->ini = c->pos = datos;
   c->dur = 0;
   c->vol = 0;
   c->vol0 = vol;
   c->nota = 0;
}

// Avanza un cuadro en un canal de música. Devuelve 1 si suena percusión.
static u8 canal_tick(TCanal* c, u8 n) {
   u8 v;
   if (!c->pos) {
      g_psg[8 + n] = 0;
      return 0;
   }
   if (!c->dur) {
      v = c->pos[0];
      if (v == MUS_BUCLE) {
         c->pos = c->ini;
         v = c->pos[0];
      }
      if (v == MUS_FIN) {
         c->pos = 0;
         g_psg[8 + n] = 0;
         return 0;
      }
      c->nota = v;
      c->dur = c->pos[1];
      c->pos += 2;
      c->vol = v ? c->vol0 : 0;
      c->cuenta = 0;
   }
   c->dur--;
   if (c->nota == NOTA_PERCUSION) {
      g_psg[8 + n] = c->vol;
      c->vol = (c->vol > 3) ? c->vol - 3 : 0;
      return 1;
   }
   if (c->nota) {
      g_psg[n * 2] = g_periodos[c->nota] & 0xFF;
      g_psg[n * 2 + 1] = g_periodos[c->nota] >> 8;
   }
   g_psg[8 + n] = c->vol;
   // "envolvente" sencilla: el volumen baja 1 cada 3 cuadros hasta la mitad
   if (++c->cuenta == 3) {
      c->cuenta = 0;
      if (c->vol > (c->vol0 >> 1))
         c->vol--;
   }
   return 0;
}

static void sonido_tick(void) {
   u8 mezcla = 0x3F;    // 1 = apagado. Bits: 0-2 tono A,B,C; 3-5 ruido A,B,C
   u8 n;

   if (g_pide_musica != 0xFF) {
      n = g_pide_musica;
      g_pide_musica = 0xFF;
      if (n) {
         canal_empieza(&canal[0], g_canciones[n - 1].canal_a, g_canciones[n - 1].vol_a);
         canal_empieza(&canal[1], g_canciones[n - 1].canal_b, g_canciones[n - 1].vol_b);
      } else {
         canal[0].pos = canal[1].pos = 0;
      }
   }
   if (g_pide_efecto) {
      sfx = &efectos[g_pide_efecto];
      g_pide_efecto = 0;
      sfx_tono = sfx->tono;
      sfx_ruido = sfx->ruido;
      sfx_vol = sfx->vol;
      sfx_dur = sfx->dur;
      sfx_cuenta = 0;
   }

   // Música
   mezcla &= ~0x01;                       // tono A
   if (canal_tick(&canal[1], 1)) {
      mezcla &= ~0x10;                    // ruido B (percusión)
      g_psg[6] = 6;
   } else {
      mezcla &= ~0x02;                    // tono B
   }
   canal_tick(&canal[0], 0);

   // Efecto
   if (sfx_dur) {
      sfx_dur--;
      if (sfx->tono) {
         mezcla &= ~0x04;
         g_psg[4] = sfx_tono & 0xFF;
         g_psg[5] = (sfx_tono >> 8) & 0x0F;
         sfx_tono += sfx->dtono;
      }
      if (sfx->ruido) {
         mezcla &= ~0x20;
         g_psg[6] = sfx_ruido & 0x1F;
         sfx_ruido += sfx->druido;
      }
      g_psg[10] = sfx_vol;
      if (++sfx_cuenta >= sfx->caida) {
         sfx_cuenta = 0;
         if (sfx_vol)
            sfx_vol--;
      }
   } else {
      g_psg[10] = 0;
   }
   g_psg[7] = mezcla;
   psg_vuelca();
}

// Se ejecuta en cada interrupción (300 veces por segundo, 6 por cuadro).
// La que coincide con el retrazado vertical (VSYNC) marca el cuadro.
static u8 interrupciones;
static void interrupcion(void) {
   if (cpct_getVSYNCStatus())
      interrupciones = 0;
   if (interrupciones == 0) {
      g_cuadros++;
      sonido_tick();
   }
   if (++interrupciones == 6)
      interrupciones = 0;
}

void sonido_inicia(void) {
   u8 i;
   for (i = 0; i < 11; i++)
      g_psg[i] = 0;
   g_psg[7] = 0x3F;
   canal[0].pos = canal[1].pos = 0;
   sfx_dur = 0;
   sfx = &efectos[0];
   g_pide_musica = 0xFF;
   g_pide_efecto = 0;
   g_cuadros = 0;
   interrupciones = 1;
   cpct_setInterruptHandler(interrupcion);
}

void musica(u8 cancion) {
   g_pide_musica = cancion;
}

void efecto(u8 n) {
   g_pide_efecto = n;
}
