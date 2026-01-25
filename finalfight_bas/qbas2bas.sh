#!/bin/sh

# V1.1 elimina lineas rem (') y lineas vacías

INPUT="$1"
OUTPUT="$2"
STEP=10

if [ -z "$INPUT" ] || [ -z "$OUTPUT" ]; then
    echo "Uso: $0 entrada.bas salida.bas"
    exit 1
fi

awk -v step="$STEP" '
BEGIN { line = step }


# Quitar comentarios al final de línea
{
    sub(/'\''.*/, "", $0)
}

# Ignorar líneas vacías tras quitar comentarios
/^[ \t]*$/ {
    next
}

# PASADA 1: detectar etiquetas
/^[ \t]*[A-Za-z_][A-Za-z0-9_]*:[ \t]*$/ {
    label = $0
    sub(/^[ \t]*/, "", label)
    sub(/:[ \t]*$/, "", label)
    labels[tolower(label)] = line
    next
}

# Guardar líneas de código
/./ {
    code[line] = $0
    order[++count] = line
    line += step
}

END {
    for (i = 1; i <= count; i++) {
        ln = order[i]
        txt = code[ln]

        # Dividir por ":" para múltiples instrucciones
        n = split(txt, parts, ":")
        for (j = 1; j <= n; j++) {
            instr = parts[j]
            gsub(/^[ \t]+|[ \t]+$/, "", instr)
            low = tolower(instr)

            # Reemplazar GOTO / GOSUB
            for (l in labels) {
                gsub("goto[ \t]+" l, "GOTO " labels[l], low)
                gsub("gosub[ \t]+" l, "GOSUB " labels[l], low)
            }

            # Reemplazar IF ... THEN etiqueta
            # Detecta "if ... then etiqueta" al final de la instrucción
            if (match(low, /^[ \t]*if[ \t].*[ \t]+then[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*$/)) {
                split(low, arr, /[ \t]+/)
                last = arr[length(arr)]
                if (last in labels) {
                    sub(last "$", labels[last], low)
                }
            }

            parts[j] = toupper(low)
        }

        # Reconstruir línea
        final_line = parts[1]
        for (j = 2; j <= n; j++) final_line = final_line ":" parts[j]

        print ln " " final_line
    }
}
' "$INPUT" > "$OUTPUT"
