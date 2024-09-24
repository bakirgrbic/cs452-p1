#include "lab.h"
#include <stdio.h>
#include <string.h>

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
    // TODO
    return -1
}

char **cmd_parse(char const *line) {
    // TODO
    char **blah = ["a", "b"];
    return blah;
}

void cmd_free(char ** line) {
    // TODO
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
    // TODO
}

void sh_init(struct shell *sh) {
    // TODO
}

void sh_destroy(struct shell *sh) {
    // TODO
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
