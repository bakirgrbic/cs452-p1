#include "../src/lab.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
// TODO: need to make sure readline and history work on github workspaces

int main(int argc, char **argv) {
    parse_args(argc, argv);

    struct shell my_shell;
    sh_init(&my_shell);

    char *line;
    char *trimmed;
    using_history();
    while ((line=readline(my_shell.prompt))){
        add_history(line);
        trimmed = trim_white(line);
        // cmd_parse to simplify processing
        // check if its a built in with do_builtin
        //      else do nada for now
        // cmd_free when done with cmd_parse
        free(line);
    }

    // Deconstruct the shell before exiting
    sh_destroy(&my_shell);

    return 0;
}
