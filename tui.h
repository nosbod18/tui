////////////////////////////////////////////////////////////////////////////////////////////////////
//                   _____ _   _ _______ ______ _____  ______      _____ ______                   //
//                  |_   _| \ | |__   __|  ____|  __ \|  ____/\   / ____|  ____|                  //
//                    | | |  \| |  | |  | |__  | |__) | |__ /  \ | |    | |__                     //
//                    | | | . ` |  | |  |  __| |  _  /|  __/ /\ \| |    |  __|                    //
//                   _| |_| |\  |  | |  | |____| | \ \| | / ____ \ |____| |____                   //
//                  |_____|_| \_|  |_|  |______|_|  \_\_|/_/    \_\_____|______|                  //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <stdint.h>

#define     TUI_BG(r, g, b) ((UINT64_C(1) << 57) | ((UINT64_C((r) & 0xff)) << 48) | ((UINT64_C((g) & 0xff)) << 40) | ((UINT64_C((b) & 0xff)) << 32))
#define     TUI_FG(r, g, b) ((UINT64_C(1) << 56) | ((UINT64_C((r) & 0xff)) << 24) | ((UINT64_C((g) & 0xff)) << 16) | ((UINT64_C((b) & 0xff)) <<  8))

void        tui_init        (int *w, int *h);
void        tui_quit        (void);
int         tui_read        (void);
void        tui_present     (void);

void        tui_putc        (int x, int y, uint64_t c);
void        tui_puts        (int x, int y, uint64_t const *s, int n);
void        tui_putf        (int x, int y, uint64_t attr, char const *f, ...);
void        tui_puth        (int x, int y, int w, uint64_t c);
void        tui_putv        (int x, int y, int h, uint64_t c);
void        tui_putr        (int x, int y, int w, int h, uint64_t c);
void        tui_putb        (int x, int y, int w, int h, uint64_t horizontal, uint64_t vertical, uint64_t corner);
void        tui_blit        (int x, int y, int w, int h, uint64_t const *s);

////////////////////////////////////////////////////////////////////////////////////////////////////
//   _____ __  __ _____  _      ______ __  __ ______ _   _ _______    _______ _____ ____  _   _   //
//  |_   _|  \/  |  __ \| |    |  ____|  \/  |  ____| \ | |__   __|/\|__   __|_   _/ __ \| \ | |  //
//    | | | \  / | |__) | |    | |__  | \  / | |__  |  \| |  | |  /  \  | |    | || |  | |  \| |  //
//    | | | |\/| |  ___/| |    |  __| | |\/| |  __| | . ` |  | | / /\ \ | |    | || |  | | . ` |  //
//   _| |_| |  | | |    | |____| |____| |  | | |____| |\  |  | |/ ____ \| |   _| || |__| | |\  |  //
//  |_____|_|  |_|_|    |______|______|_|  |_|______|_| \_|  |_/_/    \_\_|  |_____\____/|_| \_|  //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(TUI_IMPL)

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
    uint64_t *tiles[2];
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

    struct winsize winsize;

    tcsetattr(STDIN_FILENO, TCSANOW, &_tui.raw);
    write(STDOUT_FILENO, "\x1b[?25l\x1b[?1049h", 14);
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

    tcsetattr(STDIN_FILENO, TCSANOW, &_tui.old);
    write(STDOUT_FILENO, "\x1b[?25h\x1b[?1049l", 14);
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

    char *line = malloc((_tui.w * 37) + 8);

    for (int y = 0; y < _tui.h; y++) {
        if (memcmp(&_tui.tiles[0][y * _tui.w], &_tui.tiles[1][y * _tui.w], _tui.w * sizeof *_tui.tiles[0]) == 0)
            continue;

        char *ptr = line;
        ptr += snprintf(ptr, 8 + 1, "\x1b[%d;1H", y + 1);

        // 000000bf rrrrrrrr gggggggg bbbbbbbb rrrrrrrr gggggggg bbbbbbbb ccccccccc
        for (int x = 0; x < _tui.w; x++) {
            uint64_t c = _tui.tiles[0][x + y * _tui.w];

            *ptr++ = '\x1b';
            *ptr++ = '[';

            if ((c >> 56) & 3) {
                // background
                if ((c >> 57) & 1)
                    ptr += snprintf(ptr, 16 + 1, "48;2;%d;%d;%d", (int)(c >> 48) & 0xff, (int)(c >> 40) & 0xff, (int)(c >> 32) & 0xff);

                // foreground
                if ((c >> 56) & 1)
                    ptr += snprintf(ptr, 17 + 1, "%c38;2;%d;%d;%d", ((c >> 57) & 1) ? ';' : 0, (int)(c >> 24) & 0xff, (int)(c >> 16) & 0xff, (int)(c >> 8) & 0xff);
            } else {
                *ptr++ = '0';
            }

            *ptr++ = 'm';
            *ptr++ = c & 0xff;
        }

        write(STDOUT_FILENO, line, ptr - line);
        memcpy(&_tui.tiles[1][y * _tui.w], &_tui.tiles[0][y * _tui.w], _tui.w * sizeof *_tui.tiles[1]);
    }

    free(line);
}

void tui_putc(int x, int y, uint64_t c) {
    if (x >= 0 && y >= 0 && x < _tui.w && y < _tui.h)
        _tui.tiles[0][x + y * _tui.w] = c;
}

void tui_puts(int x, int y, uint64_t const *s, int n) {
    for (int i = 0; i < n; i++)
        tui_putc(x + i, y, s[i]);
}

void tui_putf(int x, int y, uint64_t attr, char const *f, ...) {
    va_list ap;
    va_start(ap, f);

    int n = vsnprintf(NULL, 0, f, ap);
    char *buf = malloc(n + 1);
    vsnprintf(buf, n + 1, f, ap);

    for (int i = 0; i < n; i++)
        tui_putc(x + i, y, buf[i] | attr);

    free(buf);
}

void tui_puth(int x, int y, int w, uint64_t c) {
    for (int xx = 0; xx < w; xx++)
        tui_putc(x + xx, y, c);
}

void tui_putv(int x, int y, int h, uint64_t c) {
    for (int yy = 0; yy < h; yy++)
        tui_putc(x, y + yy, c);
}

void tui_putr(int x, int y, int w, int h, uint64_t c) {
    for (int yy = 0; yy < h; yy++)
        for (int xx = 0; xx < w; xx++)
            tui_putc(x + xx, y + yy, c);
}

void tui_putb(int x, int y, int w, int h, uint64_t horizontal, uint64_t vertical, uint64_t corner) {
    tui_putc(x,     y,     corner);
    tui_putc(x + w, y,     corner);
    tui_putc(x + w, y + h, corner);
    tui_putc(x,     y + h, corner);

    tui_puth(x + 1, y,     w - 1, horizontal);
    tui_puth(x + 1, y + h, w - 1, horizontal);

    tui_putv(x,     y + 1, h - 1, vertical);
    tui_putv(x + w, y + 1, h - 1, vertical);
}

void tui_blit(int x, int y, int w, int h, uint64_t const *s) {
    for (int yy = 0; yy < h; yy++)
        for (int xx = 0; xx < w; xx++)
            tui_putc(x + xx, y + yy, s[xx + yy * w]);
}

#endif // TUI_IMPL
