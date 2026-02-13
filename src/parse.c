#include "parse.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int parse_file_header(int fd) {
    struct database_header_t head = { 0 };
    struct stat dbStat            = { 0 };

    if (read(fd, &head, sizeof(head)) != sizeof(head)) {
        perror("read");
        close(fd);
        return -1;
    }

    printf("Database Version: %u\n", head.version);
    printf("Number of Employees: %u\n", head.employees);
    printf("File Size: %u bytes\n", head.filesize);

    if (fstat(fd, &dbStat) < 0) {
        perror("stat");
        close(fd);
        return -1;
    };

    printf("Actual File Size: %lld bytes\n", (long long)dbStat.st_size);
    return 0;
}