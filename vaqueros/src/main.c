//------------------------------------------------------------------------------
//  READY, STEADY, GO!  -  Un duelo del oeste para Amstrad CPC (CPCtelera)
//
//  Tras tiempos aleatorios aparecen READY, STEADY y GO. Disparar (ESPACIO)
//  antes del GO es perder. Después del GO gana quien dispare primero.
//  En cada nivel el ordenador reacciona más rápido.
//------------------------------------------------------------------------------

#include <cpctelera.h>
// Ficheros generados por CPCtelera a partir de img/*.png (cfg/image_conversion.mk)
#include "g_paleta.h"
#include "jugq.h"
#include "jugd.h"
#include "jugm.h"
#include "eneq.h"
#include "ened.h"
#include "enem.h"

// Tamaño de los sprites en bytes (modo 1: 1 byte = 4 píxeles) y en líneas
#define SPR_W  G_JUGQ_W
#define SPR_H  G_JUGQ_H

// Posiciones en pantalla. X en bytes (0-79), Y en líneas (0-199)
#define X_JUG    8      // jugador a la izquierda
#define X_ENE   64      // enemigo a la derecha
#define Y_VAQ  100      // altura de los vaqueros
#define Y_SUELO (Y_VAQ + SPR_H)

#define Y_NIVEL   8
#define Y_AVISO  60     // READY / STEADY / GO
#define Y_MSG1  160
#define Y_MSG2  176
#define Y_MSG3  190

// Colores de la paleta (índices 0-3, ver cfg/image_conversion.mk)
#define NEGRO   0
#define NARANJA 1
#define ROJO    2
#define BLANCO  3

//------------------------------------------------------------------------------
// Utilidades
//------------------------------------------------------------------------------

// ¿Está pulsado el espacio ahora mismo?
u8 espacio(void) {
   cpct_scanKeyboard_f();
   return cpct_isKeyPressed(Key_Space);
}

// Espera a que el jugador suelte el espacio (para que una pulsación
// antigua no cuente como disparo)
void soltarEspacio(void) {
   while (espacio());
}

// Espera 'frames' cuadros (50 cuadros = 1 segundo). Devuelve 1 en cuanto
// se pulsa el espacio, o 0 si el tiempo pasa sin pulsarlo.
u8 esperar(u8 frames) {
   while (frames--) {
      cpct_waitVSYNCStart();   // se sincroniza con el inicio de cada cuadro
      if (espacio())
         return 1;
   }
   return 0;
}

// Pausa sin mirar el teclado
void pausa(u8 frames) {
   while (frames--)
      cpct_waitVSYNCStart();
}

// Número aleatorio entre min y max (ambos incluidos)
u8 aleatorio(u8 min, u8 max) {
   return min + cpct_rand() % (max - min + 1);
}

// Escribe n (0-99) en los dos últimos caracteres de la cadena txt
void ponerNumero(char* txt, u8 n) {
   while (*txt) txt++;           // ir al final de la cadena
   txt[-2] = '0' + n / 10;
   txt[-1] = '0' + n % 10;
}

// Borra una línea de texto (8 píxeles de alto) de lado a lado
void borrarLinea(u8 y) {
   // drawSolidBox admite como mucho 64 bytes de ancho: dos cajas de 40
   cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START,  0, y), 0, 40, 8);
   cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START, 40, y), 0, 40, 8);
}

// Escribe un texto centrado en la línea y. Cada letra mide 2 bytes de ancho.
void mensaje(u8 y, char* txt, u8 color) {
   u8 largo = 0;
   while (txt[largo]) largo++;
   borrarLinea(y);
   cpct_setDrawCharM1(color, NEGRO);
   cpct_drawStringM1(txt, cpct_getScreenPtr(CPCT_VMEM_START, 40 - largo, y));
}

void dibujar(u8* sprite, u8 x) {
   cpct_drawSprite(sprite, cpct_getScreenPtr(CPCT_VMEM_START, x, Y_VAQ), SPR_W, SPR_H);
}

// Pantalla en negro, suelo naranja y los dos vaqueros en guardia
void escenario(void) {
   cpct_clearScreen(0);
   cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START,  0, Y_SUELO), cpct_px2byteM1(NARANJA, NARANJA, NARANJA, NARANJA), 40, 2);
   cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START, 40, Y_SUELO), cpct_px2byteM1(NARANJA, NARANJA, NARANJA, NARANJA), 40, 2);
   dibujar(g_jugq, X_JUG);
   dibujar(g_eneq, X_ENE);
}

//------------------------------------------------------------------------------
// Un duelo. Devuelve 1 si gana el jugador, 0 si muere.
//------------------------------------------------------------------------------

// El jugador ha disparado antes del GO: el enemigo le dispara a él
u8 antesDeTiempo(void) {
   dibujar(g_ened, X_ENE);
   dibujar(g_jugm, X_JUG);
   mensaje(Y_MSG1, "HAS DISPARADO ANTES DE TIEMPO!", ROJO);
   return 0;
}

u8 duelo(u8 nivel) {
   u8 reaccion;
   // Texto con hueco para el número de nivel (los dos últimos caracteres).
   // OJO: es local a propósito. En CPCtelera las variables GLOBALES con valor
   // inicial no se inicializan al arrancar; las locales sí (se copian al entrar).
   char txt_nivel[] = "NIVEL 00";

   escenario();
   ponerNumero(txt_nivel, nivel);
   mensaje(Y_NIVEL, txt_nivel, BLANCO);
   soltarEspacio();

   // READY, STEADY y GO, cada uno tras una espera aleatoria (en cuadros)
   if (esperar(aleatorio(50, 125))) return antesDeTiempo();   // 1 - 2,5 s
   mensaje(Y_AVISO, "READY", BLANCO);
   if (esperar(aleatorio(25,  75))) return antesDeTiempo();   // 0,5 - 1,5 s
   mensaje(Y_AVISO, "STEADY", NARANJA);
   if (esperar(aleatorio(25, 100))) return antesDeTiempo();   // 0,5 - 2 s
   mensaje(Y_AVISO, "GO!", ROJO);

   // Tiempo que tarda el ordenador en disparar: en el nivel 1 unos
   // 41 cuadros (0,8 s); cada nivel 4 cuadros menos, hasta un mínimo de 10
   // (0,2 s, como un buen reflejo humano). Más un poco de azar.
   if (nivel < 9) reaccion = 45 - nivel * 4;
   else           reaccion = 10;
   reaccion += aleatorio(0, 8);

   if (esperar(reaccion)) {
      // El jugador ha sido más rápido
      dibujar(g_jugd, X_JUG);
      dibujar(g_enem, X_ENE);
      mensaje(Y_MSG1, "NIVEL SUPERADO!", NARANJA);
      pausa(100);
      return 1;
   }

   // El ordenador ha sido más rápido
   dibujar(g_ened, X_ENE);
   dibujar(g_jugm, X_JUG);
   mensaje(Y_MSG1, "TE HAN DADO!", ROJO);
   return 0;
}

//------------------------------------------------------------------------------
// Programa principal
//------------------------------------------------------------------------------
void main(void) {
   u8  nivel;
   char txt_llegas[] = "HAS LLEGADO AL NIVEL 00";   // local: ver duelo()
   u32 semilla;   // el generador no admite semilla 0: empezamos en 1

   // Apagar el firmware: si no, al cambiar de modo o paleta él la restauraría
   cpct_disableFirmware();
   cpct_setVideoMode(1);
   cpct_setPalette(g_paleta, 4);
   cpct_setBorder(g_paleta[NEGRO]);

   while (1) {
      // Pantalla de título
      escenario();
      mensaje(Y_AVISO - 20, "READY, STEADY, GO!", NARANJA);
      mensaje(Y_AVISO,      "UN DUELO EN EL OESTE", BLANCO);
      mensaje(Y_MSG2,       "PULSA ESPACIO", BLANCO);

      // Mientras esperamos al jugador contamos cuadros: como nunca tarda
      // lo mismo en pulsar, sirve de semilla para los números aleatorios
      soltarEspacio();
      semilla = 1;
      while (!espacio()) semilla++;
      cpct_srand(semilla);

      // Duelos hasta que el jugador muera
      nivel = 1;
      while (duelo(nivel) && nivel < 99)
         nivel++;

      ponerNumero(txt_llegas, nivel);
      mensaje(Y_MSG2, txt_llegas, BLANCO);
      pausa(100);
      mensaje(Y_MSG3, "PULSA ESPACIO", NARANJA);
      soltarEspacio();
      while (!espacio());
   }
}
