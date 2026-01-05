// main.c
#include <cpctelera.h>      // Incluye la librería de CPCTelera
#include "defines.h"        // Incluye nuestras definiciones del juego
// --- Estructura para el Jugador (Nave) ---
typedef struct {
   u16 x;          // Posición X (s16 para que pueda ser negativo si se sale de pantalla)
   u16 y;          // Posición Y
   u8  color;      // Color de la nave (no usado aquí, pero útil)
   u8  is_alive;   // 1 = vivo, 0 = muerto (para futuras colisiones)
} Player;

// --- Estructura para las Balas del Jugador ---
typedef struct {
   u16 x;          // Posición X
   u16 y;          // Posición Y
   u8  is_active;  // 1 = activa (en movimiento), 0 = inactiva
} PlayerBullet;

// --- Instancias de nuestros objetos ---
Player player;
PlayerBullet player_bullets[MAX_PLAYER_BULLETS]; // Un array para múltiples balas

// --- Prototipos de funciones (declaraciones) ---
void init_game();
void update_game();
void draw_game();
void input_handler();

// =============================================================================
//                                  MAIN
// =============================================================================
void main(void) {
   init_game();       // Inicializa el juego (nave, pantalla, etc.)

   // Bucle principal del juego (corre a 50 FPS gracias a CPCTelera)
   while (1) {
      cpct_waitVSYNC();   // Espera al refresco de pantalla para sincronizar (50 FPS)

      input_handler();    // Procesa la entrada del teclado/joystick
      update_game();      // Actualiza la lógica del juego (mover nave, balas, enemigos)
      draw_game();        // Dibuja todos los elementos en pantalla
   }
}

// =============================================================================
//                            FUNCIONES DEL JUEGO
// =============================================================================

// Inicializa todos los elementos del juego al principio
void init_game() {
   //cpct_disableInterrupts();         // Deshabilita interrupciones para evitar conflictos
   cpct_disableFirmware ();
   cpct_setVideoMode(GAME_VIDEOMODE); // Establece el modo de vídeo definido
   cpct_setBorder(COLOR_BLACK);  // Borde negro
   //cpct_setPalette(cpct_palettes_mono); // Paleta monocromo (para empezar)

   // Inicializa la nave del jugador
   player.x = PLAYER_START_X;
   player.y = PLAYER_START_Y;
   player.color = COLOR_WHITE;
   player.is_alive = 1;

   // Inicializa todas las balas como inactivas
   for (u8 i = 0; i < MAX_PLAYER_BULLETS; i++) {
      player_bullets[i].is_active = 0;
   }
}

// Maneja las entradas del jugador (teclado/joystick)
void input_handler() {
   cpct_scanKeyboard(); // Escanea el teclado

   // Mover la nave a la izquierda
   if (cpct_isKeyPressed(Key_Q)) { // O KEY_CURSOR_LEFT para joystick
      player.x -= PLAYER_SPEED;
      // Limitar el movimiento para que no se salga de la pantalla por la izquierda
      if (player.x < 0) player.x = 0;
   }

   // Mover la nave a la derecha
   if (cpct_isKeyPressed(Key_P)) { // O KEY_CURSOR_RIGHT
      player.x += PLAYER_SPEED;
      // Limitar el movimiento para que no se salga de la pantalla por la derecha
      // Restamos el ancho de la nave para que no se salga del todo
      if (player.x > (320 - PLAYER_WIDTH)) player.x = (320 - PLAYER_WIDTH);
   }

   // Disparar
   if (cpct_isKeyPressed(Key_Space)) {
      // Buscar una bala inactiva para usarla
      for (u8 i = 0; i < MAX_PLAYER_BULLETS; i++) {
         if (!player_bullets[i].is_active) {
            player_bullets[i].x = player.x + (PLAYER_WIDTH / 2) - (BULLET_WIDTH / 2); // Posición en el centro de la nave
            player_bullets[i].y = player.y - BULLET_HEIGHT;                          // Sale por encima de la nave
            player_bullets[i].is_active = 1;
            break; // Solo disparar una bala por pulsación (por ahora)
         }
      }
   }
}

// Actualiza la lógica de todos los elementos del juego (movimiento, colisiones)
void update_game() {
   // Mover balas del jugador
   for (u8 i = 0; i < MAX_PLAYER_BULLETS; i++) {
      if (player_bullets[i].is_active) {
         player_bullets[i].y -= BULLET_SPEED; // Subir la bala
         // Si la bala sale por la parte superior de la pantalla, desactivarla
         if (player_bullets[i].y <= 0) {
            player_bullets[i].is_active = 0;
         }
      }
   }
   // Aquí iría la lógica de enemigos, colisiones, etc.
}

// Dibuja todos los elementos en pantalla
void draw_game() {
   // BORRAR PANTALLA o solo los sprites anteriores
   // Por ahora, simplemente rellenamos la pantalla de negro para empezar
   cpct_memset( (void*)CPCT_VMEM_START, COLOR_BLACK, 0x4000); // Rellena toda la VRAM con negro

   // Dibuja la nave del jugador (por ahora, un simple rectángulo)
   // Más adelante usaremos cpct_drawSprite
   if (player.is_alive) {
      cpct_drawSolidBox(
         cpct_getScreenPtr(CPCT_VMEM_START, player.x, player.y),
         player.color,
         PLAYER_WIDTH,
         PLAYER_HEIGHT
      );
   }

   // Dibuja las balas del jugador
   for (u8 i = 0; i < MAX_PLAYER_BULLETS; i++) {
      if (player_bullets[i].is_active) {
         cpct_drawSolidBox(
            cpct_getScreenPtr(CPCT_VMEM_START, player_bullets[i].x, player_bullets[i].y),
            BULLET_COLOR,
            BULLET_WIDTH,
            BULLET_HEIGHT
         );
      }
   }
}