1 ' inicio
10 mode 1
20 print "Prueba de arrays"
30 input "tamanyo del array?",a
40 randomize time

1 ' dar valor a los elementos del array y mostrarlos en pantalla
100 dim v%(a)
110 print "var","valor"
120 for i=0 to a
130 v%(i) = rnd *10
140 print "v%(";i;")", v%(i)
150 next
160 print "puede probar con print v%(3)"

270 'print chr$(11)+chr$(11)
280 'print "FIN                           "


1


1 ' inicio
10 mode 1
20 print "Prueba de arrays"
30 input "tamanyo del array?",a
40 randomize time

1 ' dar valor a los elementos del array y mostrarlos en pantalla
100 dim v(a)
110 print "var","valor"
120 for i=0 to a
130 v(i) = rnd *10
140 print "v(";i;")", v(i)
150 next
160 print "puede probar con print v(3)"

270 'print chr$(11)+chr$(11)
280 'print "FIN                           "


1


1 ' inicio
10 mode 1
20 for x = 1 to 40
30 for y = 1 to 23
40 locate (x,y):   print "X"
50 next y
60 next x
70 end

1