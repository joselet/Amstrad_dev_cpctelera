1 ' Versión de 1 jugador sencilla
1 '10 inicialización de pantalla, variables
1 '20 lógica del jugador
1 '30 calculo de colisiones y refresco de pantalla
1                                        80 columnas                          |
1 mode 1:y=120:x=50:vx=2:vy=0:print "http://homeputerium.de":print "muertes:";m
2 move 10,10:draw 10,360,1:draw 630,360:draw 630,10:draw 10,10
5 if inkey(0) = 0 then vy = 2:vx = 0 else if inkey(2) = 0 then vy = -2:vx = 0 
6 if inkey(8) = 0 then vx = -2:vy = 0 else if inkey(1) = 0 then vx = 2:vy = 0
8 frame:x=x+vx:y=y+vy:if test (x,y)<>0 then m=m+1:goto 1 else plot x,y,2:goto 5


