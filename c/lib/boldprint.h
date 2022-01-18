#ifndef BOLDPRINT_H
#define BOLDPRINT_H

#ifndef NDEBUG

// If "tty", then go bold; otherwise do nothing.
#define TEXTBOLD(tty) ((tty) ? "\033[1m" : "")

// If "tty", then go normal; otherwise do nothing.
#define TEXTNORMAL(tty) ((tty) ? "\033[0m" : "")

#else

#define TEXTBOLD(tty) ""
#define TEXTNORMAL(tty) ""

#endif
#endif
