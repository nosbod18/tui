# tui

A super simple TUI, made for a roguelike side project that I decided to extract out into its own thing.

### Example
```c
#include "tui.h"

int main(void) {
    int w, h;
    tui_init(&w, &h);                           // Initialize the library and get the size of the terminal

    tui_putr(0, 0, w, h, '.');                  // Fill the screen with '.'
    tui_putc(w/2, h/2, '@' | TUI_FG_YELLOW);    // Print a yellow '@' in the center of the screen

    tui_read();                                 // Block until any key is pressed
    tui_quit();                                 // Clean up and return the screen to normal
}
```
