/**
 * builtins.h - Built-in shell commands
 *
 * Built-ins are executed inside the shell process itself (no fork).
 * Supported: cd, pwd, exit, history.
 */

#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

/**
 * is_builtin - Check if a command name is a shell built-in.
 *
 * @param name  Command name (e.g., "cd").
 * @return      1 if built-in, 0 otherwise.
 */
int is_builtin(const char *name);

/**
 * execute_builtin - Run a built-in command.
 *
 * @param cmd  Pointer to the SimpleCommand to execute.
 * @return     0 on success, 1 on failure, -1 to signal shell exit.
 */
int execute_builtin(SimpleCommand *cmd);

/**
 * history_add - Record a command line in the history buffer.
 *
 * @param line  The raw command string.
 */
void history_add(const char *line);

/**
 * history_print - Display the stored command history.
 */
void history_print(void);

/**
 * history_cleanup - Free all memory used by the history buffer.
 */
void history_cleanup(void);

#endif /* BUILTINS_H */
