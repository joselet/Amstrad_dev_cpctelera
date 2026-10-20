1 ' cliente telnet v0.1
10 MODE 2:s%=0:buf$="":l%=255:REM Modo 80 columnas para Telnet
20 host$="ai.solvalou.com":port%=32323
30 |NETCONNECT,@s%,@host$,port%
40 IF s%=255 THEN PRINT "Error de conexion":END

1 REM --- Bucle Principal ---
60 |NETGET,s%,@buf$,@l% : REM Intentar leer datos
70 IF l%>0 THEN PRINT buf$;:IF log% THEN PRINT #8,buf$; : REM Mostrar y Loguear
80 k$=INKEY$
90 IF k$<>"" THEN |NETSEND,s%,@k$: REM Enviar tecla pulsada
100 GOTO 60

