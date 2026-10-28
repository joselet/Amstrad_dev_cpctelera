# Genera el escenario: atardecer con sol, montañas, muralla, pagoda, pino y suelo.
#
# La pantalla (160x200 píxeles de modo 0) se divide en tres zonas:
#   líneas   0- 27  marcador: lo dibuja el programa (negro)
#   líneas  28- 99  cielo: se guarda comprimido (RLE) en g_cielo
#   líneas 100-199  franja de lucha: se guarda EN CRUDO en g_franja (8000 bytes)
# La franja se guarda sin comprimir porque el juego la usa constantemente para
# borrar a los luchadores: copiar bytes de g_franja a la pantalla es lo más rápido.

import math
import os
import random
from comun import PEN, byte_m0, guardar_png, array_c, escribir, DIR_IMG

ANCHO, ALTO = 160, 200
Y_CIELO, Y_FRANJA = 28, 100
Y_SUELO = 152          # empieza el suelo de madera

img = [[PEN['trans']] * ANCHO for _ in range(ALTO)]


def pon(x, y, p):
    if 0 <= x < ANCHO and 0 <= y < ALTO:
        img[y][x] = p


# --- Cielo: degradado con tramado ---------------------------------------------
# Lista de (línea donde empieza, color). Entre dos colores se mezcla con un
# tramado de cuadros (un píxel de cada color) durante unas líneas.
DEGRADADO = [(28, 'azul'), (50, 'magenta'), (72, 'purpura'), (96, 'rojoc'),
             (112, 'naranja'), (136, 'amarillo')]
MEZCLA = 6


def color_cielo(x, y):
    for i in range(len(DEGRADADO) - 1):
        y1, c1 = DEGRADADO[i + 1]
        if y < y1 - MEZCLA:
            return PEN[DEGRADADO[i][1]]
        if y < y1:
            # zona de mezcla: más píxeles del color nuevo según nos acercamos
            k = y - (y1 - MEZCLA)   # 0..MEZCLA-1
            patron = (x + y) % 2 == 0 if k >= 2 else ((x + 2 * y) % 4 == 0 and k >= 0 and (x % 2 == 0))
            if k >= 4:
                patron = not ((x + 2 * y) % 4 == 0 and x % 2 == 0)
            return PEN[c1] if patron else PEN[DEGRADADO[i][1]]
    return PEN[DEGRADADO[-1][1]]


for y in range(Y_CIELO, Y_SUELO):
    for x in range(ANCHO):
        img[y][x] = color_cielo(x, y)

# Estrellas en la parte oscura
rnd = random.Random(7)
for _ in range(22):
    x, y = rnd.randrange(ANCHO), rnd.randrange(Y_CIELO + 2, 62)
    pon(x, y, PEN['blanco'] if rnd.random() < 0.5 else PEN['crema'])

# --- Sol: círculo (en píxeles de modo 0 el radio horizontal es la mitad) -------
SOL_X, SOL_Y, SOL_R = 50, 110, 24
for y in range(SOL_Y - SOL_R, SOL_Y + SOL_R + 1):
    dy = y - SOL_Y
    # franjas horizontales en la mitad de abajo (estilo "puesta de sol retro")
    if dy > 4 and (dy % 6) < (dy // 7):
        continue
    semi = math.sqrt(max(0, SOL_R * SOL_R - dy * dy)) / 2
    for x in range(int(SOL_X - semi), int(SOL_X + semi) + 1):
        borde = (x - SOL_X) ** 2 * 4 + dy * dy > (SOL_R - 4) ** 2
        pon(x, y, PEN['amarillo'] if borde or dy > 8 else PEN['crema'])


# --- Montañas: silueta generada sumando ondas ----------------------------------
def montanas(base, alto, color, semilla, suave):
    r = random.Random(semilla)
    ondas = [(r.uniform(0.02, 0.05) * suave, r.uniform(0, 6.28), r.uniform(0.4, 1.0)) for _ in range(4)]
    for x in range(ANCHO):
        h = sum(a * math.sin(x * f + fase) for f, fase, a in ondas)
        top = int(base - alto * (0.55 + 0.45 * h / 2.2))
        for y in range(top, Y_SUELO):
            pon(x, y, PEN[color])


montanas(136, 22, 'purpura', 3, 1.0)
montanas(142, 16, 'magenta', 11, 2.0)

# --- Muralla del templo (piedra gris con juntas y tejadillo) -------------------
for y in range(138, Y_SUELO):
    for x in range(ANCHO):
        fila = (y - 141) // 4
        if y < 141:
            p = PEN['negro'] if y < 140 or x % 4 else PEN['rojo']        # tejadillo
        elif (y - 141) % 4 == 3 or (x + (fila % 2) * 4) % 8 == 0:
            p = PEN['negro']                                              # juntas
        else:
            p = PEN['gris']
        pon(x, y, p)

# --- Pagoda (silueta negra con ventanas encendidas) -----------------------------
PAG_X = 126                       # centro
pisos = [(138, 9, 13), (124, 8, 11), (111, 7, 9), (99, 6, 7)]   # (base, alto, medio ancho)
for base, alto, semi in pisos:
    for y in range(base - alto, base):
        for x in range(PAG_X - semi + 3, PAG_X + semi - 2):
            pon(x, y, PEN['negro'])
    # ventanas
    for x in range(PAG_X - semi + 5, PAG_X + semi - 4, 3):
        pon(x, base - alto + 3, PEN['amarillo'])
        pon(x, base - alto + 4, PEN['amarillo'])
    # tejado curvado: más ancho y con las puntas levantadas
    for k in range(4):
        y = base - alto - k
        ext = semi + 3 - k * 2
        for x in range(PAG_X - ext, PAG_X + ext + 1):
            pon(x, y, PEN['negro'])
    pon(PAG_X - semi - 4, base - alto - 2, PEN['negro'])
    pon(PAG_X + semi + 4, base - alto - 2, PEN['negro'])
# aguja
for y in range(84, 92):
    pon(PAG_X, y, PEN['negro'])
pon(PAG_X - 1, 89, PEN['negro'])
pon(PAG_X + 1, 89, PEN['negro'])


# --- Pino japonés a la izquierda (tronco y nubes de hojas) ---------------------
def elipse(cx, cy, rx, ry, p, p_borde=None):
    for y in range(cy - ry, cy + ry + 1):
        for x in range(cx - rx, cx + rx + 1):
            d = ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2
            if d <= 1:
                pon(x, y, p_borde if (p_borde is not None and d > 0.55 and y < cy) else p)


tronco = [(14, 152), (13, 140), (11, 128), (13, 116), (18, 106), (22, 96), (21, 88)]
for (x0, y0), (x1, y1) in zip(tronco, tronco[1:]):
    for t in range(12):
        x = round(x0 + (x1 - x0) * t / 12)
        y = round(y0 + (y1 - y0) * t / 12)
        for dx in range(-2, 2):
            pon(x + dx, y, PEN['negro'])
# ramas
for (x0, y0, x1, y1) in [(13, 118, 30, 110), (18, 104, 6, 96), (21, 92, 34, 84)]:
    for t in range(16):
        pon(round(x0 + (x1 - x0) * t / 16), round(y0 + (y1 - y0) * t / 16), PEN['negro'])
for cx, cy, rx, ry in [(32, 108, 9, 5), (6, 94, 8, 4), (34, 82, 10, 5), (20, 86, 7, 4)]:
    elipse(cx, cy, rx, ry, PEN['negro'], PEN['verde'])

# --- Suelo de madera en perspectiva ---------------------------------------------
FUGA_X, FUGA_Y = 80, 60          # punto de fuga
for y in range(Y_SUELO, ALTO):
    prof = (y - FUGA_Y)
    for x in range(ANCHO):
        # tablas: líneas que salen del punto de fuga
        u = (x - FUGA_X) * 2 / prof          # coordenada "real" horizontal
        tabla = math.floor(u * 3.2)
        borde_tabla = abs(u * 3.2 - round(u * 3.2)) < 0.9 / prof * 3.2 * 2
        # juntas horizontales cada vez más separadas (perspectiva)
        v = 400 / prof
        junta = (math.floor(v * 1.5 + (tabla % 2) * 0.5) != math.floor(400 / (prof + 1) * 1.5 + (tabla % 2) * 0.5))
        if borde_tabla or junta:
            p = PEN['negro']
        elif y < Y_SUELO + 10:
            p = PEN['rojo'] if (x + y) % 2 or y < Y_SUELO + 5 else PEN['naranja']
        else:
            p = PEN['naranja'] if (tabla % 3 == 0 and (x + y) % 2 == 0) else PEN['rojo']
        img[y][x] = p
# sombra bajo la muralla
for x in range(ANCHO):
    if x % 2 == 0:
        img[Y_SUELO][x] = PEN['negro']


def rle(datos):
    """Comprime con RLE sencillo:
       byte n < 128:  copiar los n+1 bytes siguientes tal cual
       byte n >= 128: repetir (n-128)+1 veces el byte siguiente"""
    res, i = [], 0
    while i < len(datos):
        j = i
        while j < len(datos) and j - i < 128 and datos[j] == datos[i]:
            j += 1
        if j - i >= 3:
            res += [0x80 + (j - i - 1), datos[i]]
            i = j
            continue
        # tramo literal: hasta que empiece una repetición de 3
        j = i
        while j < len(datos) and j - i < 128:
            if j + 2 < len(datos) and datos[j] == datos[j + 1] == datos[j + 2]:
                break
            j += 1
        res += [j - i - 1] + datos[i:j]
        i = j
    return res


def bytes_lineas(y0, y1):
    out = []
    for y in range(y0, y1):
        for x in range(0, ANCHO, 2):
            out.append(byte_m0(img[y][x], img[y][x + 1]))
    return out


def genera():
    cielo = rle(bytes_lineas(Y_CIELO, Y_FRANJA))
    franja = bytes_lineas(Y_FRANJA, ALTO)
    escribir('fondo.h',
             '#ifndef FONDO_H\n#define FONDO_H\n#include <cpctelera.h>\n\n'
             '#define Y_CIELO   %d   // primera línea del cielo\n'
             '#define Y_FRANJA  %d   // primera línea de la franja de lucha\n\n'
             'extern const u8 g_cielo[%d];    // líneas %d-%d comprimidas con RLE\n'
             'extern const u8 g_franja[%d];  // líneas %d-%d en crudo (80 bytes por línea)\n\n#endif\n'
             % (Y_CIELO, Y_FRANJA, len(cielo), Y_CIELO, Y_FRANJA - 1, len(franja), Y_FRANJA, ALTO - 1))
    escribir('fondo.c', '#include "fondo.h"\n\n' + array_c('g_cielo', cielo) + '\n' + array_c('g_franja', franja))
    guardar_png(os.path.join(DIR_IMG, 'fondo.png'), img, 4, 2)
    print('fondo: cielo %d bytes (RLE, de %d), franja %d bytes' %
          (len(cielo), (Y_FRANJA - Y_CIELO) * 80, len(franja)))
