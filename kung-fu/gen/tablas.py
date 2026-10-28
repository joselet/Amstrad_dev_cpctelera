# Genera la paleta, la fuente de letras y las tablas que usan las rutinas de dibujo.

from comun import PALETA, HW_FW, CAMBIO_J2, byte_m0, pixeles_m0, array_c, escribir

# Fuente de 5x7 píxeles. '#' = píxel encendido.
FUENTE = {
    ' ': ['.....'] * 7,
    '!': ['..#..', '..#..', '..#..', '..#..', '..#..', '.....', '..#..'],
    '-': ['.....', '.....', '.....', '.###.', '.....', '.....', '.....'],
    '+': ['.....', '..#..', '..#..', '#####', '..#..', '..#..', '.....'],
    ',': ['.....', '.....', '.....', '.....', '.##..', '..#..', '.#...'],
    '/': ['....#', '...#.', '...#.', '..#..', '.#...', '.#...', '#....'],
    '.': ['.....', '.....', '.....', '.....', '.....', '.##..', '.##..'],
    ':': ['.....', '.##..', '.##..', '.....', '.##..', '.##..', '.....'],
    '?': ['.###.', '#...#', '....#', '..##.', '..#..', '.....', '..#..'],
    '0': ['.###.', '#...#', '#..##', '#.#.#', '##..#', '#...#', '.###.'],
    '1': ['..#..', '.##..', '..#..', '..#..', '..#..', '..#..', '.###.'],
    '2': ['.###.', '#...#', '....#', '..##.', '.#...', '#....', '#####'],
    '3': ['####.', '....#', '....#', '.###.', '....#', '....#', '####.'],
    '4': ['...#.', '..##.', '.#.#.', '#..#.', '#####', '...#.', '...#.'],
    '5': ['#####', '#....', '####.', '....#', '....#', '#...#', '.###.'],
    '6': ['.###.', '#....', '#....', '####.', '#...#', '#...#', '.###.'],
    '7': ['#####', '....#', '...#.', '..#..', '.#...', '.#...', '.#...'],
    '8': ['.###.', '#...#', '#...#', '.###.', '#...#', '#...#', '.###.'],
    '9': ['.###.', '#...#', '#...#', '.####', '....#', '....#', '.###.'],
    'A': ['.###.', '#...#', '#...#', '#####', '#...#', '#...#', '#...#'],
    'B': ['####.', '#...#', '#...#', '####.', '#...#', '#...#', '####.'],
    'C': ['.###.', '#...#', '#....', '#....', '#....', '#...#', '.###.'],
    'D': ['####.', '#...#', '#...#', '#...#', '#...#', '#...#', '####.'],
    'E': ['#####', '#....', '#....', '####.', '#....', '#....', '#####'],
    'F': ['#####', '#....', '#....', '####.', '#....', '#....', '#....'],
    'G': ['.###.', '#...#', '#....', '#.###', '#...#', '#...#', '.####'],
    'H': ['#...#', '#...#', '#...#', '#####', '#...#', '#...#', '#...#'],
    'I': ['.###.', '..#..', '..#..', '..#..', '..#..', '..#..', '.###.'],
    'J': ['..###', '...#.', '...#.', '...#.', '#..#.', '#..#.', '.##..'],
    'K': ['#...#', '#..#.', '#.#..', '##...', '#.#..', '#..#.', '#...#'],
    'L': ['#....', '#....', '#....', '#....', '#....', '#....', '#####'],
    'M': ['#...#', '##.##', '#.#.#', '#.#.#', '#...#', '#...#', '#...#'],
    'N': ['#...#', '##..#', '#.#.#', '#..##', '#...#', '#...#', '#...#'],
    'O': ['.###.', '#...#', '#...#', '#...#', '#...#', '#...#', '.###.'],
    'P': ['####.', '#...#', '#...#', '####.', '#....', '#....', '#....'],
    'Q': ['.###.', '#...#', '#...#', '#...#', '#.#.#', '#..#.', '.##.#'],
    'R': ['####.', '#...#', '#...#', '####.', '#.#..', '#..#.', '#...#'],
    'S': ['.####', '#....', '#....', '.###.', '....#', '....#', '####.'],
    'T': ['#####', '..#..', '..#..', '..#..', '..#..', '..#..', '..#..'],
    'U': ['#...#', '#...#', '#...#', '#...#', '#...#', '#...#', '.###.'],
    'V': ['#...#', '#...#', '#...#', '#...#', '#...#', '.#.#.', '..#..'],
    'W': ['#...#', '#...#', '#...#', '#.#.#', '#.#.#', '##.##', '#...#'],
    'X': ['#...#', '#...#', '.#.#.', '..#..', '.#.#.', '#...#', '#...#'],
    'Y': ['#...#', '#...#', '.#.#.', '..#..', '..#..', '..#..', '..#..'],
    'Z': ['#####', '....#', '...#.', '..#..', '.#...', '#....', '#####'],
}


def genera():
    # Paleta en valores hardware (los que entiende el Gate Array)
    paleta = [HW_FW[fw] for _, fw in PALETA]

    # Tabla de máscaras: para cada byte de sprite, qué bits del fondo se conservan.
    # Un píxel con tinta 0 es transparente: sus bits de la máscara valen 1.
    mascara = []
    for b in range(256):
        izq, der = pixeles_m0(b)
        mascara.append(byte_m0(15 if izq == 0 else 0, 15 if der == 0 else 0))

    # Tabla del jugador 2: cambia colores y da la vuelta a los 2 píxeles del byte
    espejo = []
    for b in range(256):
        izq, der = pixeles_m0(b)
        espejo.append(byte_m0(CAMBIO_J2.get(der, der), CAMBIO_J2.get(izq, izq)))

    # Bytes de cada tinta como píxel izquierdo y como derecho (para pintar texto)
    pen_izq = [byte_m0(p, 0) for p in range(16)]
    pen_der = [byte_m0(0, p) for p in range(16)]

    # Fuente: caracteres ASCII 32 (espacio) a 90 (Z), 7 bytes por letra
    fuente = []
    for codigo in range(32, 91):
        dib = FUENTE.get(chr(codigo), ['.....'] * 7)
        for fila in dib:
            v = 0
            for i, ch in enumerate(fila):
                if ch == '#':
                    v |= 0x80 >> i
            fuente.append(v)

    escribir('tablas.h',
             '#ifndef TABLAS_H\n#define TABLAS_H\n#include <cpctelera.h>\n\n'
             'extern const u8 g_paleta[16];          // colores hardware de las 16 tintas\n'
             'extern const u8 g_tabla_mascara[256];  // byte de sprite -> máscara de transparencia\n'
             'extern const u8 g_tabla_espejo[256];   // byte de sprite -> byte en espejo y con colores del J2\n'
             'extern const u8 g_pen_izq[16];         // tinta -> bits del píxel izquierdo\n'
             'extern const u8 g_pen_der[16];         // tinta -> bits del píxel derecho\n'
             'extern const u8 g_fuente[%d];         // letras 5x7, ASCII 32..90, 7 bytes cada una\n\n'
             '#endif\n' % len(fuente))
    escribir('tablas.c', '#include "tablas.h"\n\n' +
             array_c('g_paleta', paleta) + '\n' +
             array_c('g_tabla_mascara', mascara) + '\n' +
             array_c('g_tabla_espejo', espejo) + '\n' +
             array_c('g_pen_izq', pen_izq) + '\n' +
             array_c('g_pen_der', pen_der) + '\n' +
             array_c('g_fuente', fuente, por_linea=14))
    print('tablas: paleta, máscara, espejo, fuente (%d bytes)' % len(fuente))
