
1 ' Versíón de 1 jugador, con galletas.
1 '1 Bienvenida e instrucciones del juego 
1 '2-3 inicialización de pantalla, variables
1 '4-5 lógica del jugador
1 '6 logica de galletas
1 '7 calculo de colisiones y refresco de posiciones de jugador en pantalla
1 '8 You found the bug!!!! (so difficult) and WON the game!
1 '9-10 cálculo de número de vidas y GAME OVER

1 'Variables:
1 't inicialización del contador de tiempo ((time-t)/300 = el tiempo que lleva el jugador vivo = a)
1 'x y coordenadas del jugador
1 'i j velocidad horizontal o vertical del jugador
1 'd contador de número de muertes del jugador
1 'p contador de puntos del jugador
1 'c contador para imprimir una nueva galleta
1                                        80 columnas                          |
1

1 MODE 1:print"Snack-e:":print:print"Hunt cookies. Don't hit walls ":CALL &BB18
2 cls:o=10:t=time:y=120:x=50:i=2:j=0:print "LTA:";a:print chr$(228);d+3;"$: ";p
3 def FNp=INT(RND*350)+o:move o,o:draw o,362,3:draw 639,362:draw 639,o:draw o,o
4 g$="Total $":c=c+1:if inkey(0)=0 then j=2:i=0 else if inkey(1)=0 then i=2:j=0
5 f$="  GAME OVER":if inkey(8)=0 then i=-2:j=0 else if inkey(2)=0 then j=-2:i=0
6 if c>99 then c=0:randomize time:plot FNp*2-8,FNp+2,1:plot x,y,2:SOUND 1,139,5
7 x=x+i:y=y+j:if test(x,y)=1 then p=p+1:locate 8,2:print p;chr$(7):c=100:goto 6
8 if x>639 then cls:print "CONGRATS,YOU FOUND A BUG AND ESCAPED THE GAME!": end
9 if test(x,y)<>0 then a=(time-t)/300:SOUND 1,99,9:d=d-1 else plot x,y,2:goto 4
10 if d<-3 then cls:print g$;p;f$: SOUND 1,0,99,,,,31:input z$: run else goto 2

