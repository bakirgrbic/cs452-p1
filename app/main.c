#include "../src/lab.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>

int check_processes(char* trimmed);
void prune_processes(int bgp_length);
void create_process(struct shell *sh, char** argv, char* trimmed);

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

        if (line != NULL) {
            trimmed = trim_white(line);
        } else {
            trimmed = "";
        }

        int i = check_processes(trimmed);
        prune_processes(i);

        // EOF check
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

        // Handles built in commands and processes
        if (!do_builtin(&my_shell, parsed)) {
            create_process(&my_shell, parsed, trimmed);
        }

        cmd_free(parsed);
        free(line);
    }

    // Shouldn't need to exit here but rather with my_exit but just in case.
    sh_destroy(&my_shell);
    return 10;
}
