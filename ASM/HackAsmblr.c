#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

enum err {
    INVALID_ARG_CNT,
    INVALID_FILE,
    INVALID_FILE_EXTNSN
};

int checkExtension(char*, char**);

int main(int argc, char* argv[]) {
    int retval = 0;
    FILE* f_input;
    FILE* f_output;
    char* extension;
    char* output_name;
 
    if (argc != 2) {
        fprintf(stderr, "usage: ./HackAsmblr file.asm");
        retval = INVALID_ARG_CNT;
        goto exit;
    } 
    if (checkExtension(argv[1], &extension) == 0) {
        fprintf(stderr, "file: %s has invalid extension: %s\n", argv[1], extension);
        retval = INVALID_FILE;
        goto exit;
    }
    if ((f_input = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "cannot open: %s\n", argv[1]);
        retval = INVALID_FILE_EXTNSN; 
        goto exit;
    }
    extension[0] = '\0';
    output_name = malloc(strlen(argv[1]) + 8);
    sprintf(output_name, "%s%s", argv[1], ".hack");
    if ((f_output = fopen(output_name, "w")) == NULL) {
        fprintf(stderr, "cannot open output file: %s\n", argv[1]);
        retval = INVALID_FILE; 
        goto close_input;
    }

close_output:
    fclose(f_output);
close_input:
    fclose(f_input);
exit:
    exit(retval);
}

int checkExtension(char* filename, char** extension) {
    char *dot = strrchr(filename, '.');
    *extension = dot;
    if (!dot || dot == filename || strcmp(dot, ".asm") != 0) {
        return 0; 
    }
    return 1;
}