#!/bin/bash
IDSK=~/sd/cpctelera/cpctelera/tools/iDSK-0.13/bin/iDSK
DSK=tron.dsk
BAS=tron.bas
$IDSK $DSK -n
unix2dos $BAS
$IDSK $DSK -i $BAS -t 0
echo "EJECUTANDO EMULADOR"
amstrad $DSK