// define header guard
#ifndef PARSE_H
#define PARSE_H

#define HEADER_MAGIC 0x4c4c4144

struct db_header_t {
    unsigned int magic;
    unsigned short version;
    unsigned short count;
    unsigned int filesize;
};

struct employee_t {
    char name[256];
    char address[256];
    unsigned int hours;
};

int delete_employee(struct db_header_t* header, struct employee_t** employees, char* name);
int create_db_header(int fd, struct db_header_t** headerOut);
int retrieve_and_validate_db_header(int fd, struct db_header_t** headerOut);
int read_employees(int fd, struct db_header_t*, struct employee_t** employeesOut);
int add_employee(struct db_header_t*, struct employee_t** employees, char* addstring);
int output_file(int fd, struct db_header_t* header, struct employee_t* employees);
void list_employees(struct db_header_t* header, struct employee_t* employees);

#endif // PARSE_H