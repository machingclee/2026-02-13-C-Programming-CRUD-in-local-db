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
    printf("  -a addstring  Add data in name,address,hours format\n");
    printf("  -d            Delete the employee by name\n");
    printf("  -l            List the employees\n");
    return;
}

int main(int argc, char* argv[]) {
    char* filepath             = NULL;
    char* addstring            = NULL;
    char* delete_employee_name = NULL;
    bool newfile               = false;
    bool list                  = false;
    bool delete                = false;
    int c;
    int db_fd                    = -1;
    struct db_header_t* header   = NULL;
    struct employee_t* employees = NULL;

    while ((c = getopt(argc, argv, "nf:a:d:l")) != -1) {
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
        case 'l':
            list = true;
            break;
        case 'd':
            delete               = true;
            delete_employee_name = optarg;
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
        int status = retrieve_and_validate_db_header(db_fd, &header);
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
        add_employee(header, &employees, addstring);
    }

    if (list) {
        list_employees(header, employees);
    }

    if (delete) {
        delete_employee(header, &employees, delete_employee_name);
    }

    printf("Latest count: %d\n", header->count);

    output_file(db_fd, header, employees);

    return STATUS_SUCCESS;
}
