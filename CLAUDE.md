# CLAUDE.md

Programas y pruebas para Amstrad CPC: BASIC y C/ensamblador con CPCtelera. El usuario está aprendiendo a programar para CPC; Claude le acompaña.

## Cómo trabajar
- Todo en español (explicaciones, comentarios del código, documentación).
- Explicar el porqué de las cosas y avanzar poco a poco; mejor un programa pequeño que funcione que uno grande a medias.
- Muchas carpetas son pruebas antiguas que no funcionan; el repaso está pendiente. No arreglarlas sin que el usuario lo pida.

## Entorno
- Todo se compila en **WSL (Debian)**; Windows solo aloja el emulador.
- CPCtelera (rama `development`) está en `~/cpctelera` dentro de WSL; sus `cpct_*` entran en el PATH vía `~/.bashrc`.
- Para lanzar comandos desde Claude: herramienta Bash con `wsl -d Debian -e bash -ic '...'` (con `-i` para que cargue `.bashrc`; comillas simples, desde PowerShell las comillas se rompen). El aviso `Failed to translate 'M:\bin'` es inofensivo.
- Ruta del repo en WSL: la equivalente a la carpeta de Windows donde esté clonado (`E:\x\y` → `/mnt/e/x/y`; o `wslpath -a 'E:\x\y'`). No hay ruta fija: deducirla del directorio de trabajo.

## Compilar y ejecutar (desde la raíz, en WSL)
- `./cpc.sh run <carpeta|fichero> [-e cap32|winape|rvm]`: compila y lanza en el emulador.
- `./cpc.sh build <carpeta|fichero>`: solo genera el `.dsk`.
- `./cpc.sh new <nombre>`: proyecto CPCtelera nuevo. `./cpc.sh emus`: emuladores configurados.
- Según el argumento:
  - `.bas`: `unix2dos` + iDSK al `.dsk`.
  - `.qbas`: primero `qbas2bas/qbas2bas.sh`, luego como `.bas`.
  - Carpeta con `src/`: regenera con `cpct_mkproject` lo que falte de la plantilla (`Makefile`, `cfg/`, `exp/`...), y luego `make`.
  - Carpeta con un único `.bas`/`.qbas`: lo usa directamente.
- Rutas de emuladores en `cpc.conf` (local, no versionado; plantilla `cpc.conf.example`). En este PC: Caprice32 en `../Amstrad_asm/tools/cap32/` (fuera del repo).

## Estructura y convenciones
- `*_bas/`: BASIC de Locomotive en ASCII con LF (cpc.sh lo convierte). `.qbas`: BASIC con etiquetas en vez de números de línea.
- Carpetas con `src/` (`holamundo/`, `example/`, `empirec/`...): proyectos CPCtelera; solo se versiona `src/`.
- `.gitignore` es una lista blanca: solo `src/`, `img/`, `*.bas`, `*.qbas`, `*.sh`, `*.md` (+ `cpc.conf.example`, `.gitattributes`). Un tipo de fichero nuevo (p. ej. mapas `.tmx`) hay que añadirlo explícitamente. Si un proyecto usa conversión de imágenes, su `cfg/image_conversion.mk` se añade a mano (ver `claudegame/`).
- CPCtelera no inicializa las variables globales con valor inicial: usar locales o `const`.
- Nombres AMSDOS: máximo 8 caracteres, **solo letras y números** (con `_` el `RUN"..."` da "Bad command"). Proyectos nuevos con nombre corto así.
- `.gitattributes` fuerza LF en `*.sh` (si no, bash falla al clonar en Windows).

## Verificar un programa (Claude)
- Lanzar con `./cpc.sh run ...`, esperar ~8-10 s y capturar la ventana con `powershell -File ../Amstrad_asm/tools/captura.ps1 <png en el scratchpad>` (si existe en este PC; está fuera del repo); después leer la imagen.
- Para simular una tecla, usar `keybd_event` manteniéndola pulsada ~300 ms (con `SendKeys` el CPC no la detecta).
- Al acabar, cerrar el emulador: `Stop-Process -Name cap32`.
