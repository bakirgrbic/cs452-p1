#include "../src/lab.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <sys/wait.h>
// TODO: need to make sure readline and history work on github workspaces

void my_exit(struct shell *sh, char **argv);
void my_pwd();
void my_history();
bool is_background(char** argv);

int main(int argc, char **argv) {
    parse_args(argc, argv);

    struct shell my_shell;
    sh_init(&my_shell);

    char *line;
    char *trimmed;
    char **parsed;
    using_history();

    while (1) {
        line = readline(my_shell.prompt);

        // EOF read
        if (line == NULL) {
            printf("\n");
            char *command = "exit";
            char **command_parsed = (char**) malloc(2 * sizeof(char*));
            command_parsed[0] = (char*) malloc((strlen(command) + 1) * sizeof(char));
            stpcpy(command_parsed[0], command);
            command_parsed[1] = (char*) NULL;
            do_builtin(&my_shell, command_parsed);
        }

        add_history(line);
        trimmed = trim_white(line);
        parsed = cmd_parse(trimmed);

        if (!do_builtin(&my_shell, parsed) && parsed[0] != NULL) {
            int rc = fork();
            if (rc < 0) {
                fprintf(stderr, "fork failed\n");
                exit(3);
            } else if (rc == 0) {
                pid_t child = getpid();
                setpgid(child, child);
                // Set an if branch here to control if a proc is foreground or not
                // For now it is foreground
                tcsetpgrp(my_shell.shell_terminal, child);

                signal (SIGINT, SIG_DFL);
                signal (SIGQUIT, SIG_DFL);
                signal (SIGTSTP, SIG_DFL);
                signal (SIGTTIN, SIG_DFL);
                signal (SIGTTOU, SIG_DFL);

                execvp(parsed[0], parsed);
                perror("exec failed");
                exit(4);
            } else {
                wait(NULL);
                tcsetpgrp(my_shell.shell_terminal, my_shell.shell_pgid);
                tcsetattr(my_shell.shell_terminal, TCSADRAIN, &(my_shell.shell_tmodes));
            }
        }

        cmd_free(parsed);
        free(line);
    }

    // Shouldn't need to exit here but rather with my_exit.
    //  Just in case though :)
    sh_destroy(&my_shell);
    return 10;
}
