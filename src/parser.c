/**
 * parser.c - Command parsing and tokenization
 *
 * Converts a raw input line into a Pipeline structure, handling:
 *   - pipe splitting (|)
 *   - I/O redirection (<, >, >>)
 *   - background execution (&)
 *   - single/double quoted strings
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "utils.h"

/* ── Forward declarations (internal helpers) ────────────────────── */
static int  parse_simple_command(char *segment, SimpleCommand *cmd);
static char *next_token(char **cursor);

/* ── pipeline_init ──────────────────────────────────────────────── */
void pipeline_init(Pipeline *pl)
{
    memset(pl, 0, sizeof(*pl));
}

/* ── parse_input ────────────────────────────────────────────────── */
int parse_input(char *input, Pipeline *pl)
{
    pipeline_init(pl);

    if (!input || is_empty(input))
        return -1;

    /* ── detect & remove trailing '&' (background flag) ─────────── */
    char *trimmed = trim_whitespace(input);
    size_t len = strlen(trimmed);

    if (len > 0 && trimmed[len - 1] == '&') {
        pl->background = 1;
        trimmed[len - 1] = '\0';
        trimmed = trim_whitespace(trimmed);
        if (is_empty(trimmed))
            return -1;
    }

    /* ── split on unquoted '|' characters ───────────────────────── */
    char *segments[MAX_PIPE_CMDS];
    int   seg_count = 0;

    char *p = trimmed;
    char *seg_start = p;
    int   in_single = 0;
    int   in_double = 0;

    while (*p) {
        if (*p == '\'' && !in_double)
            in_single = !in_single;
        else if (*p == '"' && !in_single)
            in_double = !in_double;
        else if (*p == '|' && !in_single && !in_double) {
            if (seg_count >= MAX_PIPE_CMDS) {
                fprintf(stderr, "mysh: too many piped commands\n");
                return -1;
            }
            *p = '\0';
            segments[seg_count++] = seg_start;
            seg_start = p + 1;
        }
        p++;
    }

    /* last segment */
    if (seg_count >= MAX_PIPE_CMDS) {
        fprintf(stderr, "mysh: too many piped commands\n");
        return -1;
    }
    segments[seg_count++] = seg_start;

    /* ── parse each pipe segment ────────────────────────────────── */
    for (int i = 0; i < seg_count; i++) {
        if (parse_simple_command(segments[i], &pl->commands[i]) != 0)
            return -1;
    }
    pl->num_commands = seg_count;

    return 0;
}

/* ── parse_simple_command ───────────────────────────────────────── *
 * Tokenizes one pipe-segment into arguments and redirection info.  */
static int parse_simple_command(char *segment, SimpleCommand *cmd)
{
    memset(cmd, 0, sizeof(*cmd));
    char *cursor = segment;
    char *token;

    while ((token = next_token(&cursor)) != NULL) {
        /* ── input redirection ──────────────────────────────────── */
        if (strcmp(token, "<") == 0) {
            token = next_token(&cursor);
            if (!token) {
                fprintf(stderr, "mysh: syntax error near '<'\n");
                return -1;
            }
            cmd->redir.input_file = token; /* points into segment */
        }
        /* ── append redirection ─────────────────────────────────── */
        else if (strcmp(token, ">>") == 0) {
            token = next_token(&cursor);
            if (!token) {
                fprintf(stderr, "mysh: syntax error near '>>'\n");
                return -1;
            }
            cmd->redir.output_file = token;
            cmd->redir.append = 1;
        }
        /* ── output redirection ─────────────────────────────────── */
        else if (strcmp(token, ">") == 0) {
            token = next_token(&cursor);
            if (!token) {
                fprintf(stderr, "mysh: syntax error near '>'\n");
                return -1;
            }
            cmd->redir.output_file = token;
            cmd->redir.append = 0;
        }
        /* ── normal argument ────────────────────────────────────── */
        else {
            if (cmd->argc >= MAX_ARGS - 1) {
                fprintf(stderr, "mysh: too many arguments\n");
                return -1;
            }
            cmd->args[cmd->argc++] = token;
        }
    }

    cmd->args[cmd->argc] = NULL; /* execvp requires NULL-terminated */

    if (cmd->argc == 0) {
        fprintf(stderr, "mysh: empty command in pipeline\n");
        return -1;
    }
    return 0;
}

/* ── next_token ─────────────────────────────────────────────────── *
 * Extracts the next whitespace-delimited token from *cursor.       *
 * Respects single and double quoting, and treats >, >>, < as       *
 * standalone tokens even without surrounding spaces.               */
static char *next_token(char **cursor)
{
    char *p = *cursor;

    /* skip leading whitespace */
    while (*p && isspace((unsigned char)*p))
        p++;

    if (*p == '\0')
        return NULL;

    /* ── check for redirection operators ────────────────────────── */
    if (*p == '<') {
        *cursor = p + 1;
        /* we need a stable string to return; mutate in-place */
        /* '<' is already a single char; just null-terminate  */
        /* but we can't destroy the next char—return literal.  */
        static char lt[] = "<";
        return lt;
    }
    if (*p == '>') {
        if (*(p + 1) == '>') {
            *cursor = p + 2;
            static char append[] = ">>";
            return append;
        }
        *cursor = p + 1;
        static char gt[] = ">";
        return gt;
    }

    /* ── quoted string ──────────────────────────────────────────── */
    if (*p == '\'' || *p == '"') {
        char quote = *p;
        p++;  /* skip opening quote */
        char *start = p;
        while (*p && *p != quote)
            p++;
        if (*p == quote) {
            *p = '\0'; /* terminate the token */
            p++;
        }
        /* else: unterminated quote—use what we have */
        *cursor = p;
        return start;
    }

    /* ── unquoted word ──────────────────────────────────────────── */
    char *start = p;
    while (*p && !isspace((unsigned char)*p)
               && *p != '<' && *p != '>'
               && *p != '\'' && *p != '"') {
        p++;
    }
    if (*p && !isspace((unsigned char)*p)) {
        /* stopped at a special char—don't consume it */
        *cursor = p;
    } else if (*p) {
        *p = '\0';
        *cursor = p + 1;
    } else {
        *cursor = p;
    }

    return start;
}
