#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

#define INPUT_BUFSIZE 1024
#define LINE_BUFSIZE 1024
#define SYMBOL_TABLE_START_LEN 1024

int checkExtension(char* filename, char** extension);  
void preprocessor(FILE* f_input, FILE* f_tmp);

typedef unsigned int uint;

typedef struct symbol {
    char* name;
    union {
        struct {
            uint data : 15; 
            uint labeled : 1;
        } s;
        uint16_t u; 
    } value;
} symbol;

// 23 Reserved symbols
int symtab_len = SYMBOL_TABLE_START_LEN;
int symtab_next_var = 16;
int symtab_next_entry = 23; 
symbol symbol_table[SYMBOL_TABLE_START_LEN] = {
    "R0", 0, 1, 
    "R1", 1, 1, 
    "R2", 2, 1, 
    "R3", 3, 1, 
    "R4", 4, 1, 
    "R5", 5, 1, 
    "R6", 6, 1, 
    "R7", 7, 1, 
    "R8", 8, 1, 
    "R9", 9, 1, 
    "R10", 10, 1, 
    "R11", 11, 1, 
    "R12", 12, 1, 
    "R13", 13, 1, 
    "R14", 14, 1, 
    "R15", 15, 1,
    "SCREEN", 16384, 1,
    "KBD", 24576, 1,
    "SP", 0, 1,
    "LCL", 1, 1,
    "ARG", 2, 1,
    "THIS", 3, 1,
    "THAT", 4, 1,
    };

enum err {
    INVALID_ARG_CNT,
    INVALID_FILE,
    INVALID_FILE_EXTNSN,
};

char input_buf[INPUT_BUFSIZE]; 
char line_buf[LINE_BUFSIZE]; 

int main(int argc, char* argv[]) {
    FILE* f_input;
    FILE* f_output;
    FILE* f_tmp;
    int retval = 0;
    char* extension;
    char* output_name;
    for (int i = 0; i < 28; i++) {
        printf("%d\t", i);
        for (int j = 15; j >= 0; j--) {
            printf("%d", (symbol_table[i].value.u >> j) & 1);
        }
        printf("\n");
    }
    
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
    output_name = malloc(strlen(argv[1]) + 6);
    sprintf(output_name, "%s%s", argv[1], ".hack");
    f_output = fopen(output_name, "w");
    free(output_name);
    if (f_output == NULL) {
        fprintf(stderr, "cannot open output file: %s\n", argv[1]);
        retval = INVALID_FILE; 
        goto close_input;
    }
    // if ((f_tmp = tmpfile()) == NULL) {
    if ((f_tmp = fopen("AsmblrTemp.txt", "w+")) == NULL) {
        fprintf(stderr, "ERR: could not create internal temporary file errno: %d\n", errno);
        retval = INVALID_FILE; 
        goto close_output;
    }

    preprocessor(f_input, f_tmp);


close_tmp:
    fclose(f_tmp);
close_output:
    fclose(f_output);
close_input:
    fclose(f_input);
exit:
    exit(retval);
}

void preprocessor(FILE* f_input, FILE* f_tmp) {
    char buf[INPUT_BUFSIZE];
    while (fgets(input_buf, INPUT_BUFSIZE, f_input) != NULL) {
        char* ibuf_p = input_buf;
        char* buf_p = buf;
        char next, curr;
        curr = *ibuf_p;
        while (next = *++ibuf_p) {
            if (curr == '/' && next == '/') {
                break;
            }
            if (curr != ' ' && curr != '\t') {
                *buf_p++ = curr;
            }
            curr = next;
        } 
        if (buf_p != buf) {
            *buf_p++ = '\n';
        }
        *buf_p = '\0';
        fprintf(f_tmp, buf);
    }
}

int checkExtension(char* filename, char** extension) {
    char *dot = strrchr(filename, '.');
    *extension = dot;
    if (!dot || dot == filename || strcmp(dot, ".asm") != 0) {
        return 0; 
    }
    return 1;
}