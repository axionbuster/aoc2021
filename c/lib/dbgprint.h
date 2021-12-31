#ifndef DBGPRINT_H
#define DBGPRINT_H
#ifndef NDEBUG
#define dbgprintf(...) fprintf(stderr, __VA_ARGS__)
#define dbgflush(...) fflush(__VA_ARGS__)
#else
#define dbgprintf(...) 0
#define dbgflush(...) 0
#endif
#endif
