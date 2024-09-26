#include "../src/lab.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
// TODO: need to make sure readline and history work on github workspaces
// TODO: make sure ctrl-d calls my_exit and returns the same exit number

void my_exit(struct shell *sh, char **argv);
void my_pwd();
void my_history();

int main(int argc, char **argv) {
    parse_args(argc, argv);

    struct shell my_shell;
    sh_init(&my_shell);

    char *line;
    char *trimmed;
    char **parsed;
    using_history();

    while ((line=readline(my_shell.prompt))){
        add_history(line);
        trimmed = trim_white(line);
        parsed = cmd_parse(line);
        free(line);
        do_builtin(&my_shell, parsed);
        // Below doesnt work, 
        //  Figure it out when sh_init is more realistic
        //  https://tiswww.case.edu/php/chet/readline/readline.html#Alternate-Interface-Example
        if (line == NULL) {
            char *command[1] = {"exit"};
            do_builtin(&my_shell, command);
        }
        // check for ctrl d
        cmd_free(parsed);
    }

    sh_destroy(&my_shell);

    return 0;
}
