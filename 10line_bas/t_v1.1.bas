1 ' Versíón de 1 jugador, con galletas.
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

10 call &bbff:t=time:y=120:x=50:vx=2:vy=0:locate 1,1:?"LTA:";ta:?"D:";m;"P: 0":move 10,10:draw 10,360,3:draw 630,360:draw 630,10:draw 10,10
20 if inkey(0) = 0 then vy = 2:vx = 0 else if inkey(2) = 0 then vy = -2:vx = 0 else if inkey(8) = 0 then vx = -2:vy = 0 else if inkey(1) = 0 then vx = 2:vy = 0
30 c=c+1:if c>50 then c=0:randomize time:plot(INT(RND*620)+10),(INT(RND*350)+10),1:plot x,y,2
40 frame:x=x+vx:y=y+vy:if test(x,y)=1 then p=p+1:locate 6,2:?"P";p:c=100:goto 30 else if test (x,y)<>0 then ta=(time-t)/300:m=m+1:p=0:goto 10 else plot x,y,2:goto 20
1


