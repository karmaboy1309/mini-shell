/**
 * executor.c - Command execution engine
 *
 * Handles single/pipeline execution via fork()+execvp(),
 * I/O redirection via dup2(), and background processes.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#include "executor.h"
#include "builtins.h"
#include "utils.h"

static int setup_redir(const Redirect *r)
{
    if (r->input_file) {
        int fd = open(r->input_file, O_RDONLY);
        if (fd == -1) { perror("mysh"); return -1; }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (r->output_file) {
        int flags = O_WRONLY | O_CREAT | (r->append ? O_APPEND : O_TRUNC);
        int fd = open(r->output_file, flags, 0644);
        if (fd == -1) { perror("mysh"); return -1; }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    return 0;
}

static void reap_bg(void)
{
    int st;
    pid_t p;
    while ((p = waitpid(-1, &st, WNOHANG)) > 0)
        fprintf(stderr, "[done] %d (exit %d)\n", p,
                WIFEXITED(st) ? WEXITSTATUS(st) : WTERMSIG(st));
}

int execute_pipeline(Pipeline *pl)
{
    if (!pl || pl->num_commands == 0) return -1;
    reap_bg();

    if (pl->num_commands == 1 && is_builtin(pl->commands[0].args[0]))
        return execute_builtin(&pl->commands[0]);

    int n = pl->num_commands;
    int pfd[MAX_PIPE_CMDS - 1][2];
    pid_t pids[MAX_PIPE_CMDS];

    for (int i = 0; i < n - 1; i++)
        if (pipe(pfd[i]) == -1) { perror("mysh: pipe"); return -1; }

    for (int i = 0; i < n; i++) {
        pids[i] = fork();
        if (pids[i] < 0) { perror("mysh: fork"); return -1; }
        if (pids[i] == 0) {
            signal(SIGINT, SIG_DFL);
            if (i > 0) dup2(pfd[i-1][0], STDIN_FILENO);
            if (i < n-1) dup2(pfd[i][1], STDOUT_FILENO);
            for (int j = 0; j < n-1; j++) { close(pfd[j][0]); close(pfd[j][1]); }
            if (setup_redir(&pl->commands[i].redir) != 0) _exit(1);
            if (is_builtin(pl->commands[i].args[0])) {
                int r = execute_builtin(&pl->commands[i]);
                _exit(r == 0 ? 0 : 1);
            }
            execvp(pl->commands[i].args[0], pl->commands[i].args);
            fprintf(stderr, "mysh: %s: %s\n", pl->commands[i].args[0], strerror(errno));
            _exit(127);
        }
    }

    for (int i = 0; i < n-1; i++) { close(pfd[i][0]); close(pfd[i][1]); }

    int last = 0;
    if (pl->background) {
        printf("[bg] %d\n", pids[n-1]);
    } else {
        for (int i = 0; i < n; i++) {
            int st;
            while (waitpid(pids[i], &st, 0) == -1)
                if (errno != EINTR) { perror("mysh: waitpid"); return -1; }
            if (i == n-1) last = WIFEXITED(st) ? WEXITSTATUS(st) : 128+WTERMSIG(st);
        }
    }
    return last;
}
