1 ' INK
1 ' 0 negro
1 ' 13 blanco
1 ' 26 blanco intenso
1 ' 3 rojo

10 mode 1:border 13
20 ink 0,0
21 ink 1,1
22 ink 2,26
23 ink 3,3

200 for p = 0 to 3
210 pen p:paper p+1
220 print "agua    " ; chr$(128)
230 print "arena   " ; chr$(143)
240 print "fino    " ; chr$(207)
250 print "rugoso  " ; chr$(206)
260 print "rugoso2 " ; chr$(127)
270 next

