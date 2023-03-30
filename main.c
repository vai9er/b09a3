#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int system_flag = 0;
    int user_flag = 0;
    int graphics_flag = 0;
    int sequential_flag = 0;
    int samples = 10;
    int tdelay = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--system") == 0) {
            system_flag = 1;
        } else if (strcmp(argv[i], "--user") == 0) {
            user_flag = 1;
        } else if (strcmp(argv[i], "--graphics") == 0) {
            graphics_flag = 1;
        } else if (strcmp(argv[i], "--sequential") == 0) {
            sequential_flag = 1;
        } else if (strncmp(argv[i], "--samples=", 10) == 0) {
            samples = atoi(argv[i] + 10);
        } else if (strncmp(argv[i], "--tdelay=", 9) == 0) {
            tdelay = atoi(argv[i] + 9);
        } else if (i == argc - 2) {
            samples = atoi(argv[i]);
        } else if (i == argc - 1) {
            tdelay = atoi(argv[i]);
        }
    }

    printf("system_flag: %d\n", system_flag);
    printf("user_flag: %d\n", user_flag);
    printf("graphics_flag: %d\n", graphics_flag);
    printf("sequential_flag: %d\n", sequential_flag);
    printf("samples: %d\n", samples);
    printf("tdelay: %d\n", tdelay);

    return 0;
}
