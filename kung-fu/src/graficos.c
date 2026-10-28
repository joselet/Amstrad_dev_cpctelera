// graficos.c - doble búfer, sprites por tramos, fondo y texto
#include "kungfu.h"

const u8* g_spr;
u8*       g_pant;
const u8* g_fon;
u8        g_esp;
u8*       g_oculta;
const u8* g_degradado;

// ---------------------------------------------------------------------------
// dibuja_spr: dibuja el sprite g_spr en g_pant con transparencia.
//
// Formato del sprite (generado por gen/sprites.py, función a_tramos):
//    ancho, alto, y por cada fila: inicio, tramos..., 0
//    cabecera de tramo: 0x80+n = n bytes opacos, 0x40+n = n bytes vacíos,
//                       n = n bytes con transparencia, 0 = fin de fila
//
// - Bytes opacos: se copian con LDIR (6 microsegundos por byte).
// - Bytes con transparencia: se mezclan con el fondo usando la tabla de
//   máscaras (alineada en 0x7D00): (fondo AND mascara[b]) OR b.
// - Huecos: solo se avanza el puntero de pantalla.
// Con g_esp=1 (jugador 2) cada fila se dibuja de derecha a izquierda y cada
// byte pasa antes por la tabla de espejo (0x7C00), que intercambia sus 2
// píxeles y cambia los colores del kimono.
//
// Truco: para saber cuándo acaba un tramo se compara el byte bajo de la
// dirección de destino con un valor que se escribe DENTRO de la instrucción
// "cp #0" (código automodificable: el programa está en RAM).
// ---------------------------------------------------------------------------
void dibuja_spr(void) __naked {
   __asm
   ld hl, (_g_spr)
   ld a, (_g_esp)
   or a
   jp nz, de_empieza

   ;; ===================== NORMAL (izquierda a derecha) ====================
   inc hl                  ; saltar el ancho
   ld a, (hl)
   inc hl
   ld (dn_filas), a
   ld de, (_g_pant)
dn_fila:
   push de                 ; guardar el principio de la línea de pantalla
   ld a, (hl)              ; inicio de la fila
   inc hl
   add a, e                ; de += inicio
   ld e, a
   adc a, d
   sub e
   ld d, a
dn_tramo:
   ld a, (hl)              ; cabecera del tramo
   inc hl
   or a
   jr z, dn_finfila
   jp m, dn_opaco          ; bit 7: opaco
   cp #0x40
   jr nc, dn_hueco         ; bit 6: hueco
   ;; --- con transparencia: a = número de bytes
   ex de, hl               ; hl = pantalla, de = datos
   add a, l
   ld (dn_fin+1), a
   ld b, #0x7D             ; bc apuntará a la tabla de máscaras
dn_mezcla:
   ld a, (de)
   inc de
   ld c, a
   ld a, (bc)              ; máscara de este byte
   and (hl)                ; deja solo el fondo de los píxeles transparentes
   or c                    ; y pone los del sprite
   ld (hl), a
   inc hl
   ld a, l
dn_fin:
   cp #0
   jr nz, dn_mezcla
   ex de, hl
   jr dn_tramo
dn_hueco:
   and #0x3F
   add a, e
   ld e, a
   adc a, d
   sub e
   ld d, a
   jr dn_tramo
dn_opaco:
   and #0x7F
   ld c, a
   ld b, #0
   ldir                    ; copia directa: (de) <- (hl), n veces
   jr dn_tramo
dn_finfila:
   pop de
   ;; siguiente línea de pantalla: +0x800, y si se sale del bloque, +0xC050
   ld a, d
   add a, #0x08
   ld d, a
   and #0x38
   jr nz, dn_ok
   ld a, e
   add a, #0x50
   ld e, a
   ld a, d
   adc a, #0xC0
   ld d, a
dn_ok:
   ld a, (dn_filas)
   dec a
   ld (dn_filas), a
   jr nz, dn_fila
   ret
dn_filas: .db 0

   ;; ================ ESPEJO (derecha a izquierda, jugador 2) ===============
de_empieza:
   ld a, (hl)              ; ancho
   inc hl
   dec a
   ld (de_anc+1), a        ; ancho-1, dentro de "ld a,#0"
   ld a, (hl)
   inc hl
   ld (de_filas), a
   ld de, (_g_pant)
de_fila:
   push de
de_anc:
   ld a, #0                ; ancho-1
   sub (hl)                ; - inicio = byte más a la derecha de la fila
   inc hl
   add a, e
   ld e, a
   adc a, d
   sub e
   ld d, a
de_tramo:
   ld a, (hl)
   inc hl
   or a
   jr z, de_finfila
   jp m, de_opaco
   cp #0x40
   jr nc, de_hueco
   ;; --- con transparencia
   ld c, a
   ld a, e
   sub c
   ld (de_fin1+1), a
de_mezcla:
   ld c, (hl)
   inc hl
   ld b, #0x7C
   ld a, (bc)              ; byte en espejo y con los colores del J2
   ld c, a
   inc b                   ; 0x7D: tabla de máscaras
   ld a, (bc)
   ex de, hl
   and (hl)
   or c
   ld (hl), a
   dec hl
   ex de, hl
   ld a, e
de_fin1:
   cp #0
   jr nz, de_mezcla
   jr de_tramo
de_hueco:
   and #0x3F
   ld c, a
   ld a, e
   sub c
   ld e, a
   jr nc, de_tramo
   dec d
   jr de_tramo
de_opaco:
   and #0x7F
   ld c, a
   ld a, e
   sub c
   ld (de_fin2+1), a
   ld b, #0x7C
de_opa:
   ld c, (hl)
   inc hl
   ld a, (bc)              ; byte en espejo
   ld (de), a
   dec de
   ld a, e
de_fin2:
   cp #0
   jr nz, de_opa
   jr de_tramo
de_finfila:
   pop de
   ld a, d
   add a, #0x08
   ld d, a
   and #0x38
   jr nz, de_ok
   ld a, e
   add a, #0x50
   ld e, a
   ld a, d
   adc a, #0xC0
   ld d, a
de_ok:
   ld a, (de_filas)
   dec a
   ld (de_filas), a
   jp nz, de_fila
   ret
de_filas: .db 0
   __endasm;
}

// ---------------------------------------------------------------------------
// restaura_spr: borra el sprite g_spr copiando encima el fondo original
// (g_fon apunta a la misma esquina dentro de g_franja). De cada fila solo se
// copia desde el primer byte hasta el último que tenía dibujo, con LDIR.
// Para saber cuántos son se recorren las cabeceras de los tramos.
// ---------------------------------------------------------------------------
void restaura_spr(void) __naked {
   __asm
   ld hl, (_g_spr)
   ld a, (hl)
   ld (rs_ancho+1), a
   inc hl
   ld a, (hl)
   ld (rs_filas), a
   inc hl
   ld (rs_src), hl
   ld hl, (_g_pant)
   ld (rs_lin), hl
   ld hl, (_g_fon)
   ld (rs_fon), hl
rs_fila:
   ld hl, (rs_src)
   ld e, (hl)              ; e = inicio
   inc hl
   ld c, #0                ; c = bytes de la fila
rs_tramo:
   ld a, (hl)
   inc hl
   or a
   jr z, rs_fintramos
   ld b, a
   and #0x3F               ; n de huecos y tramos con transparencia
   bit 7, b
   jr nz, rs_datos
   bit 6, b
   jr nz, rs_suma          ; hueco: no hay datos que saltar
rs_datos:
   bit 7, b
   jr z, rs_n
   ld a, b
   and #0x7F
rs_n:
   push af                 ; saltar los n bytes de datos
   add a, l
   ld l, a
   adc a, h
   sub l
   ld h, a
   pop af
rs_suma:
   add a, c
   ld c, a
   jr rs_tramo
rs_fintramos:
   ld (rs_src), hl
   ld a, c
   or a
   jr z, rs_sig            ; fila vacía
   ld a, (_g_esp)
   or a
   jr z, rs_norm
rs_ancho:
   ld a, #0                ; en espejo la fila empieza en ancho-inicio-n
   sub e
   sub c
   ld e, a
rs_norm:
   ld d, #0
   ld b, d
   ld hl, (rs_lin)
   add hl, de
   push hl
   ld hl, (rs_fon)
   add hl, de
   pop de
   ldir
rs_sig:
   ld hl, (rs_fon)
   ld bc, #80
   add hl, bc
   ld (rs_fon), hl
   ld hl, (rs_lin)
   ld a, h
   add a, #0x08
   ld h, a
   and #0x38
   jr nz, rs_ok
   ld a, l
   add a, #0x50
   ld l, a
   ld a, h
   adc a, #0xC0
   ld h, a
rs_ok:
   ld (rs_lin), hl
   ld hl, #rs_filas
   dec (hl)
   jr nz, rs_fila
   ret
rs_src:   .dw 0
rs_lin:   .dw 0
rs_fon:   .dw 0
rs_filas: .db 0
   __endasm;
}

// ---------------------------------------------------------------------------
void graf_inicia(void) {
   cpct_setVideoMode(0);
   cpct_setPalette((u8*)g_paleta, 16);
   cpct_setPALColour(16, 0x14);                 // borde negro
   cpct_memcpy(TABLA_ESPEJO, g_tabla_espejo, 256);
   cpct_memcpy(TABLA_MASCARA, g_tabla_mascara, 256);
   borra_pantalla(PANT_A);
   borra_pantalla(PANT_B);
   cpct_setVideoMemoryPage(cpct_pageC0);
   g_oculta = PANT_B;
   g_degradado = 0;
}

u8 graf_indice(void) {
   return (g_oculta == PANT_A) ? 0 : 1;
}

// Enseña la pantalla oculta. El cambio de página lo aplica el CRTC al empezar
// el siguiente cuadro; se espera a que hayan pasado al menos 2 cuadros desde el
// cambio anterior (25 imágenes por segundo, siempre al mismo ritmo).
void graf_cambia(void) {
   u8 tarde = (g_cuadros >= 2);
   while (g_cuadros < 2);
   g_cuadros = 0;
   if (g_oculta == PANT_A) {
      cpct_setVideoMemoryPage(cpct_pageC0);
      g_oculta = PANT_B;
   } else {
      cpct_setVideoMemoryPage(cpct_page80);
      g_oculta = PANT_A;
   }
   // Si llegamos tarde, el cambio se ha hecho a mitad de cuadro: la pantalla
   // que ahora es "oculta" aún se está viendo hasta el final de este cuadro.
   if (tarde)
      while (g_cuadros == 0);
}

u8 color(u8 pen) {
   return g_pen_izq[pen] | g_pen_der[pen];
}

void borra_pantalla(u8* pant) {
   cpct_memset(pant, 0, 0x4000);
}

// Descomprime el cielo (RLE) desde la línea Y_CIELO hasta y_fin (sin incluir)
void pinta_cielo(u8* pant, u8 y_fin) {
   const u8* s = g_cielo;
   u8 x = 0, y = Y_CIELO;
   u8* p = cpct_getScreenPtr(pant, 0, Y_CIELO);
   u8 n, v, repite;

   while (y < y_fin) {
      n = *s++;
      repite = n & 0x80;
      n = (n & 0x7F) + 1;
      if (repite)
         v = *s++;
      while (n--) {
         *p++ = repite ? v : *s++;
         if (++x == 80) {
            x = 0;
            if (++y >= y_fin)
               return;
            p = cpct_getScreenPtr(pant, 0, y);
         }
      }
   }
}

// Copia la franja de lucha completa (líneas Y_FRANJA a 199)
void pinta_franja(u8* pant) {
   u8 y;
   const u8* s = g_franja;
   for (y = Y_FRANJA; y < 200; y++) {
      cpct_memcpy(cpct_getScreenPtr(pant, 0, y), s, 80);
      s += 80;
   }
}

// Rectángulo de un color (x y ancho en bytes)
void caja(u8* pant, u8 x, u8 y, u8 ancho, u8 alto, u8 pen) {
   if (ancho)
      cpct_drawSolidBox(cpct_getScreenPtr(pant, x, y), color(pen), ancho, alto);
}

// Escribe un texto con la fuente 5x7.
//   x: en píxeles (0-159);  ancho: 1 = cada punto de la letra es 1 píxel,
//   2 = cada punto es 1 byte (letras el doble de anchas; x debe ser par)
//   alto: líneas por cada fila de la letra
// Si g_degradado no es 0, cada fila de la letra usa la tinta g_degradado[fila].
void texto(u8* pant, u8 x, u8 y, const char* s, u8 pen, u8 ancho, u8 alto) {
   u8 fila, rep, col, bits, c, px, b;
   u8* lin;
   u8* q;
   const u8* letra;

   while ((c = *s++)) {
      if (c < 32 || c > 90)
         c = 32;
      letra = g_fuente + (u16)(c - 32) * 7;
      for (fila = 0; fila < 7; fila++) {
         bits = letra[fila];
         if (!bits)
            continue;
         if (g_degradado)
            pen = g_degradado[fila];
         for (rep = 0; rep < alto; rep++) {
            lin = cpct_getScreenPtr(pant, 0, y + fila * alto + rep);
            b = bits;
            for (col = 0; col < 5; col++, b <<= 1) {
               if (!(b & 0x80))
                  continue;
               px = x + col * ancho;
               q = lin + (px >> 1);
               if (ancho == 2)
                  *q = color(pen);
               else if (px & 1)
                  *q = (*q & 0xAA) | g_pen_der[pen];
               else
                  *q = (*q & 0x55) | g_pen_izq[pen];
            }
         }
      }
      x += 6 * ancho;
   }
}

u8 largo(const char* s) {
   u8 n = 0;
   while (*s++)
      n++;
   return n;
}

void texto_centrado(u8* pant, u8 y, const char* s, u8 pen, u8 ancho, u8 alto) {
   u8 w = largo(s) * 6 * ancho - ancho;     // ancho en píxeles
   u8 x = (160 - w) / 2;
   if (ancho == 2)
      x &= 0xFE;
   texto(pant, x, y, s, pen, ancho, alto);
}

// Dibuja una postura suelta (pantalla de título)
void pon_postura(u8* pant, u8 postura, u8 x, u8 y_suelo, u8 espejo) {
   const TPostura* p = &g_posturas[postura];
   u8 w = p->datos[0];
   g_spr = p->datos;
   g_esp = espejo;
   g_pant = cpct_getScreenPtr(pant, espejo ? x - p->dx - w + 1 : x + p->dx, y_suelo + p->oy);
   dibuja_spr();
}
