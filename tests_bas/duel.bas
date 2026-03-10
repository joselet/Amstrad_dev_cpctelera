10 MODE 1:PRINT "COWBOY DUEL":PRINT "P1: [SPACE]  P2: [ENTER]":t=RND*500+500
20 PRINT "READY...":FOR i=1 TO t:GOSUB 100:NEXT
30 PRINT "STEADY...":FOR j=1 TO t:GOSUB 100:NEXT
40 PRINT "GO!!":t1=TIME
50 if inkey(8)=0 THEN PRINT "P1 WINS!":call &bb03:CALL &BB18:run
60 if inkey(1)=0 THEN PRINT "P2 WINS!":call &bb03:CALL &BB18:run
70 GOTO 50
100 if inkey(8)=0 THEN PRINT "P1 SHOT EARLY! P2 WINS":call &bb03:CALL &BB18:run
110 if inkey(1)=0 THEN PRINT "P2 SHOT EARLY! P1 WINS":call &bb03:CALL &BB18:run
120 RETURN




10 MODE 1:PRINT "COWBOY DUEL":PRINT "P1: [SPACE]  P2: [ENTER]":t=RND*500+500:def FNa=(inkey(8)=0):def FNb=(inkey(1)=0)
20 PRINT "READY...":FOR i=1 TO t:GOSUB 100:NEXT
30 PRINT "STEADY...":FOR j=1 TO t:GOSUB 100:NEXT
40 PRINT "GO!!":t1=TIME
50 if FNa THEN PRINT "P1 WINS!":call &bb03:CALL &BB18:run
60 if FNb THEN PRINT "P2 WINS!":call &bb03:CALL &BB18:run
70 GOTO 50
100 if FNa THEN PRINT "P1 SHOT EARLY! P2 WINS":call &bb03:CALL &BB18:run
110 if FNb THEN PRINT "P2 SHOT EARLY! P1 WINS":call &bb03:CALL &BB18:run
120 RETURN


10 def FNa=(inkey(8)=0):def FNb=(inkey(1)=0):MODE 1:? "READY...":GOSUB 50:? "STEADY...":GOSUB 50:? "FIRE!!"
20 if FNa THEN ? "P1 WINS!":GOTO 40
30 if FNb THEN ? "P2 WINS!":GOTO 40 ELSE 20
40 CALL &BB03:CALL &BB18:RUN
50 FOR i=1 TO RND*500+500:if FNa THEN ? "EARLY SHOT! P1":GOTO 40 else if FNb then? "EARLY SHOT! P2":GOTO 40
60 NEXT:RETURN


ideas:
aleatoriamente al ser go, cada jugador tiene una tecla aleatoria para hacer diana sobre el otro.