1 ' INK
1 ' 0 negro
1 ' 13 blanco
1 ' 26 blanco intenso
1 ' 3 rojo

10 mode 1:border 13
20 ink 0,0
21 ink 1,13
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

300 paper 1
310 gosub 410
320 paper 2
330 gosub 410
340 paper 3
350 gosub 410
360 paper 0:pen 1

400 end

410 pen 0:print chr$(207);
420 pen 1:print chr$(143);
430 pen 1:print chr$(207);
440 pen 2:print chr$(207);
450 return


