#include "lab.h"
#include <stdio.h>

char *get_prompt(const char *env) {
    // No need for malloc, getenv()'s returned pointer should not be modified.
    char *prompt = getenv(env);
    if (prompt == NULL) {
        prompt = "shell>";
    }
    return prompt;
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
