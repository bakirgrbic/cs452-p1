#include "lab.h"
#include <stdio.h>

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
