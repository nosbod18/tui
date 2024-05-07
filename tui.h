#pragma once
#include <stdint.h>

// tile:
// 00000000 0000mmmm ffffbbbb cccccccc

enum {
    TUI_BOLD       = 1 << 16,
    TUI_DIM        = 2 << 16,
    TUI_ITALIC     = 3 << 16,
    TUI_UNDERLINE  = 4 << 16,
    TUI_BLINK      = 5 << 16,
    TUI_REVERSE    = 6 << 16,
    TUI_HIDDEN     = 7 << 16,
    TUI_STRIKE     = 9 << 16,

    TUI_FG_BLACK   = 1 << 15 | 0 << 12,
    TUI_FG_RED     = 1 << 15 | 1 << 12,
    TUI_FG_GREEN   = 1 << 15 | 2 << 12,
    TUI_FG_YELLOW  = 1 << 15 | 3 << 12,
    TUI_FG_BLUE    = 1 << 15 | 4 << 12,
    TUI_FG_PURPLE  = 1 << 15 | 5 << 12,
    TUI_FG_CYAN    = 1 << 15 | 6 << 12,
    TUI_FG_WHITE   = 1 << 15 | 7 << 12,

    TUI_BG_BLACK   = 1 << 11 | 0 << 8,
    TUI_BG_RED     = 1 << 11 | 1 << 8,
    TUI_BG_GREEN   = 1 << 11 | 2 << 8,
    TUI_BG_YELLOW  = 1 << 11 | 3 << 8,
    TUI_BG_BLUE    = 1 << 11 | 4 << 8,
    TUI_BG_PURPLE  = 1 << 11 | 5 << 8,
    TUI_BG_CYAN    = 1 << 11 | 6 << 8,
    TUI_BG_WHITE   = 1 << 11 | 7 << 8,
};

void    tui_init        (int *w, int *h);
void    tui_quit        (void);
int     tui_read        (void);
void    tui_present     (void);

int     tui_attr        (uint32_t c);
int     tui_fg          (uint32_t c);
int     tui_bg          (uint32_t c);
char    tui_ch          (uint32_t c);
void    tui_set_attr    (uint32_t *c, int attr);
void    tui_set_fg      (uint32_t *c, int fg);
void    tui_set_bg      (uint32_t *c, int bg);
void    tui_set_ch      (uint32_t *c, char ch);

void    tui_putc        (int x, int y, uint32_t c);
void    tui_puts        (int x, int y, uint32_t const *s, int n);
void    tui_putf        (int x, int y, uint32_t attr, char const *f, ...);
void    tui_puth        (int x, int y, int w, uint32_t c);
void    tui_putv        (int x, int y, int h, uint32_t c);
void    tui_putr        (int x, int y, int w, int h, uint32_t c);
void    tui_putb        (int x, int y, int w, int h, uint32_t horizontal, uint32_t vertical, uint32_t corner);
void    tui_blit        (int x, int y, int w, int h, uint32_t const *s);
