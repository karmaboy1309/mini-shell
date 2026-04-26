/**
 * utils.c - Utility / helper functions
 *
 * String manipulation, prompt rendering, and safe allocation helpers.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <pwd.h>

#include "utils.h"

/* ── trim_whitespace ────────────────────────────────────────────── */
char *trim_whitespace(char *str)
{
    if (!str)
        return str;

    /* skip leading whitespace */
    while (isspace((unsigned char)*str))
        str++;

    if (*str == '\0')
        return str;

    /* trim trailing whitespace */
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';

    return str;
}

/* ── strip_newline ──────────────────────────────────────────────── */
void strip_newline(char *str)
{
    if (!str)
        return;

    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

/* ── is_empty ───────────────────────────────────────────────────── */
int is_empty(const char *str)
{
    if (!str)
        return 1;

    while (*str) {
        if (!isspace((unsigned char)*str))
            return 0;
        str++;
    }
    return 1;
}

/* ── print_prompt ───────────────────────────────────────────────── */
void print_prompt(void)
{
    char cwd[PATH_MAX];
    const char *user = NULL;
    const char *home = NULL;

    /* try $USER, fall back to getpwuid */
    user = getenv("USER");
    if (!user) {
        struct passwd *pw = getpwuid(getuid());
        if (pw)
            user = pw->pw_name;
    }
    if (!user)
        user = "user";

    home = getenv("HOME");

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        /* fallback if getcwd fails */
        fprintf(stderr, "\033[1;32mmysh\033[0m$ ");
        fflush(stdout);
        return;
    }

    /* shorten $HOME to ~ for display */
    const char *display_cwd = cwd;
    char short_cwd[PATH_MAX];
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        snprintf(short_cwd, sizeof(short_cwd), "~%s", cwd + strlen(home));
        display_cwd = short_cwd;
    }

    /* colorized prompt: mysh@user:cwd$ */
    fprintf(stderr,
            "\033[1;32mmysh\033[0m@\033[1;34m%s\033[0m:\033[1;36m%s\033[0m$ ",
            user, display_cwd);
    fflush(stderr);
}

/* ── safe_strdup ────────────────────────────────────────────────── */
char *safe_strdup(const char *s)
{
    if (!s)
        return NULL;

    char *dup = strdup(s);
    if (!dup) {
        perror("mysh: strdup");
        exit(EXIT_FAILURE);
    }
    return dup;
}
