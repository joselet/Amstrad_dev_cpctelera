10 MEMORY &3FFF
20 'login
30 state=0:' 0=pedir usuario, 1=pedir pass, 2=jugar
40 u$="":p$=""
50 MODE 1:INK 0,0:INK 1,26:INK 2,13:INK 3,6:BORDER 0
60 urlbase$="nosoft.es/services/ESP8266BBS/games/empire/core/empire.php" 
70 url$=urlbase$
80 GOSUB 200:u$=a$:'usuario
90 'contrase«a
100 state=1
110 url$=urlbase$+"?cmd="+u$
120 GOSUB 200:p$=a$:'password
130 'jugar
140 c$="v":'comando generico
150 ' bucle principal
160 url$=urlbase$+"?cmd="+c$+"|"+u$+"|"+p$
170 GOSUB 200
180 c$=a$
190 GOTO 150
200 ' peticion
201 REM FOR m=&4000 TO &47FF:POKE m,0:NEXT m
210 |HTTPMEM,url$,&4000,2048
215 IF PEEK(&4000)=83 AND PEEK(&4008)=126 THEN CLS:PRINT "Hasta la proxima Almirante":END:'SALIENDO~
220 'cls controlado por codigo ascii del servidor
230 FOR i=&4000 TO &47FF
240   k=PEEK(i):IF k=126 THEN GOTO 280: ' si encuentra chr$(126) se acaba el buffer
250   IF k=27 THEN GOSUB 300
260   SOUND 1,20,1,7:PRINT CHR$(k);
270 NEXT i
280 SOUND 1,120,5,15:SOUND 1,60,5,15:INPUT "",a$
290 RETURN
300 'transformacion de colores ansi a colores amstrad
310 i=i+2:c1=PEEK(i)
320 'reset color
330 IF c1=48 THEN PRINT CHR$(14)+CHR$(0)+CHR$(15)+CHR$(1);:i=i+1: RETURN
340 i=i+3:col=PEEK(i)
350 'pen 2=azul 3=rojo 1=amarillo
355 'debug del codigo de color:'PRINT col;
360 IF col=53 THEN PRINT CHR$(14)+CHR$(3)+CHR$(15)+CHR$(0);:'rojo/negro
370 IF col=51 THEN PRINT CHR$(15)+CHR$(2);:                 'negro/cyan
380 IF col=50 THEN PRINT CHR$(14)+CHR$(2)+CHR$(15)+CHR$(0);:'cyan/negro
390 IF col=52 THEN PRINT CHR$(14)+CHR$(1)+CHR$(15)+CHR$(2);:'blanco/cyan
400 IF col=49 THEN PRINT CHR$(15)+CHR$(3);:                 'negro/rojo
410 IF col=48 THEN PRINT CHR$(14)+CHR$(1)+CHR$(15)+CHR$(0);:'blanco/negro
420 IF col=54 THEN PRINT CHR$(14)+CHR$(2)+CHR$(15)+CHR$(1);:'cyan/blanco
490 IF col=91 THEN CLS
500 i=i+1
510 RETURN
