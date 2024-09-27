#include "../src/lab.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <sys/wait.h>
// TODO: Do you need to free up allocated bgps? Seems to handle itself
// TODO: need to make sure readline and history work on github workspaces
// TODO: Make sure implementation tests pass

bool is_background(char** argv);

struct background_process {
    int n;
    pid_t bg_pid;
    char* command;
    struct background_process *next;
    bool completed;
    bool reported;
};

int main(int argc, char **argv) {
    parse_args(argc, argv);

    struct shell my_shell;
    sh_init(&my_shell);
    extern struct background_process *bgp_list;

    char *line;
    char *trimmed;
    char **parsed;
    int status;
    using_history();

    while (1) {

        line = readline(my_shell.prompt);
        if (line != NULL) {
            trimmed = trim_white(line);
        } else {
            trimmed = "";
        }


        // Check if all process are done and print any if command is not jobs
        int i = 0; 
        struct background_process *current_bgp = bgp_list;
        while (current_bgp != NULL) {
            if (waitpid(current_bgp->bg_pid, &status, WNOHANG)) {
                current_bgp->completed = true;
            }
            if (current_bgp->completed && !current_bgp->reported && strcmp("jobs", trimmed) != 0) {
                printf("[%d] Done %s\n", current_bgp->n, current_bgp->command);
                current_bgp->reported = true;
            }
            current_bgp = current_bgp->next;
            i++;
        }

        // Remove all nodes that are done
        struct background_process *prev_bgp;
        int j = 0;
        while (j < i) {
            current_bgp = bgp_list;
            prev_bgp = current_bgp;
            while (current_bgp != NULL) {
                if (current_bgp->completed && current_bgp->reported) {
                    if (current_bgp == bgp_list && current_bgp->next == NULL) { // singleton list
                        free(current_bgp->command);
                        free(current_bgp);
                        bgp_list = NULL;
                    } else if (current_bgp == bgp_list && current_bgp->next != NULL) { // start of list with multiple items
                        current_bgp = bgp_list->next;
                        free(bgp_list->command);
                        free(bgp_list);
                        bgp_list = current_bgp;
                    } else if (current_bgp->next != NULL) { // middle of list
                        prev_bgp->next = current_bgp->next;
                        free(current_bgp->command);
                        free(current_bgp);
                        current_bgp = NULL;
                    } else { // end
                        prev_bgp->next = NULL;
                        free(current_bgp->command);
                        free(current_bgp);
                    }
                    break;
                }
                prev_bgp = current_bgp;
                current_bgp = current_bgp->next;
            }
            j++;
        }


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
        parsed = cmd_parse(trimmed);

        // Built in commands and creating processes
        if (!do_builtin(&my_shell, parsed) && parsed[0] != NULL) {
            bool background = is_background(parsed);
            int rc = fork();

            if (rc < 0) {
                fprintf(stderr, "fork failed\n");
                exit(3);
            } else if (rc == 0) {
                pid_t child = getpid();
                setpgid(child, child);

                if (!background) {
                    tcsetpgrp(my_shell.shell_terminal, child);
                }

                signal (SIGINT, SIG_DFL);
                signal (SIGQUIT, SIG_DFL);
                signal (SIGTSTP, SIG_DFL);
                signal (SIGTTIN, SIG_DFL);
                signal (SIGTTOU, SIG_DFL);

                execvp(parsed[0], parsed);
                perror("exec failed");
                exit(4);
            } else {
                if (!background) {
                    wait(NULL);
                    tcsetpgrp(my_shell.shell_terminal, my_shell.shell_pgid);
                    tcsetattr(my_shell.shell_terminal, TCSADRAIN, &(my_shell.shell_tmodes));
                } else {
                    // Need to add background process to list and output info
                    struct background_process *current_bgp = (struct background_process*) malloc(sizeof(struct background_process));
                    current_bgp->bg_pid = rc;
                    current_bgp->command = (char*) malloc((strlen(trimmed) + 1) * sizeof(char));
                    stpcpy(current_bgp->command, trimmed);
                    current_bgp->next = NULL;
                    current_bgp->completed = false;
                    current_bgp->reported = false;

                    if (bgp_list == NULL) {
                        current_bgp->n = 1;

                        printf("[%d] %d %s\n", current_bgp->n, current_bgp->bg_pid, current_bgp->command);

                        bgp_list = current_bgp;
                    } else {
                        struct background_process *bgp = bgp_list;
                        while(bgp->next != NULL) {
                            bgp = bgp->next;
                        }
                        current_bgp->n = bgp->n + 1;

                        printf("[%d] %d %s\n", current_bgp->n, current_bgp->bg_pid, current_bgp->command);

                        bgp->next = current_bgp;
                    }
                }
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
