#pragma once

#include <Arduino.h>

// Inspiration: https://github.com/littlstar/asprintf.c/blob/master/asprintf.c

/**
 * Sets `char **' pointer to be a buffer
 * large enough to hold the PGM_P formatted
 * string accepting `n' arguments of
 * variadic arguments.
 */

int asprintf_P(char **str, PGM_P formatP, ...);

/**
 * Sets `char **' pointer to be a buffer
 * large enough to hold the __FlashStringHelper (ie F()) formatted
 * string accepting `n' arguments of
 * variadic arguments.
 */

int asprintf(char **str, const __FlashStringHelper *formatP, ...);

/**
 * Sets `char **' pointer to be a buffer
 * large enough to hold the PGM_P formatted string
 * accepting a `va_list' args of variadic
 * arguments.
 */

int vasprintf_P(char **str, const char *formatP, va_list ap);