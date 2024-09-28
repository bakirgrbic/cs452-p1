#include "lab.h"
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <readline/history.h>
#include <signal.h>
#include <sys/wait.h>

void my_exit(struct shell *sh, char **argv);
void my_pwd();
void my_history();
bool is_background(char** argv);
void my_jobs();

struct background_process {
    int n;
    pid_t bg_pid;
    char* command;
    struct background_process *next;
    bool completed;
    bool reported;
};

/* This list is only used in this file and no where else. It is needed
 * becuase there is no other way to share the list information on background
 * processes to my_jobs method defined in this file without changing
 * the api declaraition for do_builtin in lab.h. */
struct background_process *bgp_list = NULL;

char *get_prompt(const char *env) {
    int default_length = 0;
    int prompt_length = 0;
    char *env_var = getenv(env);
    char *prompt;

    if (env_var == NULL) {
        default_length = 7; // 6 chars + 1 for \0
        prompt = (char*) calloc(default_length, sizeof(char));
        strncpy(prompt, "shell>", default_length);
    } else {
        prompt_length = strlen(env_var) + 1; // + 1 for \0
        prompt = (char*) calloc(prompt_length, sizeof(char));
        strncpy(prompt, env_var, prompt_length);
    }

    return prompt;
}

int change_dir(char **dir) {
    // 1st part of dir is the command so ignore it
    errno = 0;
    char *target_dir = dir[1]; // Assuming that no flags are passed and only changing dirs
    char *path;
    int return_value;
    if (target_dir == NULL) {
        char *home = getenv("HOME");
        if (home == NULL) {
            uid_t user_id = getuid();
            struct passwd *pwd = getpwuid(user_id);
            home = pwd->pw_dir;
        }
        path = home;

    } else {
        path = target_dir;
    }

    return_value = chdir(path);

    if (return_value == -1) {
        perror("cd failed");
    }

    return return_value;
}

char **cmd_parse(char const *line) {
    errno = 0;
    long max_args = sysconf(_SC_ARG_MAX);
    if (max_args < 0 && errno != 0) {
          perror ("sysconf error trying to get ARG_MAX");
    }

    int number_tokens = 1; // at least null is there
    int length = strlen(line);
    char copy[length+1];
    stpcpy(copy, line);

    char *token = strtok(copy, " ");
    while (token != NULL) {
        number_tokens++;
        token = strtok(NULL, " ");
    }

    int command_length = number_tokens;
    if (number_tokens > max_args) {
        command_length = max_args;
    }

    char **parsed = (char**) malloc(command_length * sizeof(char*));

    stpcpy(copy, line);
    token = strtok(copy, " ");
    int i = 0;
    while (i < command_length) {
        if (token != NULL) {
            parsed[i] = (char*) malloc((strlen(token) + 1) * sizeof(char));
            stpcpy(parsed[i], token);
        } else {
            parsed[i] = (char*) NULL;
        }
        token = strtok(NULL, " ");
        i++;
    }

    return parsed;
}

void cmd_free(char ** line) {
    int i = 0;
    while (line[i] != NULL) {
        free(line[i]);
        i++;
    }
    free(line);
}

char *trim_white(char *line) {
    int length = strlen(line);
    int start = -1;
    int end = 0;
    for (int i = 0; i < length; i++) {
        if (start == -1 && line[i] != ' ') {
            start = i;
        }
        if (start != -1 && line[i] != ' ') {
            end = i;
        }
    }
    if (start != -1) { 
        int i = 0;
        while (start <= end) {
            line[i] = line[start];
            start++;
            i++;
        }
        while (i < length) {
            line[i] = '\0';
            i++;
        }
    } else {
        line[end] = '\0';
    }

    return line; 
}

bool do_builtin(struct shell *sh, char **argv) {
    char* command = argv[0];
    if (command == NULL) {
        return false;
    }

    if (strcmp(command, "exit") == 0) {
        my_exit(sh, argv);
        return true;
    } else if (strcmp(command, "cd") == 0) {
        change_dir(argv);
        return true;
    } else if (strcmp(command, "pwd") == 0) {
        my_pwd();
        return true;
    } else if (strcmp(command, "history") == 0) {
        my_history();
        return true;
    } else if (strcmp(command, "jobs") == 0) {
        my_jobs();
        return true;
    }

    return false;
}

void sh_init(struct shell *sh) {
    sh->shell_terminal = STDIN_FILENO;
    sh->shell_is_interactive = isatty(sh->shell_terminal);
    if (sh->shell_is_interactive) {
        while (tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp())) {
            kill(- sh->shell_pgid, SIGTTIN);
        }
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        sh->shell_pgid = getpid();
        if (setpgid(sh->shell_pgid, sh->shell_pgid) < 0) {
          perror ("Couldn't put the shell in its own process group");
          exit(1);
        }

        tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
        tcgetattr(sh->shell_terminal, &(sh->shell_tmodes));

        sh->prompt = get_prompt("MY_PROMPT");
    }
}

void sh_destroy(struct shell *sh) {
    free(sh->prompt);
}

void parse_args(int argc, char **argv) {
    int opt;

    while ( (opt = getopt(argc, argv, "hv")) != -1) {
        switch (opt) {
            case 'h':
                fprintf(stderr, "Usage: %s [-v] [-h]\n", argv[0]);
                exit(0);
                break;
            case 'v':
                fprintf(stdout, "Version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
                exit(0);
                break;
            default:
                fprintf(stderr, "Usage: %s [-v] [-h]\n", argv[0]);
                exit(2);
                break;
        }
    } 
}

/**
* @brief Takes shell, and formatted command 
* to properly dellocate memory and exit the program.
*
* @param sh The shell
* @param argv The formated command
*/
void my_exit(struct shell *sh, char **argv) {
    cmd_free(argv);
    sh_destroy(sh);

    // Free background processes
    struct background_process *current_bgp = bgp_list;
    struct background_process *next_bgp = current_bgp;
    while (current_bgp != NULL) {
        next_bgp = current_bgp->next;
        free(current_bgp->command);
        free(current_bgp);
        current_bgp = next_bgp;
    }

    exit(0);
}

/**
* @brief Prints the current working directory in the shell
*/
void my_pwd() {
    char *cwd = getcwd(NULL, 0);
    printf("%s\n", cwd);
    free(cwd);
}

/**
* @brief Prints history of all commands entered into the shell.
*/
void my_history() {
    HIST_ENTRY **history = history_list();

    int i = 0;
    while (history[i] != NULL) {
        printf("%s\n", history[i]->line);
        i++;
    }
}

/**
* @brief Checks whether a parsed command is a background process. Also 
* removes ampersand character from background proccess's formatted
* commands to have sanitized input for exec() functions.
*
* @param argv The formated command
* @return True if the command is a background process, false otherwise.
*/
bool is_background(char** argv) {
    bool background = false;

    int i = 0;
    while (argv[i] != NULL) {
        // Need to walk to the end of the list to check the last command
        if (argv[i+1] == NULL) {
            if (strcmp(argv[i], "&") == 0) {
                background = true;
                free(argv[i]);
                argv[i] = (char*) NULL;
            } else {
                // Need to make sure last string doesn't contain & at the end
                int length = strlen(argv[i]);
                if (argv[i][length-1] == '&') {
                    background = true;
                    argv[i][length-1] = '\0';
                }
            }
        }
        i++;
    }

    return background;
}

/**
* @brief Prints all running and done process.
*/
void my_jobs() {
    struct background_process *current_bgp = bgp_list;

    while (current_bgp != NULL) {
        if (!current_bgp->completed) {
            printf("[%d] %d Running %s\n", current_bgp->n, current_bgp->bg_pid, current_bgp->command);
        }
        if (current_bgp->completed && !current_bgp->reported) {
            printf("[%d] Done %s\n", current_bgp->n, current_bgp->command);
            current_bgp->reported = true;
        }

        current_bgp = current_bgp->next;
    }
}

/**
* @brief Checks if all processes are done. Prints any jobs that are so long
* that the trimmed argument is not "jobs".
*
* @param trimmed White space trimmed string
* @return length of background process list
*/
int check_processes(char* trimmed) {
    int status;
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

    return i;
}

/**
* @brief Removes all background processes that have been completed and reported.
*
* @param bgp_length Total length of background processes in bgp_list
*/
void prune_processes(int bgp_length) {
    struct background_process *current_bgp = bgp_list;
    struct background_process *prev_bgp;
    int i = 0;
    while (i < bgp_length) {
        current_bgp = bgp_list;
        prev_bgp = current_bgp;
        while (current_bgp != NULL) {
            if (current_bgp->completed && current_bgp->reported) {
                if (current_bgp == bgp_list) { // start of list
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
        i++;
    }
}

/**
* @brief Creates foreground and background processes. Updates control of shell
* for child and parent processes. Creates background_process elements and
* adds them to bgp_list.
*
* @param sh The shell
* @param argv The formated command
* @param trimmed The white space trimmed user input
*/
void create_process(struct shell *sh, char** argv, char* trimmed) {
    if (argv == NULL || argv[0] == NULL) {
        return;
    }
    bool background = is_background(argv);
    int rc = fork();

    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(3);
    } else if (rc == 0) { // child
        pid_t child = getpid();
        setpgid(child, child);

        if (!background) {
            tcsetpgrp(sh->shell_terminal, child);
        }

        signal (SIGINT, SIG_DFL);
        signal (SIGQUIT, SIG_DFL);
        signal (SIGTSTP, SIG_DFL);
        signal (SIGTTIN, SIG_DFL);
        signal (SIGTTOU, SIG_DFL);

        execvp(argv[0], argv);
        perror("exec failed");
        exit(4);
    } else { // parent
        if (!background) { // foreground
            wait(NULL);
            tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
            tcsetattr(sh->shell_terminal, TCSADRAIN, &(sh->shell_tmodes));
        } else { // background
            struct background_process *current_bgp = (struct background_process*) malloc(sizeof(struct background_process));
            current_bgp->bg_pid = rc;
            current_bgp->command = (char*) malloc((strlen(trimmed) + 1) * sizeof(char));
            stpcpy(current_bgp->command, trimmed);
            current_bgp->next = NULL;
            current_bgp->completed = false;
            current_bgp->reported = false;

            if (bgp_list == NULL) { // No other background processes
                current_bgp->n = 1;

                printf("[%d] %d %s\n", current_bgp->n, current_bgp->bg_pid, current_bgp->command);

                bgp_list = current_bgp;
            } else { // Other background processes exist
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
