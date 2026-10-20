# make && /mnt/d/RETRO/cpc6128/emu/retroVirtualMachine/RetroVirtualMachine.exe -b=cpc6128 -s invaders.sna
# make && cpct_winape invaders.dsk

make

DSK=invaders.dsk
SNA=invaders.sna

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
            -s $SNA
            break
            ;;
        "Salir")
            exit
            ;;
        *) echo "Opción inválida $REPLY";;
    esac
done