1 ' Versíón de 1 jugador, con galletas. 4 líneas de código: 160 columnas
1 '10 inicialización de pantalla, variables
1 '20 lógica del jugador
1 '30 logica de galletas
1 '40 calculo de colisiones y refresco de posiciones de jugador en pantalla
1 'Variables:
1 't inicialización del contador de tiempo ((time-t)/300 = el tiempo que lleva el jugador vivo = s)
1 'x y coordenadas del jugador
1 'h a velocidad horizontal o vertical del jugador
1 'm contador de número de muertes del jugador
1 'p contador de puntos del jugador

1 call &bbff:t=time:y=120:x=50:h=2:a=0:locate 1,1:?"LTA:";s:?"D:";m;"P: 0":move 10,10:draw 10,360,3:draw 630,360:draw 630,10:draw 10,10
2 if inkey(0) = 0 then a = 2:h = 0 else if inkey(2) = 0 then a = -2:h = 0 else if inkey(8) = 0 then h = -2:a = 0 else if inkey(1) = 0 then h = 2:a = 0
3 c=c+1:if c>50 then c=0:randomize time:plot(INT(RND*620)+10),(INT(RND*350)+10),1:plot x,y,2
4 frame:x=x+h:y=y+a:if test(x,y)=1 then p=p+1:locate 6,2:?"P";p:c=99:goto 3 else if test(x,y)<>0 then s=(time-t)/300:m=m+1:p=0:goto 1 else plot x,y,2:goto 2


