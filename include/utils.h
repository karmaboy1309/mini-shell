/**
 * utils.h - Utility / helper functions
 *
 * Provides string manipulation helpers and prompt rendering
 * used throughout the shell.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/**
 * trim_whitespace - Remove leading and trailing whitespace in-place.
 *
 * @param str  String to trim (modified in-place).
 * @return     Pointer to the trimmed string (may differ from str).
 */
char *trim_whitespace(char *str);

/**
 * strip_newline - Remove trailing newline / carriage-return characters.
 *
 * @param str  String to strip.
 */
void strip_newline(char *str);

/**
 * is_empty - Check if a string is NULL or contains only whitespace.
 *
 * @param str  String to test.
 * @return     1 if empty, 0 otherwise.
 */
int is_empty(const char *str);

/**
 * print_prompt - Render and display the shell prompt.
 *
 * Format: mysh@<user>:<cwd>$
 * Falls back to "mysh$ " if environment info is unavailable.
 */
void print_prompt(void);

/**
 * safe_strdup - strdup() wrapper that exits on allocation failure.
 *
 * @param s  String to duplicate.
 * @return   Newly allocated copy (caller must free).
 */
char *safe_strdup(const char *s);

#endif /* UTILS_H */
