# Programas y pruebas de programas Amstrad

Programas en BASIC (carpetas `*_bas/`) y proyectos en C/ensamblador con [CPCtelera](https://github.com/lronaldo/cpctelera) (`holamundo/`, `invaders/`, `empirec/`...). Muchos son pruebas que aún no funcionan.

## Instalación en un ordenador nuevo (Windows)

CPCtelera funciona en Linux, así que en Windows se usa **WSL** (Linux dentro de Windows). Casi todo se ejecuta **dentro de WSL**; en Windows solo se instala WSL y el emulador.

| Paso | Dónde se ejecuta |
|------|------------------|
| 1. Instalar WSL + Debian | 🪟 PowerShell **como administrador** |
| 2. Dependencias (`apt`) | 🐧 Terminal WSL (Debian) |
| 3. Instalar CPCtelera | 🐧 Terminal WSL (Debian) |
| 4. Clonar este repositorio | 🐧 Terminal WSL (o Windows, da igual) |
| 5. Emulador (Caprice32 / WinAPE / RetroVirtualMachine) | 🪟 Windows (descargar y descomprimir) |
| 6. Configurar rutas de emuladores (`cpc.conf`) | 🐧 Terminal WSL (Debian) |
| Compilar y lanzar programas | 🐧 Terminal WSL (Debian) |

### 1. 🪟 Instalar WSL con Debian (PowerShell como administrador)
```powershell
wsl --install -d Debian
```
Reinicia si lo pide. La primera vez que se abre Debian pide crear un usuario y contraseña de Linux (la contraseña es la de `sudo`).

Opcional, recomendable: WSL2 compila bastante más rápido que WSL1:
```powershell
wsl --set-version Debian 2
```

Para abrir la terminal de Linux desde cualquier ventana de Windows: `wsl -d Debian` (o buscar "Debian" en el menú Inicio).

### 2. 🐧 Dependencias (terminal WSL)
```bash
sudo apt update
sudo apt install -y build-essential bison flex libboost-dev libfreeimage-dev mono-runtime wget curl unzip dos2unix git
```
`mono` lo exige el `setup.sh` de CPCtelera (para convertir música de Arkos Tracker) aunque no se use.

### 3. 🐧 Instalar CPCtelera (terminal WSL)
```bash
cd ~
git clone -b development https://github.com/lronaldo/cpctelera.git
cd ~/cpctelera
./setup.sh
```
Se usa la rama `development` porque los proyectos de este repo usan funciones que no están en `master`, como `cpct_setDrawCharM1`.
`setup.sh` compila SDCC, iDSK, etc. (tarda bastante) y añade CPCtelera al PATH en `~/.bashrc`. Después **cierra y vuelve a abrir** la terminal (o `source ~/.bashrc`) y comprueba:
```bash
which cpct_mkproject cpct_winape
```
(`sdcc` e `iDSK` no se añaden al PATH: están en `~/cpctelera/cpctelera/tools/` y los usan los Makefiles y scripts con su ruta completa.)

Prueba rápida de que todo compila:
```bash
cd /tmp && cpct_mkproject prueba && cd prueba && make   # debe generar prueba.dsk
```
Ojo: `cpct_mkproject` solo acepta un nombre de carpeta, sin `/` (hay que hacer `cd` antes).

### 4. 🐧 Clonar este repositorio
Puede estar en un disco de Windows. Desde WSL los discos de Windows están en `/mnt/<letra>/`, por ejemplo `E:\cajondesastre\...` → `/mnt/e/cajondesastre/...`.

### 5. 🪟 Emulador (Windows)
Vale cualquiera de estos (basta con uno):
- **Caprice32** (recomendado): es portable, solo hay que descomprimirlo. No necesita instalación ni permisos de administrador. https://github.com/ColinPitrat/caprice32/releases
- **WinAPE**.
- **RetroVirtualMachine**.

Desde WSL se pueden ejecutar directamente los `.exe` de Windows, así que el emulador se abre como una ventana normal.

### 6. 🐧 Configurar este ordenador (`cpc.conf`)
Las rutas de los emuladores se configuran **una sola vez** en `cpc.conf`. Ese fichero no se sube a git, porque cada ordenador tiene las suyas:
```bash
cp cpc.conf.example cpc.conf
nano cpc.conf          # rutas de Linux: E:\... se escribe /mnt/e/...
./cpc.sh emus          # comprueba qué emuladores encuentra
```

## Cómo compilar y ejecutar los programas (🐧 terminal WSL)

Todo se hace con `cpc.sh`, desde la raíz del repositorio:
```bash
./cpc.sh run holamundo_bas                 # BASIC: crea el .dsk y lo ejecuta en el emulador
./cpc.sh run tron_bas/tron.bas             # si la carpeta tiene varios .bas, se indica cuál
./cpc.sh run qbas2bas/prueba.qbas          # BASIC con etiquetas: lo convierte con qbas2bas y lo ejecuta
./cpc.sh run example                       # proyecto C de CPCtelera: make + emulador
./cpc.sh run example -e winape             # con otro emulador distinto del de por defecto
./cpc.sh build example                     # solo compilar / generar el .dsk
./cpc.sh new mi_proyecto                   # crear un proyecto CPCtelera nuevo
```

Qué hace `cpc.sh` según lo que le pases:
- **`.bas`**: lo pasa a finales de línea de DOS y lo mete en un `.dsk` con iDSK. En el disco el nombre se queda solo con letras y números, con un máximo de 8 caracteres (AMSDOS). Por ejemplo, `ff_0.2.qbas` se graba como `ff02`. Símbolos como `_` dan "Bad command" al hacer `RUN`.
- **`.qbas`**: es BASIC escrito con etiquetas en lugar de números de línea. Primero lo convierte con `qbas2bas/qbas2bas.sh` y después lo trata como un `.bas`.
- **Carpeta con `src/`** (C/ensamblador con CPCtelera): Git solo guarda `src/` (ver `.gitignore`). Si faltan los ficheros de proyecto (`Makefile`, `cfg/`, `exp/`...), los regenera automáticamente con `cpct_mkproject`, sin tocar `src/`, y luego ejecuta `make`.

Los ficheros generados (`.dsk`, `obj/`, `Makefile`, `cfg/`) no se suben a git.