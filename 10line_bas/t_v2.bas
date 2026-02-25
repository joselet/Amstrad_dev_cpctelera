1 ' Versíón de 2 jugadores, con galletas.
1 '10 inicialización de pantalla, variables
1 '20 lógica del jugador
1 '30 logica de galletas
1 '40 calculo de colisiones y refresco de posiciones de jugador en pantalla
1 'Variables:
1 't inicialización del contador de tiempo ((time-t)/300 = el tiempo que lleva el jugador vivo = ta)
1 'x y coordenadas del jugador
1 'vx vy velocidad horizontal o vertical del jugador
1 'm contador de número de muertes del jugador
1 'p contador de puntos del jugador


170 IF INKEY(59) <> -1 THEN VY2=  vel : VX2=0 : 'W
180 IF INKEY(60) <> -1 THEN VY2= -vel : VX2=0 : 'S
190 IF INKEY(69) <> -1 THEN VX2= -vel : VY2=0 : 'A
200 IF INKEY(61) <> -1 THEN VX2=  vel : VY2=0 : 'D

1 call &bbff:t=time:y=120:x=50:vx=2:vy=0:locate 1,1:?"LTA:";ta:?"D:";m;"P: 0":move 10,10:draw 10,360,3:draw 630,360:draw 630,10:draw 10,10
4 if inkey(0) = 0 then vy = 2:vx = 0 else if inkey(2) = 0 then vy = -2:vx = 0 else if inkey(8) = 0 then vx = -2:vy = 0 else if inkey(1) = 0 then vx = 2:vy = 0
6 c=c+1:if c>50 then c=0:randomize time:plot(INT(RND*620)+10),(INT(RND*350)+10),1:plot x,y,2
8 frame:x=x+vx:y=y+vy:if test(x,y)=1 then p=p+1:locate 6,2:?"P";p:c=100:goto 6 else if test (x,y)<>0 then ta=(time-t)/300:m=m+1:p=0:goto 1 else plot x,y,2:goto 4



