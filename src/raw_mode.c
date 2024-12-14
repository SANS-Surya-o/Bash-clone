#include "shell.h"
#include "raw_mode.h"

/* Store original terminal attributes */
struct termios orig_termios;

/* Function to disable raw mode (restore original terminal attributes) */
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

/* Function to enable raw mode */
void enable_raw_mode() {
    struct termios raw;

    /* Get current terminal attributes */
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    /* Ensure original attributes are restored when the shell exits */
    atexit(disable_raw_mode);

    /* Copy original attributes to modify */
    raw = orig_termios;

    /* Modify the terminal attributes to enable raw mode */
    raw.c_lflag &= ~(ECHO | ICANON | ISIG); // Disable echo, canonical mode, extended input, and signal handling (e.g., Ctrl+C, Ctrl+Z)


    /* Set the modified terminal attributes */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}