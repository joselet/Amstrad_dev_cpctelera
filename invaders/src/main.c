/*

https://gemini.google.com/u/1/app/93ea20810fd5d0a9?hl=es&pageId=none

*/



//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <cpctelera.h>
#include <stdbool.h>

// Definiciones básicas
#define ALIEN_COUNT 10
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 200

// Estructura para entidades (Nave, Aliens, Balas)
typedef struct {
   u8 x, y;
   u8 w, h;
   bool active;
} Entity;

Entity player = {40, 180, 4, 8, true};
Entity aliens[ALIEN_COUNT];
Entity bullet = {0, 0, 1, 4, false};

// Inicializar alienígenas
void init_aliens() {
   for(u8 i=0; i < ALIEN_COUNT; ++i) {
      aliens[i].x = 10 + (i * 6);
      aliens[i].y = 20;
      aliens[i].active = true;
   }
}

// Dibujado de entidades
void draw_entities() {
   // Dibujar jugador
   cpct_drawSprite(sp_player, cpct_getScreenPtr(CPCT_VMEM_START, player.x, player.y), 4, 8);
   
   // Dibujar aliens activos
   for(u8 i=0; i < ALIEN_COUNT; ++i) {
      if(aliens[i].active) {
         cpct_drawSprite(sp_alien, cpct_getScreenPtr(CPCT_VMEM_START, aliens[i].x, aliens[i].y), 4, 8);
      }
   }
   
   // Dibujar bala
   if(bullet.active) {
      cpct_drawSolidBox(cpct_getScreenPtr(CPCT_VMEM_START, bullet.x, bullet.y), 0xFF, 1, 4);
   }
}

void update_logic() {
   // Leer teclado (O, P para movimiento, SPACE para disparo)
   cpct_scanKeyboard();
   
   if (cpct_isKeyPressed(Key_O) && player.x > 0) player.x--;
   if (cpct_isKeyPressed(Key_P) && player.x < 76) player.x++;
   
   if (cpct_isKeyPressed(Key_Space) && !bullet.active) {
      bullet.x = player.x + 1;
      bullet.y = player.y - 4;
      bullet.active = true;
   }

   // Movimiento de la bala y colisión simple
   if (bullet.active) {
      bullet.y -= 2;
      if (bullet.y < 5) bullet.active = false;

      for(u8 i=0; i < ALIEN_COUNT; ++i) {
         if(aliens[i].active && bullet.x >= aliens[i].x && bullet.x <= aliens[i].x + 4 && bullet.y <= aliens[i].y + 8) {
            aliens[i].active = false;
            bullet.active = false;
         }
      }
   }
}

void main(void) {
   cpct_disableInterrupts();
   cpct_setVideoMode(0); // Modo 0: 160x200, 16 colores
   init_aliens();

   while (1) {
      // 1. Borrar pantalla (o usar doble buffer si quieres evitar parpadeo)
      cpct_clearScreen_fast(0x00);

      // 2. Lógica de juego
      update_logic();

      // 3. Renderizado
      draw_entities();

      // 4. Sincronización con el refresco de pantalla
      cpct_waitVSYNC();
   }
}
