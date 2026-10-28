# Utilidades comunes del generador: paleta, PNG y formato de bytes del modo 0.

import os
import struct
import zlib

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DIR_DATOS = os.path.join(RAIZ, 'src', 'datos')
DIR_IMG = os.path.join(RAIZ, 'img')

# --- Los 27 colores del CPC (número de firmware -> RGB y valor hardware) -----
RGB_FW = [
    (0, 0, 0), (0, 0, 128), (0, 0, 255), (128, 0, 0), (128, 0, 128),
    (128, 0, 255), (255, 0, 0), (255, 0, 128), (255, 0, 255), (0, 128, 0),
    (0, 128, 128), (0, 128, 255), (128, 128, 0), (128, 128, 128), (128, 128, 255),
    (255, 128, 0), (255, 128, 128), (255, 128, 255), (0, 255, 0), (0, 255, 128),
    (0, 255, 255), (128, 255, 0), (128, 255, 128), (128, 255, 255), (255, 255, 0),
    (255, 255, 128), (255, 255, 255),
]
HW_FW = [0x14, 0x04, 0x15, 0x1C, 0x18, 0x1D, 0x0C, 0x05, 0x0D, 0x16, 0x06, 0x17,
         0x1E, 0x00, 0x1F, 0x0E, 0x07, 0x0F, 0x12, 0x02, 0x13, 0x1A, 0x19, 0x1B,
         0x0A, 0x03, 0x0B]

# --- Nuestra paleta: 16 tintas (pens) del modo 0 ------------------------------
# Cada tinta tiene un nombre corto para usarla en el resto del generador.
PALETA = [
    ('trans',    0),   # 0  negro: transparente en los sprites, fondo del marcador
    ('negro',    0),   # 1  negro opaco: contornos, pelo, cinturón
    ('azul',     1),   # 2  azul oscuro: cielo alto, sombra del kimono azul
    ('azulc',    2),   # 3  azul brillante: kimono del jugador 2
    ('magenta',  4),   # 4  magenta: montañas lejanas
    ('purpura',  7),   # 5  púrpura: cielo
    ('rojo',     3),   # 6  rojo oscuro: sombra de piel, suelo
    ('rojoc',    6),   # 7  rojo vivo: cinta del pelo, energía perdida
    ('naranja', 15),   # 8  naranja: piel, horizonte
    ('amarillo', 24),  # 9  amarillo: sol, energía
    ('crema',   25),   # 10 amarillo pastel: centro del sol
    ('gris',    13),   # 11 gris: sombra del kimono blanco
    ('blanco',  26),   # 12 blanco: kimono del jugador 1, textos
    ('malva',    5),   # 13 malva: cielo
    ('rosa',    16),   # 14 rosa: cerezo, brillo de piel
    ('verde',    9),   # 15 verde: hojas del pino
]
PEN = {nombre: i for i, (nombre, _) in enumerate(PALETA)}
RGB_PEN = [RGB_FW[fw] for _, fw in PALETA]

# Cambio de colores del jugador 1 al jugador 2 (el resto se queda igual)
CAMBIO_J2 = {
    PEN['blanco']: PEN['azulc'],   # kimono
    PEN['gris']: PEN['azul'],      # sombra del kimono
    PEN['rojoc']: PEN['amarillo'], # cinta del pelo
}


def byte_m0(izq, der):
    """Junta 2 píxeles (tintas 0-15) en un byte de memoria de vídeo del modo 0.
    El píxel izquierdo usa los bits 7,3,5,1 y el derecho los bits 6,2,4,0."""
    b = 0
    for bit, (pi, pd) in enumerate(((7, 6), (3, 2), (5, 4), (1, 0))):
        if izq >> bit & 1:
            b |= 1 << pi
        if der >> bit & 1:
            b |= 1 << pd
    return b


def pixeles_m0(b):
    """Lo contrario de byte_m0: devuelve (izq, der)."""
    izq = der = 0
    for bit, (pi, pd) in enumerate(((7, 6), (3, 2), (5, 4), (1, 0))):
        if b >> pi & 1:
            izq |= 1 << bit
        if b >> pd & 1:
            der |= 1 << bit
    return izq, der


def guardar_png(ruta, filas, escala_x=2, escala_y=1):
    """filas: lista de listas de tintas (None = transparente, se pinta de gris oscuro).
    Se amplía para que se vea con la proporción real del modo 0 (píxel el doble de ancho)."""
    def rgb(p):
        return (40, 40, 40) if p is None else RGB_PEN[p]
    grande = []
    for f in filas:
        linea = b''.join(bytes(rgb(p)) * escala_x for p in f)
        grande += [linea] * escala_y
    alto, ancho = len(grande), len(filas[0]) * escala_x
    crudo = b''.join(b'\x00' + l for l in grande)

    def bloque(tipo, datos):
        return (struct.pack('>I', len(datos)) + tipo + datos
                + struct.pack('>I', zlib.crc32(tipo + datos) & 0xffffffff))
    with open(ruta, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n')
        f.write(bloque(b'IHDR', struct.pack('>IIBBBBB', ancho, alto, 8, 2, 0, 0, 0)))
        f.write(bloque(b'IDAT', zlib.compress(crudo, 9)))
        f.write(bloque(b'IEND', b''))


def array_c(nombre, datos, tipo='u8', por_linea=16):
    """Devuelve el texto C de un array constante."""
    lineas = []
    for i in range(0, len(datos), por_linea):
        trozo = datos[i:i + por_linea]
        if tipo == 'u8':
            lineas.append('   ' + ', '.join('0x%02X' % (v & 0xFF) for v in trozo) + ',')
        else:
            lineas.append('   ' + ', '.join(str(v) for v in trozo) + ',')
    return 'const %s %s[%d] = {\n%s\n};\n' % (tipo, nombre, len(datos), '\n'.join(lineas))


CABECERA = ('// Fichero GENERADO por gen/genera.py: no editar a mano.\n'
            '// Para cambiarlo, modifica el generador y ejecuta: python3 gen/genera.py\n')


def escribir(nombre, texto):
    os.makedirs(DIR_DATOS, exist_ok=True)
    with open(os.path.join(DIR_DATOS, nombre), 'w', newline='\n') as f:
        f.write(CABECERA + texto)
