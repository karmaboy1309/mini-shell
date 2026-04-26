/**
 * main.c - Shell entry point and main REPL loop
 *
 * Sets up signal handling, reads input, dispatches to
 * the parser and executor, and manages the shell lifecycle.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "parser.h"
#include "executor.h"
#include "builtins.h"
#include "utils.h"

/* ── Signal handler for SIGINT (Ctrl+C) ─────────────────────────── */
static volatile sig_atomic_t got_sigint = 0;

static void sigint_handler(int sig)
{
    (void)sig;
    got_sigint = 1;
    /* print a newline so the prompt appears on a fresh line */
    write(STDERR_FILENO, "\n", 1);
}

/* ── Main shell loop ────────────────────────────────────────────── */
int main(void)
{
    char input[MAX_INPUT_LEN];
    Pipeline pl;

    /* install SIGINT handler — shell itself should not die on ^C */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    /* ignore SIGTSTP (Ctrl+Z) in the shell process */
    signal(SIGTSTP, SIG_IGN);

    /* welcome banner */
    fprintf(stderr,
        "\033[1;36m"
        "╔══════════════════════════════════════╗\n"
        "║        mysh — Mini Unix Shell        ║\n"
        "║   Type 'exit' or Ctrl+D to quit.     ║\n"
        "╚══════════════════════════════════════╝\n"
        "\033[0m\n");

    /* ── REPL ───────────────────────────────────────────────────── */
    while (1) {
        got_sigint = 0;
        print_prompt();

        /* read one line of input */
        if (fgets(input, sizeof(input), stdin) == NULL) {
            if (got_sigint) continue; /* ^C interrupted fgets */
            /* EOF (Ctrl+D) */
            fprintf(stderr, "\nexit\n");
            break;
        }

        strip_newline(input);

        /* skip empty / whitespace-only input */
        if (is_empty(input))
            continue;

        /* record in history (before parsing mutates the buffer) */
        history_add(input);

        /* make a mutable copy for the parser (it writes NULs) */
        char line_copy[MAX_INPUT_LEN];
        strncpy(line_copy, input, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';

        /* parse */
        if (parse_input(line_copy, &pl) != 0)
            continue;

        /* execute */
        int ret = execute_pipeline(&pl);
        if (ret == -1 && pl.num_commands == 1
            && strcmp(pl.commands[0].args[0], "exit") == 0) {
            break; /* built-in exit returns -1 as sentinel */
        }
    }

    history_cleanup();
    return 0;
}
