// ---------------------------------------------------------------------
// main.c - Herramienta de pruebas: pide por teclado el parametro "cmd"
// a enviar, hace la peticion via m4_get, y muestra codigo de estado
// HTTP + cuerpo de la respuesta. Bucle infinito.
//
// Lectura de teclado: usamos la API propia de CPCtelera
// (cpct_scanKeyboard / cpct_isKeyPressed) en vez de la rutina de
// firmware KM WAIT CHAR (&BB18), porque esta ultima depende del
// escaneo de teclado por interrupcion del firmware, y CPCtelera
// sustituye el gestor de interrupciones por el suyo propio - por eso
// &BB18 devolvia basura/valores congelados.
// ---------------------------------------------------------------------
#include <cpctelera.h>
#include <string.h>

#define M4HOST   "nosoft.es"
#define M4PATH   "/services/ESP8266BBS/games/empire/core/empire.php"
#define BUFSIZE  2048   // tamano del buffer de respuesta HTTP
#define LINEMAX  80     // longitud maxima de una linea de entrada
#define PATHMAX  200    // longitud maxima de la ruta+query construida

// -----------------------------------------------------------------
// Funciones que viven en el modulo ensamblador (m4net.s) - sin cambios
// -----------------------------------------------------------------
extern void fw_print_char(unsigned char c);
extern unsigned int m4_get(const char *host, const char *path,
                            unsigned char *dst, unsigned int bufsize);

// -----------------------------------------------------------------
// Buffers globales
// -----------------------------------------------------------------
unsigned char httpbuf[BUFSIZE];
char pathbuf[PATHMAX];
char inputline[LINEMAX + 1];

// -----------------------------------------------------------------
// Utilidades de impresion
// -----------------------------------------------------------------

static void fw_print_str(const char *s)
{
    while (*s) fw_print_char((unsigned char)*s++);
}

static void print_uint16(unsigned int v)
{
    char buf[6];
    unsigned char i = 0;

    if (v == 0) { fw_print_char('0'); return; }

    while (v > 0 && i < 5) {
        buf[i++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while (i > 0) fw_print_char(buf[--i]);
}

// -----------------------------------------------------------------
// Teclado (API nativa de CPCtelera, sin depender del firmware)
// -----------------------------------------------------------------

typedef struct {
    cpct_keyID    key;
    char          lower;
    char          upper;
    unsigned char wasDown;   // para detectar flanco de bajada->pulsado
} KeyMapEntry;

static KeyMapEntry keymap[] = {
    { Key_A,'a','A',0 }, { Key_B,'b','B',0 }, { Key_C,'c','C',0 }, { Key_D,'d','D',0 },
    { Key_E,'e','E',0 }, { Key_F,'f','F',0 }, { Key_G,'g','G',0 }, { Key_H,'h','H',0 },
    { Key_I,'i','I',0 }, { Key_J,'j','J',0 }, { Key_K,'k','K',0 }, { Key_L,'l','L',0 },
    { Key_M,'m','M',0 }, { Key_N,'n','N',0 }, { Key_O,'o','O',0 }, { Key_P,'p','P',0 },
    { Key_Q,'q','Q',0 }, { Key_R,'r','R',0 }, { Key_S,'s','S',0 }, { Key_T,'t','T',0 },
    { Key_U,'u','U',0 }, { Key_V,'v','V',0 }, { Key_W,'w','W',0 }, { Key_X,'x','X',0 },
    { Key_Y,'y','Y',0 }, { Key_Z,'z','Z',0 },
    { Key_0,'0','0',0 }, { Key_1,'1','1',0 }, { Key_2,'2','2',0 }, { Key_3,'3','3',0 },
    { Key_4,'4','4',0 }, { Key_5,'5','5',0 }, { Key_6,'6','6',0 }, { Key_7,'7','7',0 },
    { Key_8,'8','8',0 }, { Key_9,'9','9',0 },
    { Key_Space,  ' ', ' ', 0 },
    { Key_Dot,    '.', ':', 0 },
    { Key_Comma,  ',', ';', 0 },
    { Key_Hyphen, '-', '_', 0 },
    { Key_Return, 13,  13,  0 },   // tecla RETURN principal
    { Key_Enter,  13,  13,  0 },   // ENTER del teclado numerico
    { Key_Del,    127, 127, 0 },   // borrar
};
#define KEYMAP_COUNT (sizeof(keymap) / sizeof(keymap[0]))

// Espera a que se pulse una tecla (flanco de subida) y devuelve su
// codigo ASCII (mayuscula si SHIFT esta pulsado). Bloqueante.
static unsigned char wait_key(void)
{
    unsigned char i, shifted, down;

    for (;;) {
        cpct_scanKeyboard();
        shifted = cpct_isKeyPressed(Key_Shift);

        for (i = 0; i < KEYMAP_COUNT; i++) {
            down = cpct_isKeyPressed(keymap[i].key);
            if (down && !keymap[i].wasDown) {
                keymap[i].wasDown = down;
                return shifted ? (unsigned char)keymap[i].upper
                                : (unsigned char)keymap[i].lower;
            }
            keymap[i].wasDown = down;
        }
    }
}

// Lee una linea de teclado con eco por pantalla, terminada en ENTER.
static void read_line(char *dst, unsigned char maxlen)
{
    unsigned char n = 0;
    unsigned char k;

    for (;;) {
        k = wait_key();

        if (k == 13) break;                    // ENTER

        if (k == 127 && n > 0) {                // DEL / backspace
            n--;
            fw_print_char(8);
            fw_print_char(' ');
            fw_print_char(8);
            continue;
        }

        if (k >= 32 && n < maxlen) {
            dst[n++] = (char)k;
            fw_print_char(k);
        }
    }

    dst[n] = 0;
    fw_print_char(13);
    fw_print_char(10);
}

// -----------------------------------------------------------------
// Parsing de la respuesta HTTP
// -----------------------------------------------------------------

static unsigned int skip_headers(unsigned char *buf, unsigned int n)
{
    unsigned int i;

    for (i = 0; i + 3 < n; i++) {
        if (buf[i] == 13 && buf[i+1] == 10 && buf[i+2] == 13 && buf[i+3] == 10) {
            return i + 4;
        }
    }
    return 0;
}

static unsigned int parse_response(unsigned char *buf, unsigned int n,
                                    unsigned int *body_start)
{
    unsigned int i, status;

    *body_start = skip_headers(buf, n);

    status = 0;
    for (i = 0; i < n; i++) {
        if (buf[i] == ' ') {
            if (i + 3 < n &&
                buf[i+1] >= '0' && buf[i+1] <= '9' &&
                buf[i+2] >= '0' && buf[i+2] <= '9' &&
                buf[i+3] >= '0' && buf[i+3] <= '9') {
                status = (unsigned int)(buf[i+1] - '0') * 100
                        + (unsigned int)(buf[i+2] - '0') * 10
                        + (unsigned int)(buf[i+3] - '0');
            }
            break;
        }
    }
    return status;
}

// -----------------------------------------------------------------
// Programa principal: bucle de pruebas manual
// -----------------------------------------------------------------
void main(void)
{
    unsigned int n, i, body, status;
    unsigned char k;

    cpct_setVideoMode(1);
    cpct_setPALColour(0, 0x00);
    cpct_setPALColour(1, 0x1A);
    cpct_setPALColour(2, 0x0D);
    cpct_setPALColour(3, 0x06);
    cpct_setBorder(0x00);

    for (;;) {
        fw_print_str("\r\ncmd> ");
        read_line(inputline, LINEMAX);

        strcpy(pathbuf, M4PATH);
        strcat(pathbuf, "?cmd=");
        strcat(pathbuf, inputline);

        n = m4_get(M4HOST, pathbuf, httpbuf, BUFSIZE);

        if (n == 0xFFFF) {
            fw_print_str("ERROR DE RED\r\n");
            continue;
        }

        status = parse_response(httpbuf, n, &body);

        fw_print_str("HTTP ");
        print_uint16(status);
        fw_print_str(" (");
        print_uint16(n);
        fw_print_str(" bytes)\r\n");

        for (i = body; i < BUFSIZE && i < n; i++) {
            k = httpbuf[i];
            if (k == '~') break;
            if (k < 32 || k > 126) continue;   // seguimos sin tocar ANSI todavia
            fw_print_char(k);
        }

        fw_print_str("\r\n---FIN---\r\n");
    }
}