# Genera los sprites de los luchadores a partir de un "esqueleto".
#
# En vez de dibujar cada postura píxel a píxel, cada postura se describe con
# unos pocos ángulos (inclinación del tronco, cadera->rodilla, rodilla, hombro,
# codo...). El programa calcula dónde cae cada articulación y pinta el cuerpo
# con "cápsulas" (segmentos gruesos) y círculos, en el orden correcto para que
# el brazo y la pierna de delante tapen a los de detrás.
#
# Unidades: 1 unidad = 1 línea de pantalla. En modo 0 un píxel es el doble de
# ancho que de alto, así que cada píxel mide 2 unidades de ancho.
# El luchador mira a la derecha (+x) y el eje y va hacia arriba desde el suelo.

import math
from comun import PEN, CAMBIO_J2, byte_m0, pixeles_m0, guardar_png, array_c, escribir, DIR_IMG
import os

# Medidas del cuerpo (en unidades)
MUSLO, PIERNA = 11.0, 11.5
BRAZO, ANTEBRAZO = 8.0, 7.5
TRONCO, CUELLO, R_CABEZA = 15.0, 1.5, 4.3

# Ventana de dibujo: x de -28 a +28 píxeles, y de -4 a 68 unidades
PX_MIN, PX_MAX = -28, 28
Y_MAX = 68          # fila 0 = y 67.5
FILAS = 72          # hasta y = -4


def rad(g):
    return math.radians(g)


def dir_abajo(angulo):
    """Vector de un miembro: ángulo 0 = hacia abajo, positivo = hacia delante."""
    return (math.sin(rad(angulo)), -math.cos(rad(angulo)))


def mas(p, d, l):
    return (p[0] + d[0] * l, p[1] + d[1] * l)


class Lienzo:
    """Rejilla de muestras: 2 muestras por píxel del modo 0 (izquierda y derecha)."""

    def __init__(self):
        self.anch = (PX_MAX - PX_MIN) * 2
        self.m = [[None] * self.anch for _ in range(FILAS)]
        self.z = 0

    def _muestras(self, xmin, xmax, ymin, ymax):
        c0 = max(0, int((xmin - PX_MIN * 2)) - 1)
        c1 = min(self.anch - 1, int((xmax - PX_MIN * 2)) + 1)
        f0 = max(0, int(Y_MAX - ymax) - 1)
        f1 = min(FILAS - 1, int(Y_MAX - ymin) + 1)
        for f in range(f0, f1 + 1):
            y = Y_MAX - f - 0.5
            for c in range(c0, c1 + 1):
                x = PX_MIN * 2 + c + 0.5
                yield f, c, x, y

    def capsula(self, a, b, r, color):
        self.z += 1
        ax, ay = a
        bx, by = b
        dx, dy = bx - ax, by - ay
        l2 = dx * dx + dy * dy or 1e-9
        for f, c, x, y in self._muestras(min(ax, bx) - r, max(ax, bx) + r,
                                         min(ay, by) - r, max(ay, by) + r):
            t = max(0.0, min(1.0, ((x - ax) * dx + (y - ay) * dy) / l2))
            px, py = ax + t * dx, ay + t * dy
            if (x - px) ** 2 + (y - py) ** 2 <= r * r:
                self.m[f][c] = (self.z, color)

    def circulo(self, cen, r, color_fn):
        """color_fn(rx, ry) devuelve la tinta según la posición relativa al centro."""
        self.z += 1
        for f, c, x, y in self._muestras(cen[0] - r, cen[0] + r, cen[1] - r, cen[1] + r):
            rx, ry = x - cen[0], y - cen[1]
            if rx * rx + ry * ry <= r * r:
                col = color_fn(rx, ry)
                if col is not None:
                    self.m[f][c] = (self.z, col)

    def pixeles(self):
        """Cada píxel toma el color de la muestra pintada más tarde (la de delante)."""
        res = []
        for fila in self.m:
            linea = []
            for c in range(0, self.anch, 2):
                a, b = fila[c], fila[c + 1]
                if a is None and b is None:
                    linea.append(None)
                elif a is None or (b is not None and b[0] > a[0]):
                    linea.append(b[1])
                else:
                    linea.append(a[1])
            res.append(linea)
        return res


def pierna(lz, cadera, muslo, rodilla, color, pie='plano'):
    d1 = dir_abajo(muslo)
    k = mas(cadera, d1, MUSLO)
    d2 = dir_abajo(muslo - rodilla)
    tob = mas(k, d2, PIERNA)
    lz.capsula(cadera, k, 3.7, color)
    lz.capsula(k, tob, 2.8, color)
    if pie == 'plano':
        lz.capsula(tob, (tob[0] + 3.8, tob[1] - 0.6), 1.5, PEN['naranja'])
    else:  # punta: el pie sigue la dirección de la pierna
        lz.capsula(tob, mas(tob, d2, 3.5), 1.4, PEN['naranja'])
    return tob


def brazo(lz, hombro, angulo, codo, color):
    d1 = dir_abajo(angulo)
    e = mas(hombro, d1, BRAZO)
    d2 = dir_abajo(angulo + codo)
    mano = mas(e, d2, ANTEBRAZO)
    medio = mas(e, d2, ANTEBRAZO * 0.45)
    lz.capsula(hombro, e, 2.6, color)
    lz.capsula(e, medio, 2.4, color)               # manga del kimono
    lz.capsula(medio, mano, 1.8, PEN['naranja'])   # antebrazo
    lz.circulo(mano, 2.2, lambda rx, ry: PEN['naranja'] if rx > -0.6 or ry > 0.8 else PEN['rojo'])


def cabeza(lz, cen, inclinacion):
    """Cabeza de perfil: cara, pelo negro, cinta roja con lazo y un ojo."""
    s, c = math.sin(rad(inclinacion)), math.cos(rad(inclinacion))

    def color(rx, ry):
        # pasar a coordenadas de la cabeza (girada con el tronco)
        hx = rx * c - ry * s
        hy = rx * s + ry * c
        if 0.5 <= hy <= 1.7:
            return PEN['rojoc']                          # cinta
        if hy > 1.7 or (hx < -1.2 and hy > -2.6):
            return PEN['negro']                          # pelo
        if 1.2 < hx < 2.8 and -0.7 < hy < 0.5:
            return PEN['negro']                          # ojo
        if hy < -2.6 and hx < 0.6:
            return PEN['rojo']                           # sombra de la barbilla
        return PEN['naranja']
    lz.circulo(cen, R_CABEZA, color)
    # lazo de la cinta, colgando por detrás
    nudo = (cen[0] - R_CABEZA * c + 0.8 * s, cen[1] + R_CABEZA * s + 0.8 * c)
    lz.capsula(nudo, (nudo[0] - 3.0, nudo[1] - 3.5), 0.9, PEN['rojoc'])


def dibuja_postura(p):
    """Pinta una postura y devuelve la rejilla de píxeles."""
    lz = Lienzo()
    cad = p.get('cadera', (0.0, 22.0))
    t = p.get('tronco', 0)
    dt = (math.sin(rad(t)), math.cos(rad(t)))
    cuello = mas(cad, dt, TRONCO)
    hombro = mas(cad, dt, TRONCO - 2.5)
    cen_cab = mas(cuello, dt, CUELLO + R_CABEZA - 0.5)
    blanco, gris = PEN['blanco'], PEN['gris']

    # Orden de pintado: lo de atrás primero
    brazo(lz, hombro, *p['bt'], gris)
    pierna(lz, cad, *p['pt'], gris, pie=p.get('pie_t', 'plano'))
    lz.capsula(cad, mas(cad, dt, TRONCO - 1.5), 5.2, blanco)            # tronco
    lz.capsula(mas(cad, dt, 1.2), mas(cad, dt, 2.6), 5.4, PEN['negro'])  # cinturón
    # solapa del kimono (la "V" del cuello)
    lz.capsula(mas(cuello, dt, -1.0), mas(mas(cad, dt, 8.0), (dt[1], -dt[0]), 3.2), 0.6, gris)
    pierna(lz, cad, *p['pd'], blanco, pie=p.get('pie_d', 'plano'))
    lz.capsula(mas(cuello, dt, -1.0), cuello, 1.8, PEN['naranja'])      # cuello
    cabeza(lz, cen_cab, t + p.get('cabeza', 0))
    brazo(lz, hombro, *p['bd'], blanco)
    return lz.pixeles()


def contorno(px):
    """Añade un contorno negro alrededor de la figura (mejor contraste con el fondo)."""
    alto, ancho = len(px), len(px[0])
    res = [fila[:] for fila in px]
    for f in range(alto):
        for c in range(ancho):
            if px[f][c] is not None:
                continue
            for df, dc in ((0, 1), (0, -1), (1, 0), (-1, 0)):
                ff, cc = f + df, c + dc
                if 0 <= ff < alto and 0 <= cc < ancho and px[ff][cc] is not None \
                        and px[ff][cc] != PEN['negro']:
                    res[f][c] = PEN['negro']
                    break
    return res


# --- Las posturas -------------------------------------------------------------
# pd/pt = pierna delantera/trasera: (ángulo del muslo, flexión de rodilla)
# bd/bt = brazo delantero/trasero:  (ángulo del hombro, flexión del codo)
# Ángulos en grados: 0 = hacia abajo, positivo = hacia delante.
GUARDIA_BRAZOS = {'bd': (55, 75), 'bt': (25, 115)}

POSTURAS = [
    ('guardia',   dict(tronco=8, pd=(28, 30), pt=(-22, 12), **GUARDIA_BRAZOS)),
    ('andar1',    dict(tronco=8, pd=(12, 18), pt=(-8, 22), **GUARDIA_BRAZOS)),
    ('andar2',    dict(tronco=8, pd=(36, 22), pt=(-34, 6), **GUARDIA_BRAZOS)),
    ('punyo',     dict(tronco=14, pd=(34, 26), pt=(-30, 4), bd=(92, 0), bt=(-25, 105))),
    ('prepara',   dict(tronco=-4, pd=(88, 115), pt=(-3, 6), **GUARDIA_BRAZOS)),
    ('patada',    dict(tronco=-22, pd=(94, 0), pt=(-8, 8), pie_d='punta',
                       bd=(45, 85), bt=(-20, 70))),
    ('pat_alta',  dict(tronco=-36, pd=(138, 0), pt=(-4, 4), pie_d='punta',
                       bd=(25, 70), bt=(-40, 40))),
    ('agachado',  dict(tronco=22, pd=(62, 122), pt=(0, 90), bd=(60, 85), bt=(35, 110))),
    ('barrido',   dict(tronco=38, pd=(74, 0), pt=(-8, 82), pie_d='punta',
                       bd=(12, 10), bt=(70, 90))),
    ('salto',     dict(tronco=5, cadera=(0.0, 24.0), pd=(82, 105), pt=(35, 115),
                       bd=(70, 80), bt=(40, 110), aire=True)),
    ('voladora',  dict(tronco=-12, cadera=(0.0, 24.0), pd=(72, 0), pt=(25, 125),
                       pie_d='punta', bd=(60, 60), bt=(-25, 80), aire=True)),
    ('bloqueo',   dict(tronco=-4, pd=(20, 22), pt=(-26, 10), bd=(62, 112), bt=(40, 125))),
    ('bloq_aga',  dict(tronco=18, pd=(62, 122), pt=(0, 90), bd=(48, 115), bt=(30, 125))),
    ('golpeado',  dict(tronco=-26, cabeza=-15, pd=(24, 10), pt=(-32, 26),
                       bd=(-15, 40), bt=(-45, 30))),
    ('cayendo',   dict(tronco=-58, cabeza=-10, cadera=(0.0, 14.0), pd=(52, 12), pt=(30, 40),
                       bd=(120, 20), bt=(150, 10))),
    ('caido',     dict(tronco=-90, cabeza=5, pd=(88, 4), pt=(76, 22),
                       bd=(160, 0), bt=(130, 30))),
    ('victoria',  dict(tronco=0, pd=(10, 6), pt=(-10, 6), bd=(155, 15), bt=(-18, 100))),
]


def recortar(px, aire):
    """Recorta la figura al rectángulo mínimo alineado a bytes (2 píxeles).
    Devuelve (filas de bytes, dx, oy, frente):
      dx     = byte de la izquierda relativo al ancla (el byte del centro del luchador)
      oy     = fila de arriba relativa a la línea del suelo (negativo)
      frente = hasta dónde llega por delante (en bytes), para saber el alcance del golpe"""
    filas_usadas = [f for f, fila in enumerate(px) if any(p is not None for p in fila)]
    cols = [c for fila in px for c, p in enumerate(fila) if p is not None]
    f0, f1 = filas_usadas[0], filas_usadas[-1]
    c0 = min(cols) + PX_MIN
    c1 = max(cols) + PX_MIN
    b0, b1 = c0 >> 1, c1 >> 1          # división hacia abajo también con negativos
    # Fila del suelo: la que corresponde a y=0 si está en el aire; si no, justo debajo de los pies
    suelo = Y_MAX if aire else f1 + 1
    filas = []
    for f in range(f0, f1 + 1):
        linea = []
        for b in range(b0, b1 + 1):
            i = b * 2 - PX_MIN
            pi = px[f][i] if 0 <= i < len(px[f]) else None
            pd = px[f][i + 1] if 0 <= i + 1 < len(px[f]) else None
            linea.append((pi, pd))
        filas.append(linea)
    return filas, b0, f0 - suelo, b1


def a_tramos(filas):
    """Convierte las filas a nuestro formato "por tramos":
       ancho, alto, y por cada fila:
         inicio   primer byte con dibujo
         tramos   cada uno empieza con una cabecera:
                    0x80+n  n bytes OPACOS (se copian tal cual con LDIR, muy rápido)
                    0x40+n  n bytes vacíos (se saltan)
                    n       n bytes con algún píxel transparente (usan la máscara)
                    0       fin de la fila
    Los tramos opacos o vacíos muy cortos se tratan como "con transparencia":
    no compensa gastar una cabecera (memoria y tiempo) para 1 o 2 bytes."""
    def tipo(v):
        izq, der = pixeles_m0(v)
        return 'vacio' if v == 0 else 'opaco' if izq and der else 'mezcla'

    def agrupa(tipos):
        grupos = []
        for t in tipos:
            if grupos and grupos[-1][0] == t:
                grupos[-1][1] += 1
            else:
                grupos.append([t, 1])
        return grupos

    datos = [len(filas[0]), len(filas)]
    for linea in filas:
        bs = [byte_m0(a or 0, b or 0) for a, b in linea]
        usados = [i for i, v in enumerate(bs) if v]
        if not usados:
            datos += [0, 0]
            continue
        i0, i1 = usados[0], usados[-1]
        bs = bs[i0:i1 + 1]
        tipos = []
        for t, n in agrupa([tipo(v) for v in bs]):
            tipos += [t if n >= 3 else 'mezcla'] * n
        tramos = []
        i = 0
        for t, n in agrupa(tipos):
            while n:
                k = min(n, 127 if t == 'opaco' else 63)
                if t == 'vacio':
                    tramos += [0x40 + k]
                elif t == 'opaco':
                    tramos += [0x80 + k] + bs[i:i + k]
                else:
                    tramos += [k] + bs[i:i + k]
                i += k
                n -= k
        datos += [i0] + tramos + [0]
    return datos


# Chispa del impacto y polvo (dibujo directo; cada carácter es un píxel)
CHISPAS = [
    ('chispa1', """
.....9.....
.9...9...9.
..9..a..9..
...9aaa9...
99aaacaaa99
...9aaa9...
..9..a..9..
.9...9...9.
.....9.....
"""),
    ('chispa2', """
..9.......9..
...9.....9...
....9.a.9....
.....aca.....
9..aacccaa..9
.....aca.....
....9.a.9....
...9.....9...
..9.......9..
"""),
]
LEYENDA_CHISPA = {'.': None, '9': PEN['amarillo'], 'a': PEN['blanco'], 'c': PEN['crema']}


def genera():
    nombres, bloques, tabla = [], [], []
    hoja = []
    total = 0
    for nombre, p in POSTURAS:
        px = contorno(dibuja_postura(p))
        filas, dx, oy, frente = recortar(px, p.get('aire', False))
        datos = a_tramos(filas)
        total += len(datos)
        ident = 'g_spr_' + nombre
        bloques.append(array_c(ident, datos))
        tabla.append('   { %s, %d, %d, %d },  // %s: %dx%d bytes' %
                     (ident, dx, oy, frente, nombre, len(filas[0]), len(filas)))
        nombres.append(nombre)
        hoja.append((filas, dx, oy))

    for nombre, dib in CHISPAS:
        lineas = dib.strip('\n').split('\n')
        filas = []
        for l in lineas:
            if len(l) % 2:
                l += '.'
            filas.append([(LEYENDA_CHISPA[l[i]], LEYENDA_CHISPA[l[i + 1]])
                          for i in range(0, len(l), 2)])
        datos = a_tramos(filas)
        total += len(datos)
        bloques.append(array_c('g_spr_' + nombre, datos))

    h = ['#ifndef SPRITES_H\n#define SPRITES_H\n#include <cpctelera.h>\n\n',
         '// Una postura: datos por tramos + posición respecto al ancla del luchador\n',
         'typedef struct {\n   const u8* datos;  // ancho, alto, y filas (inicio, longitud, bytes)\n',
         '   i8 dx;            // byte izquierdo relativo al ancla\n',
         '   i8 oy;            // fila superior relativa al suelo (negativa)\n',
         '   i8 frente;        // alcance por delante, en bytes\n} TPostura;\n\n']
    for i, n in enumerate(nombres):
        h.append('#define P_%s %d\n' % (n.upper(), i))
    h.append('#define NUM_POSTURAS %d\n\n' % len(nombres))
    h.append('extern const TPostura g_posturas[NUM_POSTURAS];\n')
    for nombre, _ in CHISPAS:
        h.append('extern const u8 g_spr_%s[];\n' % nombre)
    h.append('\n#endif\n')
    escribir('sprites.h', ''.join(h))
    escribir('sprites.c', '#include "sprites.h"\n\n' + '\n'.join(bloques) +
             '\nconst TPostura g_posturas[NUM_POSTURAS] = {\n' + '\n'.join(tabla) + '\n};\n')

    # Hoja de muestra en PNG: jugador 1 arriba, jugador 2 (espejo y otros colores) abajo
    celda_an, celda_al = 30, 64
    por_fila = 9
    nfil = (len(hoja) + por_fila - 1) // por_fila
    img = [[None] * (celda_an * por_fila) for _ in range(celda_al * nfil * 2)]
    for k, (filas, dx, oy) in enumerate(hoja):
        for j2 in (0, 1):
            cx = (k % por_fila) * celda_an + celda_an // 2
            base = ((k // por_fila) * 2 + j2) * celda_al + celda_al - 4
            for f, linea in enumerate(filas):
                for b, (a, d) in enumerate(linea):
                    for s, p in enumerate((a, d)):
                        if p is None:
                            continue
                        x = (dx + b) * 2 + s
                        if j2:
                            x, p = -x + 1, CAMBIO_J2.get(p, p)
                        y = base + oy + f
                        if 0 <= y < len(img) and 0 <= cx + x < len(img[0]):
                            img[y][cx + x] = p
            for x in range(celda_an):  # línea del suelo
                yy = base
                xx = (k % por_fila) * celda_an + x
                if img[yy][xx] is None:
                    img[yy][xx] = PEN['rojo']
    guardar_png(os.path.join(DIR_IMG, 'luchadores.png'), img, 4, 2)
    print('sprites: %d posturas, %d bytes' % (len(POSTURAS), total))
