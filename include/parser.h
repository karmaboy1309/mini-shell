/**
 * parser.h - Command parsing and tokenization
 *
 * Defines data structures for parsed commands, including support
 * for pipes, I/O redirection, and background execution.
 */

#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

/* ── Limits ─────────────────────────────────────────────────────── */
#define MAX_ARGS        64      /* max arguments per simple command   */
#define MAX_PIPE_CMDS   16      /* max commands in a pipeline         */
#define MAX_INPUT_LEN   4096    /* max characters in one input line   */

/* ── Redirection descriptor ─────────────────────────────────────── */
typedef struct {
    char *input_file;           /* file for stdin  redirection (<)    */
    char *output_file;          /* file for stdout redirection (>/>>) */
    int   append;               /* 1 = append (>>), 0 = truncate (>) */
} Redirect;

/* ── Single (simple) command ────────────────────────────────────── */
typedef struct {
    char    *args[MAX_ARGS];    /* NULL-terminated argument vector    */
    int      argc;              /* number of arguments                */
    Redirect redir;             /* I/O redirection for this command   */
} SimpleCommand;

/* ── Complete pipeline ──────────────────────────────────────────── */
typedef struct {
    SimpleCommand commands[MAX_PIPE_CMDS];
    int           num_commands;  /* how many simple commands           */
    int           background;    /* 1 = run in background (&)         */
} Pipeline;

/* ── Public API ─────────────────────────────────────────────────── */

/**
 * parse_input - Parse a raw input line into a Pipeline structure.
 *
 * Handles:
 *   - splitting on '|' for pipes
 *   - '<', '>', '>>' for I/O redirection
 *   - trailing '&' for background execution
 *   - quoted strings (single and double quotes)
 *
 * @param input  Raw input string (will be modified in-place).
 * @param pl     Pointer to a Pipeline struct to fill.
 * @return       0 on success, -1 on parse error.
 */
int parse_input(char *input, Pipeline *pl);

/**
 * pipeline_init - Zero-initialize a Pipeline struct.
 */
void pipeline_init(Pipeline *pl);

#endif /* PARSER_H */
