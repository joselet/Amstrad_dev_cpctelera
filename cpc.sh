#!/bin/bash
# cpc.sh - compila y lanza las pruebas de Amstrad CPC de este repositorio
#
# Uso:
#   ./cpc.sh run   <carpeta|fichero> [-e cap32|winape|rvm]   compila y lanza en el emulador
#   ./cpc.sh build <carpeta|fichero>                         solo compila / genera el .dsk
#   ./cpc.sh new   <nombre>                                  nuevo proyecto CPCtelera
#   ./cpc.sh emus                                            emuladores configurados
#
# <carpeta|fichero> puede ser:
#   - una carpeta con src/      -> proyecto C/ASM de CPCtelera (make)
#   - un fichero .bas           -> BASIC de Amstrad
#   - un fichero .qbas          -> BASIC con etiquetas (se convierte con qbas2bas)
#   - una carpeta con un solo .bas o .qbas
#
# La configuración de cada ordenador (emuladores) va en cpc.conf (ver cpc.conf.example).

set -e

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONF="$REPO/cpc.conf"
QBAS2BAS="$REPO/qbas2bas/qbas2bas.sh"

EMU=cap32
CAP32=
WINAPE=
RVM=
CPCT_PATH=

error() { echo "ERROR: $*" >&2; exit 1; }

if [ -f "$CONF" ]; then
    source "$CONF"
else
    echo "AVISO: no existe cpc.conf. Créalo con:  cp cpc.conf.example cpc.conf  y edita las rutas." >&2
fi

# --- CPCtelera ---------------------------------------------------------------
if [ -n "$CPCT_PATH" ]; then
    PATH="$CPCT_PATH/tools/scripts:$PATH"
elif MK=$(command -v cpct_mkproject); then
    CPCT_PATH="$(cd "$(dirname "$MK")/../.." && pwd)"
else
    error "no encuentro CPCtelera (cpct_mkproject no está en el PATH). Abre una terminal nueva o define CPCT_PATH en cpc.conf"
fi
IDSK="$CPCT_PATH/tools/iDSK-0.13/bin/iDSK"
[ -x "$IDSK" ] || error "no encuentro iDSK en $IDSK"

# --- Utilidades --------------------------------------------------------------
is_wsl() { grep -qi microsoft /proc/version 2>/dev/null; }

# Ruta que entiende el emulador (en WSL los .exe de Windows necesitan rutas de Windows)
emu_path() { if is_wsl && [[ "$EXE" == *.exe ]]; then wslpath -w "$1"; else echo "$1"; fi; }

# Nombre AMSDOS: máximo 8 caracteres, solo letras y números
# (el emulador teclea el RUN"nombre y símbolos como _ dan "Bad command")
amsdos_name() {
    local n
    n=$(basename "$1"); n=${n%.*}
    echo "$n" | tr -cd 'A-Za-z0-9' | cut -c1-8 | tr 'A-Z' 'a-z'
}

# --- Compilación -------------------------------------------------------------
# Todas las funciones build_* dejan el resultado en DSK (ruta del disco) y RUNNAME (programa a ejecutar)

# build_bas <fichero.bas> <carpeta_destino>
build_bas() {
    local bas="$1" outdir="$2" name tmp
    name=$(amsdos_name "$bas")
    [ -n "$name" ] || error "nombre de fichero no válido: $bas"
    DSK="$outdir/$name.dsk"
    RUNNAME="$name"
    tmp=$(mktemp -d)
    cp "$bas" "$tmp/$name.bas"
    unix2dos -q "$tmp/$name.bas"
    "$IDSK" "$DSK" -n >/dev/null 2>&1
    (cd "$tmp" && "$IDSK" "$DSK" -i "$name.bas" -t 0 >/dev/null 2>&1)
    rm -rf "$tmp"
    echo "Creado $DSK con $name.bas"
}

# build_qbas <fichero.qbas>
build_qbas() {
    local qbas="$1" tmp
    tmp=$(mktemp -d)
    sh "$QBAS2BAS" "$qbas" "$tmp/$(basename "${qbas%.*}").bas"
    echo "Convertido $qbas -> BASIC de Amstrad"
    build_bas "$tmp/$(basename "${qbas%.*}").bas" "$(dirname "$qbas")"
    rm -rf "$tmp"
}

# build_cpct <carpeta con src/>
build_cpct() {
    local dir="$1" name tmp projname
    name=$(basename "$dir")
    # Git solo guarda src/: lo que falte de la plantilla (Makefile, cfg/, exp/...) se regenera con cpct_mkproject
    if [ ! -f "$dir/Makefile" ] || [ ! -d "$dir/cfg" ] || [ ! -d "$dir/exp" ]; then
        echo "Regenerando los ficheros de proyecto de $name con cpct_mkproject..."
        tmp=$(mktemp -d)
        (cd "$tmp" && cpct_mkproject "$name" >/dev/null)
        for f in "$tmp/$name"/* "$tmp/$name"/.vscode; do
            [ -e "$f" ] || continue
            [ "$(basename "$f")" = src ] && continue
            # -n: no pisa lo que ya exista (p. ej. un cfg/image_conversion.mk versionado)
            cp -rn "$f" "$dir/"
        done
        rm -rf "$tmp"
    fi
    make -C "$dir"
    projname=$(sed -n 's/^PROJNAME *:= *//p' "$dir/cfg/build_config.mk" | tr -d ' \r')
    DSK="$dir/$projname.dsk"
    [ -f "$DSK" ] || error "make no ha generado $DSK"
    RUNNAME=$("$IDSK" "$DSK" -l | awk '/\.BIN/ { split($1, n, "."); print tolower(n[1]); exit }')
    [ -n "$RUNNAME" ] || error "no encuentro ningún .BIN dentro de $DSK"
    if [[ "$RUNNAME" =~ [^a-z0-9] ]]; then
        echo "AVISO: '$RUNNAME' tiene símbolos y puede dar 'Bad command' al ejecutarlo." \
             "Cambia PROJNAME en cfg/build_config.mk por un nombre de solo letras y números." >&2
    fi
}

# build <carpeta|fichero>
build() {
    local target files
    [ -n "$1" ] || error "indica una carpeta o fichero"
    target=$(realpath "$1") || error "no existe: $1"

    if [ -d "$target" ]; then
        if [ -d "$target/src" ]; then
            build_cpct "$target"
            return
        fi
        shopt -s nullglob
        files=("$target"/*.bas "$target"/*.qbas)
        shopt -u nullglob
        if [ ${#files[@]} -eq 0 ]; then
            error "$1 no tiene src/ ni ficheros .bas/.qbas"
        elif [ ${#files[@]} -gt 1 ]; then
            echo "Hay varios programas en $1, indica cuál:" >&2
            for f in "${files[@]}"; do echo "  ./cpc.sh ${CMD} $(realpath --relative-to="$PWD" "$f")" >&2; done
            exit 1
        fi
        target="${files[0]}"
    fi

    case "${target,,}" in
        *.bas)  build_bas "$target" "$(dirname "$target")" ;;
        *.qbas) build_qbas "$target" ;;
        *)      error "tipo de fichero no soportado: $1 (usa .bas, .qbas o una carpeta con src/)" ;;
    esac
}

# --- Emuladores --------------------------------------------------------------
emu_exe() {
    case "$1" in
        cap32)  echo "$CAP32" ;;
        winape) echo "$WINAPE" ;;
        rvm)    echo "$RVM" ;;
        *)      error "emulador desconocido: $1 (cap32, winape o rvm)" ;;
    esac
}

# launch <emulador>   (usa DSK y RUNNAME)
launch() {
    local emu="$1" dsk
    EXE=$(emu_exe "$emu")
    [ -n "$EXE" ] || error "el emulador '$emu' no está configurado en cpc.conf"
    EXE=$(command -v "$EXE") || error "no encuentro el emulador '$emu' en $(emu_exe "$emu")"
    dsk=$(emu_path "$DSK")
    echo "Lanzando $emu: RUN\"$RUNNAME\""
    # El emulador se lanza desacoplado de la terminal (setsid) para no bloquearla
    cd "$(dirname "$EXE")"   # cap32 busca cap32.cfg en su propia carpeta
    case "$emu" in
        cap32)  setsid -f "$EXE" "$dsk" -a "run\"$RUNNAME" </dev/null >/dev/null 2>&1 ;;
        winape) setsid -f "$EXE" "$dsk" "/A:$RUNNAME" </dev/null >/dev/null 2>&1 ;;
        rvm)    setsid -f "$EXE" -b=cpc6128 -i "$dsk" -c="run\"$RUNNAME\n" </dev/null >/dev/null 2>&1 ;;
    esac
}

show_emus() {
    local emu exe mark
    for emu in cap32 winape rvm; do
        exe=$(emu_exe "$emu")
        mark=" "; [ "$emu" = "$EMU" ] && mark="*"
        if [ -z "$exe" ]; then
            printf "%s %-7s (no configurado)\n" "$mark" "$emu"
        elif command -v "$exe" >/dev/null; then
            printf "%s %-7s OK         %s\n" "$mark" "$emu" "$exe"
        else
            printf "%s %-7s NO EXISTE  %s\n" "$mark" "$emu" "$exe"
        fi
    done
    echo "(* = emulador por defecto, EMU en cpc.conf)"
}

usage() { sed -n '3,15p' "$0" | sed 's/^# \{0,1\}//'; exit 1; }

# --- Main --------------------------------------------------------------------
CMD="$1"; shift || true
ARGS=()
while [ $# -gt 0 ]; do
    case "$1" in
        -e) EMU="$2"; shift 2 ;;
        *)  ARGS+=("$1"); shift ;;
    esac
done

case "$CMD" in
    run)   build "${ARGS[0]}"; launch "$EMU" ;;
    build) build "${ARGS[0]}" ;;
    new)   [ -n "${ARGS[0]}" ] || error "indica el nombre del proyecto"
           (cd "$REPO" && cpct_mkproject "${ARGS[0]}") ;;
    emus)  show_emus ;;
    *)     usage ;;
esac
