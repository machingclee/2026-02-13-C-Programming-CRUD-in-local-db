#include "parse.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "common.h"

void list_employees(struct db_header_t* header, struct employee_t* employees) {
    printf("All employees: \n");
    int i = 0;
    for (; i < header->count; i++) {
        struct employee_t* e = employees + i;
        printf("Name:%s, Address:%s, Hours: %d\n", e->name, e->address, e->hours);
    }
}

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

int retrieve_and_validate_db_header(int fd, struct db_header_t** headerOut) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }
    struct db_header_t* header = calloc(1, sizeof(struct db_header_t));
    if (header == NULL) {
        perror("Calloc failed\n");
        return STATUS_ERROR;
    }
    lseek(fd, 0, SEEK_SET);
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

int output_file(int fd, struct db_header_t* header, struct employee_t* employees) {
    if (ftruncate(fd, 0) == -1) {
        perror("ftruncate failed");
        close(fd);
        return -1;
    }

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
    return STATUS_SUCCESS;
}

int read_employees(int fd, struct db_header_t* header, struct employee_t** employeesOut) {

    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }
    lseek(fd, sizeof(struct db_header_t), SEEK_SET);
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

int add_employee(struct db_header_t* header, struct employee_t** employees, char* addstring) {
    if (header == NULL || employees == NULL || *employees == NULL || addstring == NULL) {
        perror("Null Pointer");
        return STATUS_ERROR;
    }

    char* name = strtok(addstring, ",");
    if (name == NULL) {
        return STATUS_ERROR;
    }
    char* addr  = strtok(NULL, ",");
    char* hours = strtok(NULL, ",");
    printf("Trying to add the following information into the datbase:\n");
    printf("Name: %s\nAddress: %s\nHours: %s\n", name, addr, hours);

    struct employee_t* e = realloc(*employees, sizeof(struct employee_t) * (header->count + 1));
    if (e == NULL) {
        return STATUS_ERROR;
    }
    header->count++;
    int lastindex = header->count - 1;
    strncpy(e[lastindex].name, name, sizeof(e[lastindex].name) - 1);
    strncpy(e[lastindex].address, addr, sizeof(e[lastindex].address) - 1);
    e[lastindex].hours = atoi(hours);

    *employees = e;

    return STATUS_SUCCESS;
}

int delete_employee(struct db_header_t* header, struct employee_t** employees, char* name) {
    int delete_index          = 0;
    bool deletion_takes_place = false;

    for (; delete_index < header->count; delete_index++) {
        struct employee_t* e = *employees + delete_index;
        if ((char*)e->name == NULL) {
            perror("e->name == NULL");
            return STATUS_ERROR;
        }
        if (name == NULL) {
            perror("name == NULL");
            return STATUS_ERROR;
        }
        bool name_matched = strcmp(e->name, name) == 0;
        if (name_matched) {
            deletion_takes_place = true;
            break;
        }
    }

    printf("Deleting User: %s\n", (*employees)[delete_index].name);

    if (deletion_takes_place) {
        int old_count = header->count;
        int new_count = header->count - 1;

        struct employee_t* updated_employees = calloc(new_count, sizeof(struct employee_t));

        int i            = 0;
        int new_e_cursor = 0;
        for (; i < old_count; i++) {
            if (i != delete_index) {
                struct employee_t* remained_employee = *employees + i;
                strncpy(updated_employees[new_e_cursor].name, remained_employee->name, sizeof(remained_employee->name) - 1);
                strncpy(updated_employees[new_e_cursor].address, remained_employee->address, sizeof(remained_employee->address) - 1);
                updated_employees[new_e_cursor].hours = remained_employee->hours;

                new_e_cursor++;
            }
        }

        header->count--;
        *employees = updated_employees;
    }
}