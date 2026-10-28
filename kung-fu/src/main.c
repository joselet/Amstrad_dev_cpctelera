// KUNG-FU - juego de lucha para Amstrad CPC hecho con CPCtelera
//
// main.c: arranque y pantalla de título. Ver README.md para la explicación.
#include "kungfu.h"

// Colores de las filas del logo: de amarillo a rojo
static const u8 degradado_logo[7] = { 10, 9, 9, 8, 8, 7, 6 };

// Pantalla de título. Devuelve 1 o 2 jugadores.
static u8 titulo(void) {
   u8* p = PANT_A;
   u8 cuenta = 0, visible = 0;
   u16 semilla = 1;

   cpct_setVideoMemoryPage(cpct_pageC0);
   g_oculta = PANT_B;
   borra_pantalla(p);

   // Logo con sombra y degradado
   texto_centrado(p, 13, "KUNG-FU", 6, 2, 4);
   g_degradado = degradado_logo;
   texto_centrado(p, 11, "KUNG-FU", 0, 2, 4);
   g_degradado = 0;
   texto_centrado(p, 44, "LA LEYENDA DEL DOJO", 11, 1, 1);

   // Los dos luchadores frente a frente, sobre una tarima
   caja(p, 8, 116, 64, 2, 6);
   pon_postura(p, P_GUARDIA, 26, 116, 0);
   pon_postura(p, P_GUARDIA, 53, 116, 1);

   texto_centrado(p, 126, "1  UN JUGADOR", 12, 1, 2);
   texto_centrado(p, 142, "2  DOS JUGADORES", 12, 1, 2);
   texto(p, 4, 168, "J1: W A S D + ESPACIO", 9, 1, 1);
   texto(p, 4, 178, "J2: CURSORES + RETURN", 3, 1, 1);
   texto_centrado(p, 190, "O JOYSTICKS 1 Y 2", 11, 1, 1);

   musica(MUS_TITULO);
   while (1) {
      // Mientras se espera, la semilla del azar va cambiando: cada partida será distinta
      semilla++;
      cpct_scanKeyboard_f();
      if (cpct_isKeyPressed(Key_1) || cpct_isKeyPressed(Joy0_Fire1)) break;
      if (cpct_isKeyPressed(Key_2)) {
         cuenta = 0xFF;
         break;
      }
      if (g_cuadros >= 25) {             // parpadeo cada medio segundo
         g_cuadros = 0;
         visible ^= 1;
         if (visible)
            texto_centrado(p, 157, "ELIGE 1 O 2", 14, 1, 1);
         else
            caja(p, 20, 157, 40, 7, 0);
      }
   }
   efecto(SFX_MENU);
   musica(0);
   cpct_srand(semilla);
   return (cuenta == 0xFF) ? 2 : 1;
}

static void arranca(void) {
   cpct_disableFirmware();     // a partir de aquí el firmware no toca nada
   graf_inicia();
   sonido_inicia();
   while (1)
      partida(titulo());
}

void main(void) {
   // La pila estaba al final de la memoria (0xBFFF), justo donde va la pantalla B:
   // se mueve a 0x8000 (crece hacia abajo, hasta 0x7E00)
   cpct_setStackLocation(PILA);
   arranca();
}
