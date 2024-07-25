#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

int checkExtension(char* filename, char** extension);  
int preprocessor(FILE* f_input, FILE* f_tmp);
int insertSymbol(char* name, uint16_t value, bool labeled);
void freeSymtab(); 

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

symbol predefined_symbols[] = {
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

#define INPUT_BUFSIZE 1024
#define LINE_BUFSIZE 1024
#define SYMBOL_TABLE_START_LEN 1024
#define SYMBOL_VAR_START_ADD 16
#define PREDEF_SYMTAB_LEN sizeof(predefined_symbols) / sizeof(predefined_symbols[0])

int symtab_len = SYMBOL_TABLE_START_LEN;
int symtab_next_entry;
symbol* symtab;

enum err {
    INVALID_ARG_CNT,
    INVALID_FILE,
    INVALID_FILE_EXTNSN,
    FAILED_MALLOC
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

    symtab_next_entry = PREDEF_SYMTAB_LEN;
    symtab = malloc(SYMBOL_TABLE_START_LEN * sizeof(symbol));
    if (symtab == NULL) {
        fprintf(stderr, "ERR: could not allocate memory for the symbol table");
        retval = FAILED_MALLOC;
        goto exit;
    }
    memcpy(symtab, predefined_symbols, sizeof(predefined_symbols));

    // printf("%d\n", insertSymbol("dog", 0b10101, false));
    // printf("%d\n", insertSymbol("foo", 0b10101, true));
    // printf("%d\n", insertSymbol("dog", 0b11111, true));
    // printf("%d\n", insertSymbol("foo", 0b0, true));


    if (argc != 2) {
        fprintf(stderr, "usage: ./HackAsmblr file.asm");
        retval = INVALID_ARG_CNT;
        goto free_symtab;
    } 
    if (checkExtension(argv[1], &extension) == 0) {
        fprintf(stderr, "file: %s has invalid extension: %s\n", argv[1], extension);
        retval = INVALID_FILE;
        goto free_symtab;
    }
    if ((f_input = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "cannot open: %s\n", argv[1]);
        retval = INVALID_FILE_EXTNSN; 
        goto free_symtab;
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

    char bit_print_buf[17] = {0};
    char* name_buf;
    for (int i = PREDEF_SYMTAB_LEN; i < symtab_next_entry; i++) {
        for (int j = 0; j < 16; j++) {
            bit_print_buf[15-j] = ((symtab[i].value.u >> j) & 1) + '0';
        }
        if (symtab[i].name == 0) {
            name_buf = "(nil)";
        } else {
            name_buf = symtab[i].name;
        }
        printf("%-4d|%-20s|%-5d|%c|%s\n", i, name_buf, symtab[i].value.s.data, bit_print_buf[0], &bit_print_buf[1]);
    }

close_tmp:
    fclose(f_tmp);
close_output:
    fclose(f_output);
close_input:
    fclose(f_input);
free_symtab:
    freeSymtab();
exit:
    exit(retval);
}

int preprocessor(FILE* f_input, FILE* f_tmp) {
    int symtab_next_var = SYMBOL_VAR_START_ADD;
    char tmp_buf[INPUT_BUFSIZE + 1], curr, next, c;
    char* ibuf_p;
    char* tbuf_p;
    int ROM_line_num = 0;
    int line_num = 1;

    while (fgets(input_buf, INPUT_BUFSIZE, f_input) != NULL) {
        ibuf_p = input_buf;
        tbuf_p = tmp_buf;
        curr = *ibuf_p;
        while (next = *++ibuf_p) {
            if (curr == '/' && next == '/') {
                break;
            }
            if (curr != ' ' && curr != '\t') {
                *tbuf_p++ = curr;
            }
            curr = next;
        } 
        if (tbuf_p == tmp_buf) {
            continue;
        }

        switch (tmp_buf[0]) {
            case '(':
                if (*--tbuf_p != ')') {
                    fprintf(stderr, "ERR: invalid label on line %d\n", line_num);
                    return -1;
                }
                *tbuf_p = '\0';
                insertSymbol(&tmp_buf[1], ROM_line_num, true);
                break;
            case '@':
                *tbuf_p = '\0';
                insertSymbol(&tmp_buf[1], 0, false);
            default:
                *tbuf_p++ = '\n';
                *tbuf_p = '\0';
                fprintf(f_tmp, tmp_buf);
                ROM_line_num++;
        }

        line_num++;
    }

    for (int i = PREDEF_SYMTAB_LEN; i < symtab_next_entry; i++) {
        if (!symtab[i].value.s.labeled) {
            symtab[i].value.s.data = symtab_next_var++;
        }
    }
}

int insertSymbol(char* name, uint16_t value, bool labeled) {
    bool symbol_in_table = false;
    symbol* next_sym_p;
    char* name_buf;

    int i;
    for (i = 0; i < symtab_next_entry; i++) {
        if (strcmp(name, symtab[i].name) == 0) {
            symbol_in_table = true;
            break;
        }
    }

    if (symbol_in_table) {
        if (symtab[i].value.s.labeled) {
            return labeled ? 1 : 0;
        }
        if (labeled) {
            symtab[i].value.s.data = value;
            symtab[i].value.s.labeled = true;
        }
        return 0;
    }
    
    if (symtab_next_entry >= symtab_len) {
        symtab_len *= 2;
        symtab = realloc(symtab, symtab_len * sizeof(symbol));
        if (symtab == NULL) {
            fprintf(stderr, "ERR: could not allocate memory for the symbol table");
            return -1;
        }
    }

    name_buf = malloc(strlen(name) + 1);
    strcpy(name_buf, name);

    next_sym_p = symtab + symtab_next_entry++; 
    next_sym_p->name = name_buf; 
    next_sym_p->value.s.data = value; 
    next_sym_p->value.s.labeled = labeled; 

    return 0;
}

void freeSymtab() {
    for (int i = PREDEF_SYMTAB_LEN; i < symtab_len; i++) {
        free(symtab[i].name);
    }
    free(symtab);
}

int checkExtension(char* filename, char** extension) {
    char *dot = strrchr(filename, '.');
    *extension = dot;
    if (!dot || dot == filename || strcmp(dot, ".asm") != 0) {
        return 0; 
    }
    return 1;
}