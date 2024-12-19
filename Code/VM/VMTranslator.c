// gcc VMTranslator.c -Wall -Wextra -g -o VMTranslator
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <uchar.h>
#include "VMWriters.c"
#include "utilities.h"

int parseFile(FILE* f_input, char* filename_p);
 
#define TOK_BUFSIZE 16
#define INPUT_BUFSIZE 1024

int main(int argc, char* argv[])
{
    err_t retval = 0;
    if (argc != 2) {
        fprintf(stderr, "usage: ./VMTranslator (file.vm, dir)\n");
        retval = INVALID_ARG_CNT;
        goto exit;
    }

    bool parse_dir = false;
    char* extension_p;
    char* path_p = getFileName(argv[1]);
    if (checkExtension(path_p, &extension_p, "vm") == false) {
        if (extension_p[0] == '.') {
            fprintf(stderr, "file path: %s has invalid extension: %s\n", path_p, extension_p);
            retval = INVALID_FILE_EXTNSN;
            goto exit;
        }
        parse_dir = true;
    }

    FILE* f_input;
    DIR* dir_input;
    char output_name[MAX_PATH];
    if (parse_dir) {
        if ((dir_input = opendir(argv[1])) == NULL) {
            fprintf(stderr, "cannot open: %s\n", argv[1]);
            retval = INVALID_DIR;
            goto exit;
        }
        sprintf(output_name, "%s/%s%s", path_p, argv[1], ".asm");
    } else {
        if ((f_input = fopen(argv[1], "r")) == NULL) {
            fprintf(stderr, "cannot open: %s\n", argv[1]);
            retval = INVALID_FILE;
            goto exit;
        }
        extension_p[0] = '\0';
        sprintf(output_name, "%s%s", argv[1], ".asm");
    }

    FILE* f_output = fopen(output_name, "w");
    if (f_output == NULL) {
        fprintf(stderr, "cannot open output file: %s\n", argv[1]);
        retval = INVALID_FILE;
        goto close_input;
    }
    setWriterOutputFile(f_output, parse_dir);

    WriteStart();
    if (parse_dir) {
        struct dirent* dirent_p;
        while ((dirent_p = readdir(dir_input)) != NULL) {
            char d_name[NAME_MAX];
            char path_name[PATH_MAX];
            strncpy(d_name, dirent_p->d_name, sizeof (d_name));
            sprintf(path_name, "%s/%s", path_p, d_name);
            if (checkExtension(d_name, &extension_p, "vm") == false) continue;
            if ((f_input = fopen(path_name, "r")) == NULL) {
                fprintf(stderr, "cannot open: %s\n", d_name);
                retval = INVALID_FILE;
                break;
            }
            extension_p[0] = '\0';
            parseFile(f_input, d_name);
        }
    } else {
        if (parseFile(f_input, path_p) != 0) {
            fprintf(stderr, "ERR: parseFile\n");
        }
    }
    WriteEnd();

    fclose(f_output);
close_input:
    if (parse_dir) {
        closedir(dir_input);
    } else {
        fclose(f_input);
    }
exit:
    exit(retval);
}

int parseFile(FILE* f_input, char* file_name_p)
{
    char input_buf[INPUT_BUFSIZE];
    char strtok_buf[INPUT_BUFSIZE];
    setWriterFileName(file_name_p);
    for (uint line_num = 1; fgets(input_buf, INPUT_BUFSIZE, f_input) != NULL; ++line_num) {
        char c;
        for (uint i = 0; (c = input_buf[i]); ++i) {
            if (c == '\n' || (c == '/' && input_buf[i+1] == '/')) {
                input_buf[i] = '\0';
                break;
            }
        }
        strncpy(strtok_buf, input_buf, sizeof (strtok_buf));

        char* c_p = strtok(strtok_buf, " \t\n"); // command  pop   call
        if (c_p == NULL) continue;
        char* s_p = strtok(NULL, " \t\n");       // segment  local power
        char* i_p = strtok(NULL, " \t\n");       // index    3
        char* o_p = strtok(NULL, " \t\n");       // overflow random

        int writer_retval = 0;
        uint tok_count = (c_p ? 1 : 0) + (s_p ? 1 : 0) + (i_p ? 1 : 0) + (o_p ? 1 : 0);

        switch (tok_count) {
        case 1:
            if (strcmp(c_p, "return") == 0) {
                writeReturn();
            } else {
                writer_retval = writeArithLogical(c_p);
            }
            break;
        case 2:
            writer_retval = writeBranching(c_p, s_p);
            break;
        case 3: ;
            char* index_end_p;
            int index_val = strtol(i_p, &index_end_p, 10);
            if (index_val < 0 || *index_end_p != '\0') {
                writer_retval = -1;
                break;
            }
            if (strcmp(c_p, "function") == 0) {
                writer_retval = writeFunction(s_p, index_val);
            } else if (strcmp(c_p, "call") == 0) {
                writer_retval = writeCall(s_p, index_val);
            } else {
                writer_retval = writePushPop(c_p, s_p, index_val);
            }
            break;
        default:
            fprintf(stderr, "ERR: Invalid syntax on line: %d\n\"%s\"\n", line_num, input_buf);
            return -1;
        }

        if (writer_retval == -1) {
            fprintf(stderr, "%s\n", input_buf);
            return -1;
        }
    }

    return 0;
}
