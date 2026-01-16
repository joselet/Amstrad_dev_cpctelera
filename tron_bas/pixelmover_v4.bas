1 ' velocidad 2 que es la resolucion en modo 1.
1 ' si pones velocidad 4, asegúrate que las coordenadas de inicio son divisibles/4 para que coincidan si hay colision
1 ' aqui se redibuja con draw para que no se vea a puntitos si sube la velocidad.
10 MODE 1 : border 0
20 INK 3,6 : ' jugador 1 rojo
25 INK 2,20 : ' Jugador 2 cyan
30 INK 1,24 : ' obstaculos
40 CLS : REM Neteja la pantalla amb el color de fons (Tinta 0, normalment Blau/1)
45 obstaculo = 1 : REM Codi de tinta del obstaculo
46 vel = 2 : ' velocidad de las motos (lo mejor es dos para resolucion en modo 1)
50 ' Dibuixa una paret Groga (usant Tinta 1, codi de color 24)
60 MOVE 100,50 
70 DRAW 550,50,1 : DRAW 550,300,1 : DRAW 100,300,1 : DRAW 100,50,1 : REM Dibuixa un rectangle amb Tinta 1 (groc) [9]
71 ' obstaculos aleatorios locate 7,7:print 0:locate 30,7:print 1:locate 7,20:print 2:locate 30,20:print 3
80 X1 = 320 : Y1 = 220 : REM Coordenades inicials al centre (la pantalla és 640x400) 
81 X2 = 300 : Y2 = 180 : REM Coordenades inicials al centre (la pantalla és 640x400) 
85 VX1= vel:VY1=0 : ' Velocidad inicial
86 VX2=-vel:VY2=0 : ' Velocidad inicial
90 PLOT X1, Y1, 3 : ' inicial p1
91 PLOT X2, Y2, 2 : ' inicial p2

100 REM Bucle de Joc Principal

110 ' Moviment del cursor (els valors per a n es basen en la numeració de joysticks/tecles)
120 while inkey$<>""
130 IF INKEY(0) <> -1 THEN VY1 =  vel : VX1=0 : REM Amunt (si n=0 està premut) 
140 IF INKEY(2) <> -1 THEN VY1 = -vel : VX1=0 : REM Avall (si n=2 està premut) 
150 IF INKEY(8) <> -1 THEN VX1 = -vel : VY1=0 : REM Esquerra (si n=8 està premut)
160 IF INKEY(1) <> -1 THEN VX1 =  vel : VY1=0 : REM Dreta (si n=1 està premut)
170 IF INKEY(59) <> -1 THEN VY2=  vel : VX2=0 : 'W
180 IF INKEY(60) <> -1 THEN VY2= -vel : VX2=0 : 'S
190 IF INKEY(69) <> -1 THEN VX2= -vel : VY2=0 : 'A
200 IF INKEY(61) <> -1 THEN VX2=  vel : VY2=0 : 'D
201 wend
202 TX1= X1 + VX1: TY1= Y1+VY1
203 TX2= X2 + VX2: TY2= Y2+VY2
204 if TX1 < 0 then TX1= 639
205 if TX1 > 639 then TX1= 0
206 if TX2 < 0 then TX2= 639
207 if TX2 > 639 then TX2= 0
208 if TY1 < 0 then TY1= 399
209 if TY1 > 399 then TY1= 0
210 if TY2 < 0 then TY2= 399
211 if TY2 > 399 then TY2= 0
215 chkcolor1 = TEST(TX1, TY1) : REM Obté el codi de color de tinta pixel de destinació
216 chkcolor2 = TEST(TX2, TY2) : REM Obté el codi de color de tinta pixel de destinació
219 if tx1=tx2 and ty1=ty2 then print "los dos mueren":end
220 IF chkcolor1 <> 0 THEN print "p1 muere" : end:'p1 muere 
221 IF chkcolor2 <> 0 THEN print "p2 muere" : end:'p2 muere 
230 ' Detecció general de píxels existents (per si no vol travessar cap píxel dibuixat)
240 ' IF chkcolor <> 0 THEN GOTO 100 : REM Si xoca amb un altre píxel (no fons)
250 ' Moviment segur
270 move x1,y1:X1 = TX1 : Y1 = TY1 : REM Actualitza les coordenades
271 draw X1, Y1, 3 : REM Dibuixa el nou punt vermell (Tinta 3)
280 move x2,y2:X2 = TX2 : Y2 = TY2 : REM Actualitza les coordenades
281 draw X2, Y2, 2 : REM Dibuixa el nou punt vermell (Tinta 3)
290 GOTO 100 : REM Repeteix el bucle
1 ' extra, obstaculos y salidas aleatorias
282 if round(rnd*9)=0 then locate round(rnd*39)+1,round(rnd*23)+1:print"#"
283 if round(rnd*9)=0 then locate round(rnd*39)+1,round(rnd*23)+1:pen 0:print"#":pen 1

1 'pantalla en modo 1
500 MOVE 0,0 
510 DRAW 639,0,1 : DRAW 639,399,1 : DRAW 0,399,1 : DRAW 0,0,1 : REM Dibuixa un rectangle amb Tinta 1 (groc)


