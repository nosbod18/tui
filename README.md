# tui

A super simple TUI, made for a roguelike side project that I decided to extract out into its own thing.

### Examples
##### Draw single character to the screen:

```c
#define TUI_IMPL
#include "tui.h"

int main(void) {
    int w, h;

    // Initialize the library and get the size of the terminal
    tui_init(&w, &h);

    // Print a yellow '@' in the center of the screen
    tui_putc(w/2, h/2, '@' | TUI_FG(255, 255, 0));

    // Must be called to flush the buffer and actually print everything to the screen
    tui_present();

    // Block until any key is pressed
    tui_read();

    // Clean up and return the screen to normal
    tui_quit();
}
```

##### Move a character around on the screen:

```c
#define TUI_IMPL
#include "tui.h"

int main(void) {
    int w, h;

    tui_init(&w, &h);

    int x = w/2;
    int y = h/2;

    // Loop until the 'q' key is pressed. This is a blocking loop that only iterates when any key is pressed
    for (int key = 0; key != 'q'; key = tui_read()) {

        // The left, down, up, and right arrow keys are returned as h, j, k, and l respectively
        // to make things a little easier and to allow for vim movements
        switch (key) {
            case 'h': x--; break;
            case 'j': y++; break;
            case 'k': y--; break;
            case 'l': x++; break;
            default:       break;
        }

        // Clear the screen with spaces
        tui_putr(0, 0, w, h, ' ');

        tui_putc(x, y, '@' | TUI_FG(255, 255, 0));
        tui_present();
    }

    tui_quit();
}
```
