/**
 * executor.h - Command execution engine
 *
 * Provides functions for executing simple commands and full pipelines,
 * including fork/exec, pipe setup, and I/O redirection via dup2().
 */

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

/**
 * execute_pipeline - Execute a complete parsed pipeline.
 *
 * Creates the necessary pipes, forks child processes, sets up
 * I/O redirection, and waits for foreground processes to complete.
 *
 * @param pl  Pointer to a parsed Pipeline structure.
 * @return    Exit status of the last command in the pipeline,
 *            or -1 on internal error.
 */
int execute_pipeline(Pipeline *pl);

#endif /* EXECUTOR_H */
