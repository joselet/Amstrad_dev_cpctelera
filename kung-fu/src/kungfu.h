// kungfu.h - definiciones comunes a todos los módulos del juego
#ifndef KUNGFU_H
#define KUNGFU_H

#include <cpctelera.h>
#include "datos/sprites.h"
#include "datos/fondo.h"
#include "datos/tablas.h"
#include "datos/musica.h"

// ---------------------------------------------------------------------------
// Mapa de memoria (ver README):
//   0x0100 - ...     programa y datos
//   0x7C00 - 0x7CFF  tabla de espejo del jugador 2  (alineada a 256)
//   0x7D00 - 0x7DFF  tabla de máscaras de transparencia (alineada a 256)
//   0x7E00 - 0x7FFF  pila
//   0x8000 - 0xBFFF  pantalla B
//   0xC000 - 0xFFFF  pantalla A
// ---------------------------------------------------------------------------
#define PANT_A        ((u8*)0xC000)
#define PANT_B        ((u8*)0x8000)
#define TABLA_ESPEJO  ((u8*)0x7C00)
#define TABLA_MASCARA ((u8*)0x7D00)
#define PILA          ((void*)0x8000)

// --- Gráficos (graficos.c) -------------------------------------------------
// Parámetros de las rutinas en ensamblador (se pasan en variables globales)
extern const u8* g_spr;   // datos del sprite (formato por tramos)
extern u8*       g_pant;  // dirección de pantalla de la esquina superior izquierda
extern const u8* g_fon;   // misma esquina, pero dentro de g_franja
extern u8        g_esp;   // 1 = dibujar en espejo y con los colores del jugador 2
void dibuja_spr(void);
void restaura_spr(void);

extern u8* g_oculta;               // pantalla donde se dibuja (la que no se ve)
extern const u8* g_degradado;      // si no es 0, colores por fila para texto()
void graf_inicia(void);
void graf_cambia(void);            // muestra la pantalla oculta y espera al cuadro
u8   graf_indice(void);            // 0 si la oculta es la A, 1 si es la B
u8   color(u8 pen);                // byte con los 2 píxeles de la tinta pen
void borra_pantalla(u8* pant);
void pinta_cielo(u8* pant, u8 y_fin);
void pinta_franja(u8* pant);
void caja(u8* pant, u8 x, u8 y, u8 ancho, u8 alto, u8 pen);
void texto(u8* pant, u8 x, u8 y, const char* s, u8 pen, u8 ancho, u8 alto);
u8   largo(const char* s);
void texto_centrado(u8* pant, u8 y, const char* s, u8 pen, u8 ancho, u8 alto);
void pon_postura(u8* pant, u8 postura, u8 x, u8 y_suelo, u8 espejo);

// --- Sonido (sonido.c) -----------------------------------------------------
#define SFX_AIRE    1
#define SFX_GOLPE   2
#define SFX_BLOQUEO 3
#define SFX_SALTO   4
#define SFX_KO      5
#define SFX_GONG    6
#define SFX_MENU    7
#define SFX_CAIDA   8
extern volatile u8 g_cuadros;      // cuadros (1/50 s) desde la última vez que se puso a 0
void sonido_inicia(void);
void musica(u8 cancion);           // 0 = silencio
void efecto(u8 sfx);

// --- Luchadores (luchador.c) -----------------------------------------------
#define SUELO     190     // primera línea por debajo de los pies
#define X_MIN     7       // límites del ancla (en bytes)
#define X_MAX     72
#define DIST_MIN  9       // distancia mínima entre los dos luchadores
#define VIDA_MAX  64

// Bits de control (los mismos para teclado, joystick y CPU)
#define C_ARR 0x01
#define C_ABA 0x02
#define C_IZQ 0x04
#define C_DER 0x08
#define C_FUE 0x10

// Estados. Desde E_GOLPEADO en adelante el luchador no puede recibir golpes.
#define E_ESPERA    0
#define E_NORMAL    1
#define E_AGACHADO  2
#define E_SALTO     3
#define E_ATAQUE    4
#define E_GOLPEADO  5
#define E_CAIDO     6
#define E_KO        7
#define E_VICTORIA  8

// Ataques
#define A_PUNYO     0
#define A_PATADA    1
#define A_ALTA      2
#define A_BARRIDO   3
#define A_VOLADORA  4
#define A_NINGUNO   0xFF

#define ALT_ALTO  0
#define ALT_MEDIO 1
#define ALT_BAJO  2

typedef struct {
   u8 p_prep, t_prep;     // postura y ticks de preparación
   u8 p_golpe, t_golpe;   // postura y ticks en los que el golpe hace daño
   u8 t_recup;            // ticks para volver a la guardia
   u8 altura;             // ALT_ALTO, ALT_MEDIO o ALT_BAJO
   u8 danyo;
} TAtaque;
extern const TAtaque g_ataques[5];

typedef struct {
   u8 x;            // ancla horizontal (byte 0-79)
   u8 alt;          // altura sobre el suelo (salto)
   u8 estado, t;    // estado y ticks que lleva en él
   u8 ataque;       // ataque en curso o A_NINGUNO
   u8 golpeo;       // 1 si el ataque en curso ya ha golpeado (o fallado)
   u8 postura;      // postura que se dibuja
   i8 vx;           // avance por tick durante el salto
   u8 vida;
   u8 rondas;       // asaltos ganados
   u8 entrada, entrada_ant;
   u8 espejo;       // 0 = mira a la derecha (J1), 1 = a la izquierda (J2)
   u8 paso;         // contador para la animación de andar
} TLuchador;

extern TLuchador g_luch[2];
extern u8 g_chispa_t, g_chispa_x, g_chispa_y;   // chispa del último impacto

void luchador_inicia(TLuchador* l, u8 x, u8 espejo);
void luchador_actualiza(TLuchador* l);
void luchadores_golpes(void);
void luchadores_separa(void);
u8   bloqueando(TLuchador* l);

// --- CPU (cpu.c) -----------------------------------------------------------
void cpu_reinicia(void);
u8   cpu_entrada(TLuchador* yo, TLuchador* riv, u8 nivel);

// --- Partida (juego.c) -----------------------------------------------------
void partida(u8 jugadores);

#endif
