1 ' Versíón de 2 jugadores, con galletas.
1 '1 Instrucciones del juego
1 '2 inicialización de marcador y variables del jugador 1
1 '3 inicialización de marcador y variables del jugador 2
1 '4 captura de teclado del movimiento del jugador 1
1 '5 captura de teclado del movimiento del jugador 2
1 '6 logica de galletas
1 '7 calculo de colisiones y refresco de posiciones de jugador 1 en pantalla
1 '8 calculo de colisiones y refresco de posiciones de jugador 2 en pantalla
1 '9 pintado de pantalla o reinicio en caso de muerte
1 'Variables:
1 't inicialización del contador de tiempo ((time-t)/300 = el tiempo que lleva el jugador vivo = ta)
1 'x y coordenadas del jugador
1 'vx vy velocidad horizontal o vertical del jugador
1 'm contador de número de muertes del jugador
1 'p contador de puntos del jugador
1 'col indicador que hubo colisión

1 ' probar ENT envolvente de tono y ENV envolvente de volumen
1 ' https://latchdevel.github.io/Amstrad-CPC6128-Manual-del-Usuario/7.05.-Esquemas-de-teclado.html
1 ' https://latchdevel.github.io/Amstrad-CPC6128-Manual-del-Usuario/1.09.-Sonidos.html
1 ' https://latchdevel.github.io/Amstrad-CPC6128-Manual-del-Usuario/3.01.-Lista-completa-de-las-palabras-clave-del-BASIC-de-AMSTRAD.html

1 mode 1:?"Light bikes":?"Avoid crashing with any light path.":?"Get Yellow pixels to earn points":?"Controls: P1. W,A,S,D   P2.Arrow keys":input"Press Enter to start",a$
2 cls:t=time:y1=120:x1=50:vx1=2:vy1=0:pen 2:?"P1: D:";m1;"P: ";p1;"   LTA:";ta1$:move 10,10:draw 10,360,3:draw 630,360:draw 630,10:draw 10,10
3 y2=120:x2=550:vx2=-2:vy2=0:pen 3:?"P2: D:";m2;"P: ";p2;"   LTA:";ta2$
4 if inkey(59)=0 then vy1 = 2:vx1 = 0 else if inkey(60)=0 then vy1=-2:vx1=0 else if inkey(69)=0 then vx1=-2:vy1=0 else if inkey(61)=0 then vx1=2:vy1=0
5 if inkey(0)=0 then vy2 = 2:vx2 = 0 else if inkey(2)=0 then vy2=-2:vx2=0 else if inkey(8)=0 then vx2=-2:vy2=0 else if inkey(1)=0 then vx2=2:vy2=0
6 c=c+1:if c>10 then c=0:SOUND 1,100,5:randomize time:plot(INT(RND*620)+10),(INT(RND*350)+10),1:plot x1,y1,2:plot x2,y2,3
7 x1=x1+vx1:y1=y1+vy1:if test(x1,y1)=1 then SOUND 1,50,5:pen 2:p1=p1+1:locate 13,1:?p1:c=100:goto 6 else if test(x1,y1)<>0 then ta1$=str$((time-t)/300):ta2$="?":m1=m1+1:colision=1
8 x2=x2+vx2:y2=y2+vy2:if test(x2,y2)=1 then SOUND 1,50,5:pen 3:p2=p2+1:locate 13,2:?p2:c=100:goto 6 else if test(x2,y2)<>0 then ta2$=str$((time-t)/300):ta1$="?":m2=m2+1:colision=1
9 frame:if colision=1 then colision=0:SOUND 1,0,100,12,0,0,31:for w=1 to 50:frame:next:goto 2 else plot x1,y1,2:plot x2,y2,3:goto 4

