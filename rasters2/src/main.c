//------------------------------------------------------------------------------
// rasters2: demo de rasters en MODO 0, 1 y 2
//
//   Teclas 0, 1, 2: elegir modo.  ESPACIO: siguiente modo.
//
// Continuación de rasters/ (léelo primero). Allí cambiábamos el color de la
// pluma 1 en cada línea. Aquí vamos más lejos:
//
//   1) rasterVertical: en cada línea cambiamos DOS plumas, la 0 (fondo) y la 1
//      (letras). Con eso hasta el modo 2, que solo tiene 2 colores, enseña los
//      27 colores del CPC a la vez.
//
//   2) rasterColumnas ("split raster"): cambiamos el color de una pluma 4 VECES
//      DENTRO DE LA MISMA LÍNEA. Así, letras que están una al lado de la otra
//      tienen colores distintos aunque usen la misma pluma.
//
// LOS LÍMITES (el porqué de todo)
// -------------------------------
//   - Una línea dura 64 µs: 40 µs de zona visible + 24 µs de borde y retrazado.
//   - En 1 µs el Gate Array pinta 2 bytes de vídeo:
//        modo 0: 4 píxeles (medio carácter)
//        modo 1: 8 píxeles (1 carácter)
//        modo 2: 16 píxeles (2 caracteres)
//   - Un OUT tarda 4 µs. Como mucho cambiamos un color cada 4 µs: cada 2
//     caracteres en modo 0, cada 4 en modo 1 y cada 8 en modo 2. Carácter a
//     carácter no se puede.
//   - El MODO de vídeo no se puede cambiar a mitad de línea: el Gate Array lo
//     aplica al empezar la siguiente.
//   - El CPC tiene 27 colores: ese es el máximo en pantalla, en cualquier modo.
//   - Mientras dura un raster, la CPU no puede hacer nada más.
//
// SINCRONIZACIÓN (distinta de rasters/)
// -------------------------------------
// En rasters/ el efecto se lanzaba desde la interrupción. Aquí los efectos son
// largos (hasta 120 líneas) y se pasarían de las 52 líneas que hay entre dos
// interrupciones. Por eso los lanzamos desde el bucle principal con las
// interrupciones desactivadas (DI), y usamos HALT solo para sincronizarnos:
//
//   1. cpct_waitVSYNC():           esperamos al VSYNC (línea ~240)
//   2. halt mientras haya VSYNC:   se salta la interrupción que cae dentro del
//                                  VSYNC (que no siempre llega)
//                                  -> salimos siempre en la de la línea ~294
//   3. halt:                       la siguiente llega en la línea ~34, siempre
//                                  en el mismo instante
//   4. DI, efectos, EI
//
// El firmware está desactivado: su manejador de interrupción se queda en un
// simple "EI; RET", que nos basta para despertar del HALT.
//------------------------------------------------------------------------------

#include <cpctelera.h>
#include "pantallas.h"

// Puerto B del PPI (&F5xx): su bit 0 vale 1 mientras dura el VSYNC.
__sfr __banked __at(0xF500) PPI_B;

//------------------------------------------------------------------------------
// Ajustes finos de tiempo (calibrados a ojo en Caprice32)
//------------------------------------------------------------------------------
// Líneas desde la interrupción de la línea ~34 hasta el primer raster
#define ESPERA_V    4
// NOPs extra antes de cada efecto: mueven el punto de cambio en horizontal
// (1 NOP = 1 µs = 8 píxeles de modo 1 más a la derecha)
#define AJUSTE_V    62
#define AJUSTE_C    12

//------------------------------------------------------------------------------
// Parámetros de las rutinas de ensamblador. Son variables globales para que el
// ensamblador las lea directamente (en C se llaman vLineas, en ensamblador
// _vLineas). No tienen valor inicial: CPCtelera no lo respetaría.
//------------------------------------------------------------------------------
u8  vEspera, vLineas, vNormal0, vNormal1;   // rasterVertical
u8* vTabla;
u8  cEspera, cLineas, cPluma, cNormal;      // rasterColumnas
u8* cTabla;
u16 guardaSP;                               // para recuperar la pila

//------------------------------------------------------------------------------
// rasterVertical: plumas 0 y 1 cambian de color en cada línea.
//   Tabla: 2 bytes por línea = { color fondo, color letras } (ya con el &40)
//
// TRUCO DE LA PILA: "pop de" lee 2 bytes de memoria en 3 µs y avanza el
// puntero solo. "ld e,(hl) / inc hl / ld d,(hl) / inc hl" tardaría 6 µs. Para
// usarlo apuntamos SP (la pila) a la tabla. Es seguro porque las interrupciones
// están desactivadas: nadie más usa la pila mientras tanto.
//------------------------------------------------------------------------------
void rasterVertical(void) __naked {
   __asm
      ld    (_guardaSP), sp   ; guardamos la pila de verdad

      ;--- Esperar hasta la primera línea del efecto ---------------------------
      ld    a, (_vEspera)
   00001$:                    ; 64 µs por vuelta = 1 línea
      .rept 60
         nop
      .endm
      dec   a                 ;  1
      jr    nz, 00001$        ;  3   -> 60 + 1 + 3 = 64
      .rept AJUSTE_V          ; ajuste fino en horizontal
         nop
      .endm

      ;--- Preparar registros ----------------------------------------------------
      ld    sp, (_vTabla)     ; SP -> tabla de colores
      ld    bc, #0x7F00       ; B = &7F, puerto del Gate Array
      ld    hl, #0x0001       ; H = &00 "elegir pluma 0", L = &01 "elegir pluma 1"
      ld    a, (_vLineas)     ; A = líneas que faltan

      ;--- Una vuelta por línea: 64 µs exactos -----------------------------------
   00002$:
      pop   de                ;  3   E = color fondo, D = color letras
      out   (c), h            ;  4   elegimos la pluma 0...
      out   (c), e            ;  4   ...y le damos su color
      out   (c), l            ;  4   elegimos la pluma 1...
      out   (c), d            ;  4   ...y le damos su color
      .rept 41
         nop                  ; 41   esperar a la línea siguiente
      .endm
      dec   a                 ;  1
      jr    nz, 00002$        ;  3   -> 3 + 16 + 41 + 1 + 3 = 64

      ;--- Recuperar la pila y los colores normales --------------------------------
      ld    sp, (_guardaSP)
      ld    a, (_vNormal0)
      out   (c), h            ; pluma 0...
      out   (c), a            ; ...a su color normal
      ld    a, (_vNormal1)
      out   (c), l            ; pluma 1...
      out   (c), a            ; ...a su color normal
      ret
   __endasm;
}

//------------------------------------------------------------------------------
// rasterColumnas (split raster): la pluma cPluma cambia 4 veces por línea, una
// cada 10 µs (20 bytes de pantalla = 10 caracteres de modo 1 o 20 de modo 2).
//   Tabla: 4 bytes por línea = { columna 0, columna 1, columna 2, columna 3 }
//
// Aquí los OUT caen en plena zona visible: el cambio se ve justo en el punto
// de la línea por donde pasa el haz en ese momento. Contando los µs de cada
// instrucción decidimos en qué carácter cambia el color.
// (Se llama justo después de rasterVertical: cEspera cuenta desde su final.)
//------------------------------------------------------------------------------
void rasterColumnas(void) __naked {
   __asm
      ld    (_guardaSP), sp

      ld    a, (_cEspera)
   00003$:                    ; 64 µs por vuelta = 1 línea
      .rept 60
         nop
      .endm
      dec   a
      jr    nz, 00003$
      .rept AJUSTE_C
         nop
      .endm

      ld    bc, #0x7F00       ; B = &7F, puerto del Gate Array
      ld    a, (_cPluma)
      out   (c), a            ; elegimos la pluma una sola vez
      ld    sp, (_cTabla)     ; SP -> tabla de colores
      ld    a, (_cLineas)

   00004$:                    ;                    columna en la que cae:
      pop   de                ;  3   E = col. 0, D = col. 1
      pop   hl                ;  3   L = col. 2, H = col. 3
      out   (c), e            ;  4   ---> color de la columna 0 (borde izquierdo)
      .rept 6
         nop                  ;  6
      .endm
      out   (c), d            ;  4   ---> columna 1 (10 µs después)
      .rept 6
         nop                  ;  6
      .endm
      out   (c), l            ;  4   ---> columna 2
      .rept 6
         nop                  ;  6
      .endm
      out   (c), h            ;  4   ---> columna 3
      .rept 20
         nop                  ; 20
      .endm
      dec   a                 ;  1
      jr    nz, 00004$        ;  3   -> 6 + 4+6 + 4+6 + 4+6 + 4+20 + 1+3 = 64

      ld    sp, (_guardaSP)
      ld    a, (_cNormal)
      out   (c), a            ; la pluma vuelve a su color normal
      ret
   __endasm;
}

//------------------------------------------------------------------------------
// Tablas de colores
//------------------------------------------------------------------------------
#define C(color)  (0x40 | (color))
#define PERIODO   32            // la animación se repite cada 32 líneas

// 16 colores brillantes (letras) y 16 oscuros (fondos)
const u8 brillantes[16] = {
   HW_BRIGHT_RED,   HW_ORANGE,      HW_BRIGHT_YELLOW,  HW_PASTEL_YELLOW,
   HW_LIME,         HW_BRIGHT_GREEN,HW_PASTEL_GREEN,   HW_BRIGHT_CYAN,
   HW_PASTEL_CYAN,  HW_SKY_BLUE,    HW_PASTEL_BLUE,    HW_BRIGHT_WHITE,
   HW_PASTEL_MAGENTA,HW_BRIGHT_MAGENTA,HW_PINK,        HW_YELLOW
};
const u8 oscuros[16] = {
   HW_BLACK,  HW_BLUE,    HW_PURPLE, HW_MAGENTA, HW_RED,   HW_MAGENTA, HW_PURPLE, HW_BLUE,
   HW_BLACK,  HW_GREEN,   HW_CYAN,   HW_GREEN,   HW_BLACK, HW_BLUE,    HW_MAUVE,  HW_BLUE
};

// Las tablas tienen (líneas + PERIODO) filas. Para animar, cada imagen
// empezamos a leer una fila más abajo (fase 0..31): los colores "suben".
#define MAX_V   120
#define MAX_C   40
u8 tablaV[(MAX_V + PERIODO) * 2];
u8 tablaC[(MAX_C + PERIODO) * 4];

void construyeTablas(void) {
   u8 f, col;
   u8* p;

   // Vertical: el fondo cambia cada 2 líneas (degradado suave de 32 líneas);
   // las letras, en cada línea.
   p = tablaV;
   for (f = 0; f < MAX_V + PERIODO; ++f) {
      *p++ = C(oscuros[(f >> 1) & 15]);
      *p++ = C(brillantes[f & 15]);
   }
   // Columnas: un "plasma" en diagonal (cada columna, 4 colores más adelante)
   p = tablaC;
   for (f = 0; f < MAX_C + PERIODO; ++f)
      for (col = 0; col < 4; ++col)
         *p++ = C(brillantes[((f >> 1) + col * 4) & 15]);
}

//------------------------------------------------------------------------------
// Configuración de cada modo
//------------------------------------------------------------------------------
const u8 paleta0[16] = {
   HW_BLACK,  HW_BRIGHT_WHITE, HW_BRIGHT_YELLOW, HW_BRIGHT_CYAN,
   HW_BRIGHT_RED, HW_BRIGHT_GREEN, HW_BRIGHT_BLUE, HW_BRIGHT_MAGENTA,
   HW_ORANGE, HW_PINK, HW_SKY_BLUE, HW_LIME,
   HW_RED, HW_GREEN, HW_BLUE, HW_MAGENTA
};
const u8 paleta1[4] = { HW_BLACK, HW_BRIGHT_WHITE, HW_BRIGHT_YELLOW, HW_BRIGHT_CYAN };
const u8 paleta2[2] = { HW_BLACK, HW_BRIGHT_WHITE };

u8 modo;

void cambiaModo(u8 m) {
   const u8* pal;
   u8 n;

   modo = m;
   for (n = 0; n < 16; ++n)           // todo negro mientras dibujamos...
      cpct_setPALColour(n, HW_BLACK);
   cpct_setVideoMode(m);
   cpct_clearScreen(0);

   // Zona vertical: siempre empieza en la línea 40
   vEspera = ESPERA_V;
   if (m == 0) {
      pal = paleta0;  n = 16;
      vLineas = 80;                    // líneas 40-119
      cPluma = 1;                      // (sin columnas en modo 0)
      dibujaModo0();
   } else if (m == 1) {
      pal = paleta1;  n = 4;
      vLineas = 40;                    // líneas 40-79
      cEspera = 16;  cLineas = 40;     // líneas 96-135
      cPluma = 2;
      dibujaModo1();
   } else {
      pal = paleta2;  n = 2;
      vLineas = 120;                   // líneas 40-159
      cEspera = 8;   cLineas = 24;     // líneas 168-191
      cPluma = 1;
      dibujaModo2();
   }
   vNormal0 = C(pal[0]);
   vNormal1 = C(pal[1]);
   cNormal  = C(pal[cPluma]);
   cpct_setPalette((u8*)pal, n);      // ...y ahora los colores de verdad
}

//------------------------------------------------------------------------------
// Los efectos de una imagen. Siempre el mismo camino de código, para que el
// tiempo entre los dos rasters sea siempre igual.
//------------------------------------------------------------------------------
void efectos(void) {
   __asm
      di
   __endasm;
   rasterVertical();
   if (modo != 0)
      rasterColumnas();
   __asm
      ei
   __endasm;
}

void main(void) {
   u8 fase = 0, imagen = 0, espacioAntes = 0, espacio, m;

   cpct_disableFirmware();
   cpct_setBorder(HW_BLACK);
   construyeTablas();
   vTabla = tablaV;
   cTabla = tablaC;
   cambiaModo(1);

   while (1) {
      // --- Sincronización (ver cabecera) ---
      cpct_waitVSYNC();
      do {
         __asm
            halt
         __endasm;
      } while (PPI_B & 1);
      __asm
         halt
      __endasm;

      // --- Línea ~34: los rasters ---
      efectos();

      // --- Resto de la imagen: animación y teclado ---
      ++imagen;
      fase = (imagen >> 1) & (PERIODO - 1);   // avanza una línea cada 2 imágenes
      vTabla = tablaV + fase * 2;
      cTabla = tablaC + fase * 4;

      cpct_scanKeyboard_f();
      m = modo;
      if (cpct_isKeyPressed(Key_0)) m = 0;
      if (cpct_isKeyPressed(Key_1)) m = 1;
      if (cpct_isKeyPressed(Key_2)) m = 2;
      espacio = cpct_isKeyPressed(Key_Space) ? 1 : 0;
      if (espacio && !espacioAntes)            // solo al pulsar, no al mantener
         m = (modo == 2) ? 0 : modo + 1;
      espacioAntes = espacio;
      if (m != modo)
         cambiaModo(m);
   }
}
