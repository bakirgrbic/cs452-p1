#include "lab.h"
#include <stdio.h>
#include <string.h>

void my_exit(struct shell *sh, char **argv);
void my_pwd();

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

//  int change_dir(char **dir) {
//      // TODO
//      return -1;
//  }

char **cmd_parse(char const *line) {
    // TODO: Make sure to fully follow instructions in func stub
    int number_tokens = 1; // at least null is there
    int length = strlen(line);
    char copy[length+1];
    stpcpy(copy, line);

    char *token = strtok(copy, " ");
    while (token != NULL) {
        number_tokens++;
        token = strtok(NULL, " ");
    }

    char **parsed = (char**) malloc(number_tokens * sizeof(char*));

    stpcpy(copy, line);
    token = strtok(copy, " ");
    int i = 0;
    while (i < number_tokens) {
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
        // call my_cd();
        return true;
    } else if (strcmp(command, "pwd") == 0) {
        my_pwd();
        return true;
    } else if (strcmp(command, "history") == 0) {
        // call my_history();
        return true;
    }

    return false;
}

void sh_init(struct shell *sh) {
    // TODO: Make sure to fully follow instructions in func stub
    sh->shell_is_interactive = 0;
    sh->shell_pgid = getpid();
    // sh->shell_tmodes = NULL;
    sh->shell_terminal = 0;
    sh->prompt = get_prompt("MY_PROMPT");
}

void sh_destroy(struct shell *sh) {
    // TODO: Make sure to fully follow instructions in func stub
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
                exit(1);
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
    // TODO: when would it not exit normally?
    exit(2); // TODO: Change back to zero for normal exits after getting ctrl-d to work
}

/**
* @brief Prints the current working directory in the shell
*/
void my_pwd() {
    char *cwd = getcwd(NULL, 0);

    printf("%s\n", cwd);

    free(cwd);
}
