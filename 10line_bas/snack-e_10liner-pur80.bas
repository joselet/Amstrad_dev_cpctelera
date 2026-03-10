
1 ' Snack-e: 1-Player version, with cookies.
1 ' Source code description:
1 ' 1 Welcome and game instructions
1 ' 2-3 Screen and variable initialization
1 ' 4-5 Player logic
1 ' 6 Cookie logic
1 ' 7 Collision detection, scoring, and player position screen refresh
1 ' 8 You found the bug!!!! (so difficult) and WON the game!
1 ' 9-10 Collision detection (death), life counter, and GAME OVER
1 ' Variables:
1 ' t Time counter initialization ((time-t)/300 = player survival time = a) (in-game:LTA): Last time alive
1 ' x y Player coordinates
1 ' i j Player horizontal or vertical speed
1 ' d Player death counter
1 ' p Player score counter
1 ' c Counter for spawning a new cookie
1 ' 
1 ' Other details:
1 ' In line 3, I defined a function FNp to allow random cookie placement in line 6, freeing up enough space to add a sound effect.
1 ' In lines 4 and 5, I defined variables g$ and f$ to ensure enough space in line 10 for the final message.
1 ' In line 4, the cookie counter is incremented; while this logic belongs in line 6, it was moved here to keep the code within the 80-column limit.
1 ' Throughout the program, I have avoided using the "?" abbreviation for the PRINT command. I wanted to stay true to the Amstrad CPC experience, ensuring the listing remains within 80 columns when displayed.
1 ' 
1 ' Loading and verification instructions:
1 ' Load the snack-e_10liner-pur80.dsk disk into the emulator
1 ' load "snack-e"
1 ' To view the listing adjusted to 80 columns:
1 ' mode 2:list
1 ' To run the game:
1 ' run


1 ' Versíón de 1 jugador, con galletas.
1 '1 Bienvenida e instrucciones del juego 
1 '2-3 inicialización de pantalla, variables
1 '4-5 lógica del jugador
1 '6 logica de galletas
1 '7 calculo de colisiones con puntos y refresco de posiciones de jugador en pantalla
1 '8 You found the bug!!!! (so difficult) and WON the game!
1 '9-10 cálculo de colisiones con muerte número de vidas y GAME OVER

1 'Variables:
1 't inicialización del contador de tiempo ((time-t)/300 = el tiempo que lleva el jugador vivo = a) (en el juego: LTA): tiempo vivo la vez anterior
1 'x y coordenadas del jugador
1 'i j velocidad horizontal o vertical del jugador
1 'd contador de número de muertes del jugador
1 'p contador de puntos del jugador
1 'c contador para imprimir una nueva galleta

1 'Otros detalles:
1 'En la linea 3 he definido una función FNp para que así en la linea 6 pueda colocar galletas aleatoriamente en la pantalla dándome espacio suficiente para añadir un sonido.
1 'En las lineas 4 y 5 he definido las variables g$ y f$ para que así tenga espacio suficiente en la línea 10 para mostrar el mensaje
1 'En la línea 4 se aumenta el contador de galletas, le correspondería estar esta suma en la línea 6, pero entonces no cabría el contenido del código en 80 columnas.
1 'En todo el programa hemos prescindido del uso de la abreviatura "?" que sustituye al comando PRINT. He querido ser fiel a que en un Amstrad CPC se pueda mostrar el listado y que respete igualmente el máximo de 80 columnas.
1 '
1 'Instrucciones de carga y comprobación:
1 'Cargar el disco snack-e_10liner-pur80.dsk en el emulador
1 'load "snack-e"
1 'Para ver el listado ajustado a 80 columnas:
1 'mode 2:list
1 'Para ejecutar el juego:
1 'run


1 'código fuente el programa:
1                                        80 columnas                          |

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

