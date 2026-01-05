// defines.h
#ifndef DEFINES_H
#define DEFINES_H

// --- Configuración General del Juego ---
#define GAME_VIDEOMODE      0         // Modo de vídeo 1 (320x200, 4 colores)

// --- Jugador (Nave) ---
#define PLAYER_START_X      (320 / 2) // Posición inicial X (centro de la pantalla)
#define PLAYER_START_Y      (180)     // Posición inicial Y (cerca del fondo)
#define PLAYER_SPEED        1         // Velocidad de movimiento del jugador
#define PLAYER_WIDTH        16        // Ancho de la nave (en píxeles)
#define PLAYER_HEIGHT       8         // Alto de la nave (en píxeles)

// --- Balas (Proyectiles del jugador) ---
#define MAX_PLAYER_BULLETS  4         // Máximo número de balas en pantalla
#define BULLET_SPEED        2         // Velocidad de las balas
#define BULLET_WIDTH        2         // Ancho de la bala
#define BULLET_HEIGHT       4         // Alto de la bala
#define BULLET_COLOR        3         // Color de la bala (ej: blanco)

// --- Colores ---
#define COLOR_BLACK         0         // Color negro para borrar o fondo
#define COLOR_WHITE         3         // Color blanco para balas o nave (ejemplo)

#endif // DEFINES_H
