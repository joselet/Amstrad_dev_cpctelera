1 ' IF INKEY(0) <> -1 THEN VY1 =  vel : VX1=0 : REM Amunt (si n=0 està premut) 
1 ' IF INKEY(2) <> -1 THEN VY1 = -vel : VX1=0 : REM Avall (si n=2 està premut) 
1 ' IF INKEY(8) <> -1 THEN VX1 = -vel : VY1=0 : REM Esquerra (si n=8 està premut)
1 ' IF INKEY(1) <> -1 THEN VX1 =  vel : VY1=0 : REM Dreta (


10 MODE 1:y=120:x=50:vx=2:vy=0:print "http://homeputerium.de":print "muertes:";m
20 if inkey(0) = 0 then vy = 2:vx = 0 else if inkey(2) = 0 then vy = -2:vx = 0 else if inkey(8) = 0 then vx = -2:vy = 0 else if inkey(1) = 0 then vx = 2:vy = 0
30 frame:x=x+vx:y=y+vy:if test (x,y)<>0 then m=m+1:goto 10 else plot x,y,1:goto 20

