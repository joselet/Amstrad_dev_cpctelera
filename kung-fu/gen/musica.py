# Genera la música: partituras escritas como texto -> bytes para el chip de sonido.
#
# El chip de sonido del CPC (AY-3-8912) tiene 3 canales. La música usa el A
# (melodía) y el B (bajo y percusión); el C queda libre para los efectos.
#
# Formato de cada canal en memoria: parejas (nota, duración en 1/50 s)
#   nota 0     = silencio
#   nota 1..60 = de DO2 a SI6 (índice en la tabla de periodos)
#   nota 0x7F  = golpe de percusión (ruido)
#   0xFF       = volver al principio (bucle)
#   0xFE       = fin (se queda en silencio)

from comun import array_c, escribir

NOMBRES = {'C': 0, 'C#': 1, 'D': 2, 'D#': 3, 'E': 4, 'F': 5, 'F#': 6, 'G': 7,
           'G#': 8, 'A': 9, 'A#': 10, 'B': 11}
NOTA_BASE_OCTAVA = 2      # nota 1 = C2
PERCUSION = 0x7F


def periodo(n):
    """Periodo del AY para la nota n (1 = C2). El AY del CPC va a 1 MHz:
    frecuencia = 1000000 / (16 * periodo)."""
    midi = 36 + (n - 1)                  # C2 = MIDI 36
    f = 440.0 * 2 ** ((midi - 69) / 12)
    return round(62500 / f)


def nota(txt):
    if txt == '-':
        return 0
    if txt == 'X':
        return PERCUSION
    nombre, octava = txt[:-1], int(txt[-1])
    return 1 + NOMBRES[nombre] + 12 * (octava - NOTA_BASE_OCTAVA)


def partitura(txt, unidad, final=0xFF):
    """'E5:4 D5:2 -:2 ...' -> bytes. La duración va en unidades (semicorcheas)."""
    datos = []
    for tok in txt.split():
        n, d = tok.split(':')
        datos += [nota(n), int(d) * unidad]
    return datos + [final]


# --- Tema del título: melodía tranquila en escala pentatónica de MI menor -----
TITULO_A = """
E5:4 D5:2 E5:2 G5:4 E5:4   D5:2 B4:2 A4:4 B4:8
E5:4 D5:2 E5:2 G5:4 A5:4   B5:4 A5:2 G5:2 E5:8
D5:4 B4:4 A4:4 G4:4        A4:4 B4:4 E4:8
G4:2 A4:2 B4:2 D5:2 E5:4 D5:4   B4:4 A4:4 E4:8
"""
TITULO_B = """
E2:4 B2:4 E3:4 B2:4   D2:4 A2:4 D3:4 A2:4
E2:4 B2:4 E3:4 B2:4   G2:4 D3:4 G3:4 D3:4
D2:4 A2:4 D3:4 A2:4   E2:4 B2:4 E3:4 B2:4
G2:4 D3:4 D2:4 A2:4   E2:4 B2:4 E2:8
"""

# --- Tema del combate: más rápido, con bajo machacón y percusión -------------
COMBATE_A = """
E5:2 -:2 E5:2 D5:2 B4:4 A4:4   B4:2 D5:2 B4:4 A4:2 G4:2 E4:4
E5:2 -:2 E5:2 D5:2 B4:4 A4:4   G4:2 A4:2 B4:4 D5:4 E5:4
-:16 -:16 -:16 -:8 B4:2 D5:2 E5:4
"""
COMBATE_B = """
E2:2 X:2 E3:2 E2:2 D3:2 X:2 B2:2 E2:2   E2:2 X:2 E3:2 E2:2 D3:2 X:2 B2:2 E2:2
E2:2 X:2 E3:2 E2:2 D3:2 X:2 B2:2 E2:2   G2:2 X:2 G3:2 G2:2 A2:2 X:2 B2:2 B2:2
E2:2 X:2 E3:2 E2:2 D3:2 X:2 B2:2 E2:2   E2:2 X:2 E3:2 E2:2 D3:2 X:2 B2:2 E2:2
E2:2 X:2 E3:2 E2:2 D3:2 X:2 B2:2 E2:2   G2:2 X:2 G3:2 G2:2 A2:2 X:2 B2:2 X:2
"""

# --- Jingles (no se repiten) --------------------------------------------------
VICTORIA_A = "E4:2 G4:2 B4:2 E5:2 -:1 B4:1 E5:12"
VICTORIA_B = "E2:2 E2:2 E2:2 E3:2 -:2 E2:12"
DERROTA_A = "B4:4 A4:4 G4:4 F#4:4 E4:16"
DERROTA_B = "E3:4 D3:4 C3:4 B2:4 E2:16"

CANCIONES = [
    # nombre, canal A, canal B, unidad (1/50 s), volumen A, volumen B, final
    ('TITULO', TITULO_A, TITULO_B, 7, 13, 11, 0xFF),
    ('COMBATE', COMBATE_A, COMBATE_B, 6, 10, 11, 0xFF),
    ('VICTORIA', VICTORIA_A, VICTORIA_B, 6, 13, 12, 0xFE),
    ('DERROTA', DERROTA_A, DERROTA_B, 8, 12, 11, 0xFE),
]


def genera():
    c = ['#include "musica.h"\n\n',
         '// Periodo del AY para cada nota (índice 1 = DO2)\n',
         array_c('g_periodos', [0] + [periodo(n) for n in range(1, 61)], 'u16', 10), '\n']
    tabla = []
    total = 0
    for nombre, a, b, unidad, va, vb, fin in CANCIONES:
        da, db = partitura(a, unidad, fin), partitura(b, unidad, fin)
        dur_a = sum(da[1:-1:2])
        dur_b = sum(db[1:-1:2])
        assert dur_a == dur_b, '%s: los canales duran distinto (%d y %d)' % (nombre, dur_a, dur_b)
        assert all(d < 256 for d in da[1:-1:2] + db[1:-1:2])
        n = nombre.lower()
        c.append(array_c('g_mus_%s_a' % n, da) + '\n' + array_c('g_mus_%s_b' % n, db) + '\n')
        tabla.append('   { g_mus_%s_a, g_mus_%s_b, %d, %d },' % (n, n, va, vb))
        total += len(da) + len(db)
    c.append('const TCancion g_canciones[NUM_CANCIONES] = {\n' + '\n'.join(tabla) + '\n};\n')
    h = ['#ifndef MUSICA_H\n#define MUSICA_H\n#include <cpctelera.h>\n\n',
         'typedef struct {\n   const u8* canal_a;\n   const u8* canal_b;\n',
         '   u8 vol_a, vol_b;   // volumen inicial de cada nota (0-15)\n} TCancion;\n\n',
         '#define NOTA_PERCUSION 0x%02X\n#define MUS_BUCLE 0xFF\n#define MUS_FIN 0xFE\n\n' % PERCUSION]
    for i, (nombre, *_) in enumerate(CANCIONES):
        h.append('#define MUS_%s %d\n' % (nombre, i + 1))   # 0 = sin música
    h.append('#define NUM_CANCIONES %d\n\n' % len(CANCIONES))
    h.append('extern const u16 g_periodos[61];\nextern const TCancion g_canciones[NUM_CANCIONES];\n\n#endif\n')
    escribir('musica.h', ''.join(h))
    escribir('musica.c', ''.join(c))
    print('musica: %d canciones, %d bytes' % (len(CANCIONES), total))
