10 ' NO FUNCIONA. Se probaron listener y sender en python desde pc distintos
10 ' y esos scrips si fucnionan, pero el script de basic no funciona

10 ' --- CONFIGURACION ---
20 IP1=192:IP2=168:IP3=1:IP4=20:PORT=8080
30 MSG$="HOLA DESDE MI AMSTRAD CPC!"
40 SOCK=1: ' Usaremos el socket 1
50 DATAPORT=&FE00:ACKPORT=&FC00
60 '
70 ' --- 1. ABRIR SOCKET (TCP) ---
80 OUT DATAPORT,3: ' Tamaño del comando (3 bytes)
90 OUT DATAPORT,&31:OUT DATAPORT,&43: ' Comando C_NETSOCKET (&4331)
100 OUT DATAPORT,0: ' Protocolo 0=TCP
110 OUT ACKPORT,0: FOR T=1 TO 500:NEXT T: 'Ejecutar, Pausa tras abrir socket
120 '
130 ' --- 2. CONECTAR A IP ---
140 OUT DATAPORT,9
150 OUT DATAPORT,&32:OUT DATAPORT,&43
160 OUT DATAPORT,SOCK
170 ' Enviamos la IP
180 OUT DATAPORT,IP1:OUT DATAPORT,IP2:OUT DATAPORT,IP3:OUT DATAPORT,IP4
185 ' Enviamos el puerto (Low byte primero, luego High byte)
190 OUT DATAPORT,PORT AND 255:OUT DATAPORT,INT(PORT/256)
200 OUT ACKPORT,0
205 ' Pequeño retardo para que la M4 procese el comando antes de leer el estado
206 FOR T=1 TO 200:NEXT T
210 INFO = PEEK(&FF06) + 256 * PEEK(&FF07)
220 STADDR = INFO + (SOCK * 16)
230 ST = PEEK(STADDR)
240 IF ST = 1 THEN PRINT ".";:GOTO 230: ' Imprime puntos mientras conecta
250 IF ST <> 0 THEN PRINT "ERROR:";ST:END
260 PRINT "¡CONECTADO!"
310 MSG$="TEST"
320 L=LEN(MSG$): ' L es 4
330 ' Tamaño total: 1 (ID Lo) + 1 (ID Hi) + 1 (Socket) + 2 (Size) + 4 (Datos) = 9
340 OUT DATAPORT,9
350 OUT DATAPORT,&34:OUT DATAPORT,&43
360 OUT DATAPORT,SOCK
370 OUT DATAPORT,4:OUT DATAPORT,0: ' Longitud del mensaje (4 bytes)
380 FOR N=1 TO 4:OUT DATAPORT,ASC(MID$(MSG$,N,1)):NEXT
390 OUT ACKPORT,0
400 PRINT "ENVIADO."
