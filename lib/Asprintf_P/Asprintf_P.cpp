#include "Asprintf_P.h"

int asprintf(char **str, const __FlashStringHelper *formatP, ...) {

    int size = 0;
    va_list args;

    va_start(args, formatP);

    size = vasprintf_P(str, (PGM_P)formatP, args);

    va_end(args);

    return size;
}

int asprintf_P(char **str, PGM_P formatP, ...) {

    int size = 0;
    va_list args;

    // init variadic argumens
    va_start(args, formatP);

    // format and get size
    size = vasprintf_P(str, (PGM_P)formatP, args);

    // toss args
    va_end(args);

    return size;
}

int vasprintf_P(char **str, const char *formatP, va_list ap) {

    int size = 0;
    va_list tmpa;

    // copy
    va_copy(tmpa, ap);

    // apply variadic arguments to
    // sprintf with format to get size
    size = vsnprintf_P(NULL, 0, (PGM_P)formatP, tmpa);

    // toss args
    va_end(tmpa);

    // return -1 to be compliant if
    // size is less than 0
    if (size < 0) { return -1; }

    // alloc with size plus 1 for `\0'
    *str = (char *) malloc(size + 1);

    // return -1 to be compliant
    // if pointer is `NULL'
    if (NULL == *str) { return -1; }

    // format string with original
    // variadic arguments and set new size
    size = vsnprintf_P(*str, size, formatP, ap);
    return size;
}

