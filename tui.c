#include "tui.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

struct {
    struct termios raw, old;
    uint32_t *tiles[2];
    int w, h;
} _tui = {0};

void tui_init(int *w, int *h) {
    tcgetattr(STDIN_FILENO, &_tui.old);
    _tui.raw = _tui.old;
    _tui.raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    _tui.raw.c_oflag &= ~(OPOST);
    _tui.raw.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
    _tui.raw.c_cc[VMIN] = 1;
    _tui.raw.c_cc[VTIME] = 0;

#ifndef DEBUG
    tcsetattr(STDIN_FILENO, TCSANOW, &_tui.raw);
    write(STDOUT_FILENO, "\x1b[?25l\x1b[?1049h", 14);
#endif

    struct winsize winsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);

    _tui.w = winsize.ws_col;
    _tui.h = winsize.ws_row;
    _tui.tiles[0] = malloc(_tui.w * _tui.h * sizeof *_tui.tiles[0]);
    _tui.tiles[1] = malloc(_tui.w * _tui.h * sizeof *_tui.tiles[1]);

    for (int i = 0; i < _tui.w * _tui.h; i++)
        _tui.tiles[0][i] = _tui.tiles[1][i] = ' ';

    if (w) *w = _tui.w;
    if (h) *h = _tui.h;
}

void tui_quit(void) {
    free(_tui.tiles[0]);
    free(_tui.tiles[1]);

#ifndef DEBUG
    tcsetattr(STDIN_FILENO, TCSANOW, &_tui.old);
    write(STDOUT_FILENO, "\x1b[?25h\x1b[?1049l", 14);
#endif
}

int tui_read(void) {
    char seq[3] = {0};
    int n = read(STDIN_FILENO, seq, 3);

    if (n == 3) {
        switch (seq[2]) {
            case 'A': return 'k';
            case 'B': return 'j';
            case 'C': return 'l';
            case 'D': return 'h';
            default:  break;
        }
    }

    return seq[0];
}

void tui_present(void) {
    if (memcmp(_tui.tiles[0], _tui.tiles[1], _tui.w * _tui.h * sizeof *_tui.tiles[0]) == 0)
        return;

    char *line = malloc((_tui.w * 11) + 8);

    for (int y = 0; y < _tui.h; y++) {
        if (memcmp(&_tui.tiles[0][y * _tui.w], &_tui.tiles[1][y * _tui.w], _tui.w * sizeof *_tui.tiles[0]) == 0)
            continue;

        char *ptr = line;

        int n = snprintf(NULL, 0, "\x1b[%d;1H", y + 1);
        ptr += snprintf(ptr, n + 1, "\x1b[%d;1H", y + 1);

        for (int x = 0; x < _tui.w; x++) {
            uint32_t c = _tui.tiles[0][x + y * _tui.w];
            ptr += snprintf(ptr, 11 + 1, "\x1b[%u;3%u;4%um%c",
                    (c >> 16) & 15,
                    (c >> 15) & 1 ? (c >> 12) & 7 : 9,
                    (c >> 11) & 1 ? (c >>  8) & 7 : 9,
                     c & 255
            );
        }

        write(STDOUT_FILENO, line, ptr - line);
        memcpy(&_tui.tiles[1][y * _tui.w], &_tui.tiles[0][y * _tui.w], _tui.w * sizeof *_tui.tiles[1]);
    }

    free(line);
}

void tui_putc(int x, int y, uint32_t c) {
    if (x >= 0 && y >= 0 && x < _tui.w && y < _tui.h)
        _tui.tiles[0][x + y * _tui.w] = c;
}

void tui_puts(int x, int y, uint32_t const *s, int n) {
    for (int i = 0; i < n; i++)
        tui_putc(x + i, y, s[i]);
}

void tui_putf(int x, int y, uint32_t attr, char const *f, ...) {
    va_list ap;
    va_start(ap, f);

    int n = vsnprintf(NULL, 0, f, ap);
    char *buf = malloc(n + 1);
    vsnprintf(buf, n + 1, f, ap);

    for (int i = 0; i < n; i++)
        tui_putc(x + i, y, buf[i] | attr);

    free(buf);
}

void tui_puth(int x, int y, int w, uint32_t c) {
    for (int xx = 0; xx < w; xx++)
        tui_putc(x + xx, y, c);
}

void tui_putv(int x, int y, int h, uint32_t c) {
    for (int yy = 0; yy < h; yy++)
        tui_putc(x, y + yy, c);
}

void tui_putr(int x, int y, int w, int h, uint32_t c) {
    for (int yy = 0; yy < h; yy++)
        for (int xx = 0; xx < w; xx++)
            tui_putc(x + xx, y + yy, c);
}

void tui_putb(int x, int y, int w, int h, uint32_t horizontal, uint32_t vertical, uint32_t corner) {
    tui_putc(x,     y,     corner);
    tui_putc(x + w, y,     corner);
    tui_putc(x + w, y + h, corner);
    tui_putc(x,     y + h, corner);

    tui_puth(x + 1, y,     w - 1, horizontal);
    tui_puth(x + 1, y + h, w - 1, horizontal);

    tui_putv(x,     y + 1, h - 1, vertical);
    tui_putv(x + w, y + 1, h - 1, vertical);
}

void tui_blit(int x, int y, int w, int h, uint32_t const *s) {
    for (int yy = 0; yy < h; yy++)
        for (int xx = 0; xx < w; xx++)
            tui_putc(x + xx, y + yy, s[xx + yy * w]);
}
