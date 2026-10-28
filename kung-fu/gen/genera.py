#!/usr/bin/env python3
# Genera todos los datos del juego: sprites, fondo, fuente, música y tablas.
# Uso (desde la carpeta kung-fu):  python3 gen/genera.py
# Escribe los .c/.h en src/datos/ y unas imágenes de muestra en img/.

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import comun
import sprites
import fondo
import tablas
import musica

os.makedirs(comun.DIR_IMG, exist_ok=True)
sprites.genera()
fondo.genera()
tablas.genera()
musica.genera()
