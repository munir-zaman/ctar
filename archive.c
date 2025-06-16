#include <stdio.h>
#include <sys/stat.h>
int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *fp = fopen(argv[1], "r");
        struct stat fp_stat;
        stat(argv[1], &fp_stat);
    }
    return 1;
}
