#include "../src/lab.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
// TODO: need to make sure readline and history work on github workspaces

int main(int argc, char **argv) {
    parse_args(argc, argv);

    char *line;
    char *env_var = "MY_PROMPT";
    char *prompt = get_prompt(env_var);
    using_history();
    while ((line=readline(prompt))){
        printf("%s\n",line);
        add_history(line);
        free(line);
    }
    return 0;
}
