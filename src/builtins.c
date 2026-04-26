/**
 * builtins.c - Built-in shell command implementations
 *
 * Commands handled here run inside the shell process (no fork):
 *   cd      - change working directory
 *   pwd     - print working directory
 *   exit    - terminate the shell
 *   history - display command history
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "builtins.h"
#include "utils.h"

/* ── History ring buffer ────────────────────────────────────────── */
#define HISTORY_MAX 100

static char *history_buf[HISTORY_MAX];   /* circular buffer           */
static int   history_count = 0;          /* total commands ever added  */

/* ── Built-in command table ─────────────────────────────────────── */
static const char *builtin_names[] = {
    "cd",
    "pwd",
    "exit",
    "history",
    NULL
};

/* ── is_builtin ─────────────────────────────────────────────────── */
int is_builtin(const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; builtin_names[i]; i++) {
        if (strcmp(name, builtin_names[i]) == 0)
            return 1;
    }
    return 0;
}

/* ── builtin_cd ─────────────────────────────────────────────────── */
static int builtin_cd(SimpleCommand *cmd)
{
    const char *target;

    if (cmd->argc < 2) {
        /* no argument → go home */
        target = getenv("HOME");
        if (!target) {
            fprintf(stderr, "mysh: cd: HOME not set\n");
            return 1;
        }
    } else if (cmd->argc == 2) {
        if (strcmp(cmd->args[1], "-") == 0) {
            target = getenv("OLDPWD");
            if (!target) {
                fprintf(stderr, "mysh: cd: OLDPWD not set\n");
                return 1;
            }
            printf("%s\n", target);
        } else if (strcmp(cmd->args[1], "~") == 0) {
            target = getenv("HOME");
            if (!target) {
                fprintf(stderr, "mysh: cd: HOME not set\n");
                return 1;
            }
        } else {
            target = cmd->args[1];
        }
    } else {
        fprintf(stderr, "mysh: cd: too many arguments\n");
        return 1;
    }

    /* save current dir as OLDPWD before changing */
    char old[PATH_MAX];
    if (getcwd(old, sizeof(old)) != NULL) {
        setenv("OLDPWD", old, 1);
    }

    if (chdir(target) != 0) {
        perror("mysh: cd");
        return 1;
    }

    /* update PWD */
    char new_cwd[PATH_MAX];
    if (getcwd(new_cwd, sizeof(new_cwd)) != NULL) {
        setenv("PWD", new_cwd, 1);
    }

    return 0;
}

/* ── builtin_pwd ────────────────────────────────────────────────── */
static int builtin_pwd(void)
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("mysh: pwd");
        return 1;
    }
    printf("%s\n", cwd);
    return 0;
}

/* ── builtin_history_cmd ────────────────────────────────────────── */
static int builtin_history_cmd(SimpleCommand *cmd)
{
    int n = HISTORY_MAX; /* default: show all stored */

    if (cmd->argc >= 2) {
        char *endptr;
        long val = strtol(cmd->args[1], &endptr, 10);
        if (*endptr != '\0' || val <= 0) {
            fprintf(stderr, "mysh: history: invalid count '%s'\n",
                    cmd->args[1]);
            return 1;
        }
        n = (int)val;
    }

    /* determine range */
    int total = (history_count < HISTORY_MAX) ? history_count : HISTORY_MAX;
    if (n > total)
        n = total;

    int start_idx;
    int start_num;

    if (history_count <= HISTORY_MAX) {
        start_idx = total - n;
        start_num = start_idx + 1;
    } else {
        start_idx = (history_count % HISTORY_MAX + HISTORY_MAX - n) % HISTORY_MAX;
        start_num = history_count - n + 1;
    }

    for (int i = 0; i < n; i++) {
        int idx = (start_idx + i) % HISTORY_MAX;
        printf("  %4d  %s\n", start_num + i, history_buf[idx]);
    }

    return 0;
}

/* ── execute_builtin ────────────────────────────────────────────── */
int execute_builtin(SimpleCommand *cmd)
{
    if (!cmd || cmd->argc == 0)
        return 1;

    const char *name = cmd->args[0];

    if (strcmp(name, "cd") == 0)
        return builtin_cd(cmd);

    if (strcmp(name, "pwd") == 0)
        return builtin_pwd();

    if (strcmp(name, "exit") == 0) {
        /* optional exit code argument */
        if (cmd->argc >= 2) {
            int code = atoi(cmd->args[1]);
            history_cleanup();
            exit(code);
        }
        return -1; /* signal main loop to exit */
    }

    if (strcmp(name, "history") == 0)
        return builtin_history_cmd(cmd);

    fprintf(stderr, "mysh: unknown builtin '%s'\n", name);
    return 1;
}

/* ── history_add ────────────────────────────────────────────────── */
void history_add(const char *line)
{
    if (!line || is_empty(line))
        return;

    int idx = history_count % HISTORY_MAX;

    /* free previous entry if overwriting in circular buffer */
    free(history_buf[idx]);
    history_buf[idx] = safe_strdup(line);
    history_count++;
}

/* ── history_print ──────────────────────────────────────────────── */
void history_print(void)
{
    int total = (history_count < HISTORY_MAX) ? history_count : HISTORY_MAX;
    int start_idx = (history_count <= HISTORY_MAX)
                        ? 0
                        : (history_count % HISTORY_MAX);
    int start_num = (history_count <= HISTORY_MAX)
                        ? 1
                        : (history_count - HISTORY_MAX + 1);

    for (int i = 0; i < total; i++) {
        int idx = (start_idx + i) % HISTORY_MAX;
        printf("  %4d  %s\n", start_num + i, history_buf[idx]);
    }
}

/* ── history_cleanup ────────────────────────────────────────────── */
void history_cleanup(void)
{
    for (int i = 0; i < HISTORY_MAX; i++) {
        free(history_buf[i]);
        history_buf[i] = NULL;
    }
    history_count = 0;
}
