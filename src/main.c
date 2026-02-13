#include <fcntl.h>
#include <stdio.h>

#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "file.h"
#include "parse.h"

// use gcc -o main main.c
// to compile it

int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 0;
    }

    char* filename = argv[1];

    int fd = open_rw_file(filename);

    if (fd == -1) {
        return -1;
    }
    if (parse_file_header(fd) == -1) {
        return -1;
    }
    
    close(fd);
    return 0;
}
