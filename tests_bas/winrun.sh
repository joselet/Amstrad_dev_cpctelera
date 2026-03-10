#!/bin/bash
IDSK=~/cpctelera/cpctelera/tools/iDSK-0.13/bin/iDSK
DSK=window.dsk
BAS=window.bas
$IDSK $DSK -n
unix2dos $BAS
$IDSK $DSK -i $BAS -t 0


echo "---------------------------------"
echo "Selecciona el emulador a usar:"
echo "---------------------------------"

options=("WinApe" "RetroVirtualMachine" "Salir")

select opt in "${options[@]}"
do
    case $opt in
        "WinApe")
            echo "EJECUTANDO WINAPE..."
            cpct_winape $DSK
            break
            ;;
        "RetroVirtualMachine")
            echo "EJECUTANDO RVM..."
            # Nota: Usamos comillas dobles externas para que $BAS se reconozca
            # y escapamos las comillas internas con \ para el comando BASIC
            /mnt/d/RETRO/cpc6128/emu/retroVirtualMachine/RetroVirtualMachine.exe \
            -b=cpc6128 \
            -i $DSK \
            -c="run\"$BAS\n"
            break
            ;;
        "Salir")
            exit
            ;;
        *) echo "Opción inválida $REPLY";;
    esac
done