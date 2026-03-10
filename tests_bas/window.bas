10 MODE 1:BORDER 0
40 PAPER 0:CLS
50 PAPER #1,1:WINDOW #1,1,10,6,15:CLS #1
60 PAPER #2,2:WINDOW #2,11,20,6,15:CLS #2
70 PAPER #3,3:WINDOW #3,5,10,16,20:CLS #3
80 print #1,"1"
90 print #2,"2"
100 print #3,"3"
110 print "tecla.."
120 CALL &BB18
130 print #2,"adios"
140 window #2,15,20,17,21:print #2,"volvi"

