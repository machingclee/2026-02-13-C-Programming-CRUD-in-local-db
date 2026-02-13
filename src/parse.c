#include "parse.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "common.h"

int create_db_header(int fd, struct db_header_t** headerOut) {
    // struct db_header_t header = { 0 };
    struct db_header_t* header = calloc(1, sizeof(struct db_header_t));
    if (header == NULL) {
        printf("Calloc failed\n");
        return STATUS_ERROR;
    }
    header->version  = 0x1;
    header->count    = 0;
    header->magic    = HEADER_MAGIC;
    header->filesize = sizeof(struct db_header_t);

    *headerOut = header;
    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct db_header_t** headerOut) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }
    struct db_header_t* header = calloc(1, sizeof(struct db_header_t));
    if (header == NULL) {
        perror("Calloc failed\n");
        return STATUS_ERROR;
    }

    ssize_t bytes_read = read(fd, header, sizeof(struct db_header_t));

    bool valid_bytes_read = bytes_read == sizeof(struct db_header_t);

    if (!valid_bytes_read) {
        perror("valid_bytes_read");
        free(header);
        return STATUS_ERROR;
    }

    // network to host
    // change the decimal representation of the header fields to match the host's endianness
    header->version  = ntohs(header->version);
    header->count    = ntohs(header->count);
    header->magic    = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    bool invalidVersion = header->version != 0x1;
    bool invalidMagic   = header->magic != HEADER_MAGIC;

    struct stat dbstat = { 0 };
    fstat(fd, &dbstat);

    bool invalidFilesize = header->filesize != dbstat.st_size;

    if (invalidVersion || invalidMagic || invalidFilesize) {
        if (invalidVersion) {
            printf("Invalid version: %u\n", header->version);
        }
        if (invalidFilesize) {
            printf("Invalid filesize: %u vs actual %lld\n", header->filesize, dbstat.st_size);
        }
        if (invalidMagic) {
            printf("Invalid magic: 0x%x\n", header->magic);
        }
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;
    return STATUS_SUCCESS;
}

void output_file(int fd, struct db_header_t* header, struct employee_t* employees) {
    header->magic    = htonl(header->magic);
    header->version  = htons(header->version);
    header->count    = htons(header->count);
    header->filesize = htonl(sizeof(struct db_header_t) + sizeof(struct employee_t) * header->count);

    lseek(fd, 0, SEEK_SET);
    int bytesWritten = write(fd, header, sizeof(struct db_header_t));
    for (int i = 0; i < header->count; i++) {
        employees[i].hours = htonl(employees[i].hours);
        bytesWritten += write(fd, &employees[i], sizeof(struct employee_t));
    }
    printf("Wrote %d bytes to file\n", bytesWritten);
    return;
}

int read_employees(int fd, struct db_header_t* header, struct employee_t** employeesOut) {

    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    int count = header->count;

    struct employee_t* employees = calloc(count, sizeof(struct employee_t));

    if (employees == NULL) {
        printf("Malloc Failed");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));

    for (int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;

    return STATUS_SUCCESS;
}

int add_employee(struct db_header_t* header, struct employee_t* employees, char* addstring) {
    char* name  = strtok(addstring, ",");
    char* addr  = strtok(NULL, ",");
    char* hours = strtok(NULL, ",");
    printf("Trying to add the following information into the datbase:\n");
    printf("Name: %s\nAddress: %s\nHours: %s\n", name, addr, hours);

    int currindex = header->count - 1;

    strncpy(employees[currindex].name, name, sizeof(employees[currindex].name));
    strncpy(employees[currindex].address, addr, sizeof(employees[currindex].address));
    employees[currindex].hours = atoi(hours);

    return STATUS_SUCCESS;
}