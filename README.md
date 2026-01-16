# Programas y pruebas de programas Amstrad

Si quieres continuar un programa pero solo tienes el source.. te propongo lo siguiente (ejemplo: holamundo):
1. Renombra:
`mv holamundo holamundo_tmp`
2. Crea con cpctelera:
`cpct_mkproject holamundo`
3. Copia el source que tenías:
`cp -rf holamundo_tmp/* holamundo/`
4. COMPRUEBA que todo está bien y luego; Borra el temporal:
`rm -rf holamundo_tmp`