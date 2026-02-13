#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>

#include "file.h"
#include "parse.h"
#include "main.h"
#include "common.h"
#include <stdlib.h>

void print_usage(char* argv[]) {
    printf("Usage: %s [-n] -f filename\n", argv[0]);
    printf("  -n            Create a new file\n");
    printf("  -f filename   (required) Specify the file path\n");
    return;
}

int main(int argc, char* argv[]) {
    char* filepath  = NULL;
    char* addstring = NULL;
    bool newfile    = false;
    int c;
    int db_fd                    = -1;
    struct db_header_t* header   = NULL;
    struct employee_t* employees = NULL;

    while ((c = getopt(argc, argv, "nf:a:")) != -1) {
        switch (c) {
        case 'n':
            newfile = true;
            break;
        case 'a':
            addstring = optarg;
            break;
        case 'f':
            filepath = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-n] -f filename\n", argv[0]);
            return 1;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a required argument.\n");
        print_usage(argv);
        return 0;
    }

    if (newfile) {
        db_fd = create_db_file(filepath);
        if (db_fd == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return STATUS_ERROR;
        }
        create_db_header(db_fd, &header);
    } else {
        db_fd = open_db_file(filepath);
        if (db_fd == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return STATUS_ERROR;
        }
        int status = validate_db_header(db_fd, &header);
        if (status == STATUS_ERROR) {
            printf("Invalid database file\n");
            return STATUS_ERROR;
        }
    }

    printf("Newfile: %d\n", newfile);
    printf("Filepath: %s\n", filepath);

    if (read_employees(db_fd, header, &employees) != STATUS_SUCCESS) {
        printf("Failed to read employees");
        return 0;
    };

    if (addstring) {
        int newCount  = header->count + 1;
        header->count = newCount;

        employees = realloc(employees, newCount * sizeof(struct employee_t));
        add_employee(header, employees, addstring);
    }

    output_file(db_fd, header, employees);

    return STATUS_SUCCESS;
}
