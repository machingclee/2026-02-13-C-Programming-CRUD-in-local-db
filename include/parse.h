// define header guard
#ifndef PARSE_H
#define PARSE_H

struct database_header_t {
    unsigned short version;
    unsigned short employees;
    unsigned int filesize;
};

int parse_file_header(int fd);

#endif // PARSE_H