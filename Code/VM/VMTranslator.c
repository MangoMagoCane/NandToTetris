// gcc VMTranslator.c -Wall -Wextra -g -o VMTranslator
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include "VMWriters.c"
#include "utilities.h"

int processor(FILE* f_output, FILE* f_input, char* filename);
 
#define TOK_BUFSIZE 16
#define INPUT_BUFSIZE 1024

int main(int argc, char* argv[])
{
    FILE* f_input;
    FILE* f_output;
    char* filename = getFilename(argv[1]);
    int retval = 0;

    if (argc != 2) {
        fprintf(stderr, "usage: ./VMTranslator file.vm\n");
        retval = INVALID_ARG_CNT;
        goto exit;
    }

    char* extension_p;
    if (checkExtension(filename, &extension_p, "vm") == 0) {
        fprintf(stderr, "file: %s has invalid extension: %s\n", filename, extension_p);
        retval = INVALID_FILE_EXTNSN;
        goto exit;
    }
    if ((f_input = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "cannot open: %s\n", argv[1]);
        retval = INVALID_FILE;
        goto exit;
    }
    extension_p[0] = '\0';

    char* output_name = malloc(strlen(argv[1]) + 6);
    sprintf(output_name, "%s%s", argv[1], ".asm");
    f_output = fopen(output_name, "w");
    free(output_name);

    if (f_output == NULL) {
        fprintf(stderr, "cannot open output file: %s\n", argv[1]);
        retval = INVALID_FILE;
        goto close_input;
    }
    if (processor(f_output, f_input, filename) != 0) {
        fprintf(stderr, "ERR: processor\n");
    }

    fclose(f_output);
close_input:
    fclose(f_input);
exit:
    exit(retval);
}

int processor(FILE* f_output, FILE* f_input, char* filename)
{
    char input_buf[INPUT_BUFSIZE];
    char strtok_buf[INPUT_BUFSIZE];
    setWriterFile(f_output, filename);
    for (uint line_num = 1; fgets(input_buf, INPUT_BUFSIZE, f_input) != NULL; ++line_num) {
        input_buf[strcspn(input_buf, "\n")] = '\0';
        strncpy(strtok_buf, input_buf, sizeof (strtok_buf));

        char* c_p = strtok(strtok_buf, " \t\n"); // command  pop   call
        char* s_p = strtok(NULL, " \t\n");       // segment  local power
        char* i_p = strtok(NULL, " \t\n");       // index    3
        char* o_p = strtok(NULL, " \t\n");       // overflow random
        bool null_index_p = false;

        if (c_p == NULL || (c_p[0] == '/' && c_p[1] == '/')) {
            continue;
        } else if (s_p != NULL && s_p[0] == '/' && s_p[1] == '/') {
            i_p[0] = '\0';
            null_index_p = true;
        } else if (null_index_p || (s_p != NULL && s_p[0] == '/' && s_p[1] == '/')) {
            i_p[0] = '\0';
        }

        uint tok_count = (c_p ? 1 : 0) + (s_p ? 1 : 0) + (i_p ? 1 : 0) + (o_p ? 1 : 0);
        int writer_retval;
        // printf("%d\n", tok_count);
        switch (tok_count) {
        case 1:
            writer_retval = writeArithLogical(c_p);
            break;
        case 3:
            writer_retval = writePushPop(c_p, s_p, i_p);
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
    endWriterFile();

    return 0;
}
