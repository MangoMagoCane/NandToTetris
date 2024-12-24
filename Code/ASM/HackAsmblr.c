// gcc HackAsmblr.c -Wall -Wextra -g -o HackAsmblr
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include "../utilities.h"

int preprocessor(FILE* f_tmp, FILE* f_input);
int processor(FILE* f_output, FILE* f_input);
int insertSymbol(char* name, uint16_t value, bool labeled);
void freeSymtab();
void printSymTab(uint16_t start_index);

typedef union instruction_t {
    struct {
        uint jump : 3;
        uint dest : 3;
        uint comp : 6;
        uint a : 1;
        uint opcode : 3;
    } s;
    uint16_t u;
} instruction_t;

typedef struct symbol_t {
    char* name;
    union {
        struct {
            uint data : 15;
            uint labeled : 1;
        } s;
        uint16_t u;
    } value;
} symbol_t;

typedef struct comp_t {
    char* asmbly;
    uint bits : 6;
} comp_t;

typedef struct jump_t {
    char* asmbly;
    uint bits : 3;
} jump_t;

symbol_t predefined_symbols[] = {
    { "R0", { { 0, true } } },
    { "R1", { { 1, true } } },
    { "R2", { { 2, true } } },
    { "R3", { { 3, true } } },
    { "R4", { { 4, true } } },
    { "R5", { { 5, true } } },
    { "R6", { { 6, true } } },
    { "R7", { { 7, true } } },
    { "R8", { { 8, true } } },
    { "R9", { { 9, true } } },
    { "R10", { { 10, true } } },
    { "R11", { { 11, true } } },
    { "R12", { { 12, true } } },
    { "R13", { { 13, true } } },
    { "R14", { { 14, true } } },
    { "R15", { { 15, true } } },
    { "SCREEN", { { 16384, true } } },
    { "KBD", { { 24576, true } } },
    { "SP", { { 0, true } } },
    { "LCL", { { 1, true } } },
    { "ARG", { { 2, true } } },
    { "THIS", { { 3, true } } },
    { "THAT", { { 4, true } } }
};

comp_t comptab[] = {
    { "0", 0b101010 },
    { "1", 0b111111 },
    { "-1", 0b111010 },
    { "D", 0b001100 },
    { "A", 0b110000 },
    { "!D", 0b001101 },
    { "!A", 0b110001 },
    { "-D", 0b001111 },
    { "-A", 0b110011 },
    { "D+1", 0b011111 },
    { "A+1", 0b110111 },
    { "D-1", 0b001110 },
    { "A-1", 0b110010 },
    { "D+A", 0b000010 },
    { "D-A", 0b010011 },
    { "A-D", 0b000111 },
    { "D&A", 0b000000 },
    { "D|A", 0b010101 }
};

jump_t jumptab[] = {
    { "\0", 0b000 },
    { "JGT", 0b001 },
    { "JEQ", 0b010 },
    { "JGE", 0b011 },
    { "JLT", 0b100 },
    { "JNE", 0b101 },
    { "JLE", 0b110 },
    { "JMP", 0b111 }
};

#define INPUT_BUFSIZE 1024
#define LINE_BUFSIZE 1024
#define SYMTAB_START_LEN 1024
#define SYMTAB_MAX_UNLABELED 256
#define SYMTAB_MIN_UNLABELED 16

const uint PREDEF_SYMTAB_LEN = LENGTHOF (predefined_symbols);
const uint COMPTAB_LEN = LENGTHOF (comptab);
const uint JUMPTAB_LEN = LENGTHOF (jumptab);

uint symtab_len = SYMTAB_START_LEN;
uint symtab_next_entry;
symbol_t* symtab;

int main(int argc, char* argv[])
{
    FILE* f_input;
    FILE* f_output;
    FILE* f_tmp;
    bool print_symtab_flag = false;
    int retval = 0;

    symtab_next_entry = PREDEF_SYMTAB_LEN;
    symtab = malloc(SYMTAB_START_LEN * sizeof (symbol_t));
    if (symtab == NULL) {
        fprintf(stderr, "ERR: could not allocate memory for the symbol table");
        retval = FAILED_MALLOC;
        goto exit;
    }
    memcpy(symtab, predefined_symbols, sizeof (predefined_symbols));

    if (argc != 2) {
        if (argc == 3 && strcmp(argv[2], "-s") == 0) {
            print_symtab_flag = true;
        } else {
            fprintf(stderr, "usage: ./HackAsmblr file.asm (-s)\n"
                            "-s: print the generated symbol table\n");
            retval = INVALID_ARG_CNT;
            goto free_symtab;
        }
    }
 
    char* extension_p;
    if (checkExtension(argv[1], &extension_p, "asm") == 0) {
        fprintf(stderr, "file: %s has invalid extension: %s\n", argv[1], extension_p);
        retval = INVALID_FILE_EXTNSN;
        goto free_symtab;
    }
    if ((f_input = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "cannot open: %s\n", argv[1]);
        retval = INVALID_FILE;
        goto free_symtab;
    }
    extension_p;

    char* output_name = malloc(strlen(argv[1]) + 6);
    sprintf(output_name, "%s%s", argv[1], ".hack");
    f_output = fopen(output_name, "w");
    free(output_name);

    if (f_output == NULL) {
        fprintf(stderr, "could not open output file: %s\n", argv[1]);
        retval = INVALID_FILE;
        goto close_input;
    }
    // if ((f_tmp = tmpfile()) == NULL) {
    if ((f_tmp = fopen("AsmblrTemp.txt.asm", "w+")) == NULL) {
        fprintf(stderr, "ERR: could not create internal temporary file, errno: %d\n", errno);
        retval = INVALID_FILE;
        goto close_output;
    }

    if (preprocessor(f_tmp, f_input) != 0) {
        fprintf(stderr, "ERR: preprocessor\n");
        goto close_tmp;
    }
    if (print_symtab_flag) {
        printSymTab(0);
    }

    rewind(f_tmp);
    if (processor(f_output, f_tmp) != 0) {
        fprintf(stderr, "ERR: processor\n");
        goto close_tmp;
    }

close_tmp:
    fclose(f_tmp);
    // remove("AsmblrTemp.txt.asm");
close_output:
    fclose(f_output);
close_input:
    fclose(f_input);
free_symtab:
    freeSymtab();
exit:
    exit(retval);
}

int processor(FILE* f_output, FILE* f_input)
{
    char input_buf[INPUT_BUFSIZE];
    for (uint line_num = 1; fgets(input_buf, INPUT_BUFSIZE, f_input) != NULL; line_num++) {
        instruction_t instruction = {0};
        int comp_i = 0;
        int jump_i = 0;
        int ibuf_i;
        char c;
        char *ibuf_p = input_buf;

        input_buf[strcspn(input_buf, "\n")] = '\0';

        // A-instruction
        if (input_buf[0] == '@') {
            int value = strtol(&input_buf[1], &ibuf_p, 10);
            if (&input_buf[1] == ibuf_p) {
                for (uint i = 0; i < symtab_next_entry; i++) {
                    if (strcmp(symtab[i].name, &input_buf[1]) == 0) {
                        value = symtab[i].value.u;
                    }
                }
            }
            instruction.u |= value;
            instruction.u &= ~(1 << 15);
            goto write_instruction;
        }

        // C-instruction
        instruction.s.opcode = 0b111;
        for (ibuf_i = 0; (c = input_buf[ibuf_i]) != '\n' && c != '\0'; ibuf_i++) {
            switch (c) {
            case '=':
                if (comp_i == 0 && jump_i == 0) {
                    comp_i = ibuf_i + 1;
                } else {
                    fprintf(stderr, "ERR: Invalid syntax on line: %d\n\"%s\"\n", line_num, input_buf);
                    return -1;
                }
                break;
            case ';':
                if (jump_i == 0) {
                    jump_i = ibuf_i + 1;
                } else {
                    fprintf(stderr, "ERR: Invalid syntax on line: %d\n\"%s\"\n", line_num, input_buf);
                    return -1;
                }
                break;
            }
        }

        // set dest, only if there's an = sign
        uint shift_val;
        if (comp_i > 0) {
            for (uint i = 0; (c = input_buf[i]) != '='; i++) {
                switch (c) {
                case 'M':
                    shift_val = 0;
                    break;
                case 'D':
                    shift_val = 1;
                    break;
                case 'A':
                    shift_val = 2;
                    break;
                default:
                    fprintf(stderr, "ERR: Invalid destination on line: %s number: %d\n",
                            input_buf, line_num);
                    return -1;
                }
                if (instruction.s.dest >> shift_val & 1) {
                    fprintf(stderr, "ERR: Multiple same destinations on line: %s number: %d\n",
                            input_buf, line_num);
                    return -1;
                }
                instruction.s.dest |= 1 << shift_val;
            }
        }
        input_buf[ibuf_i] = '\0';

        // set jump
        if (jump_i) {
            input_buf[jump_i - 1] = '\0';
            for (uint i = 0; i < JUMPTAB_LEN; i++) {
                if (strcmp(jumptab[i].asmbly, &input_buf[jump_i]) == 0) {
                    instruction.s.jump = jumptab[i].bits;
                }
            }
        }

        // set comp
        for (uint i = comp_i; (c = input_buf[i]) != '\0'; i++) {
            if (c == 'M') {
                input_buf[i] = 'A';
                instruction.s.a = 0b1;
                break;
            }
        }
        for (uint i = 0; i < COMPTAB_LEN; i++) {
            if (strcmp(comptab[i].asmbly, &input_buf[comp_i]) == 0) {
                instruction.s.comp = comptab[i].bits;
                break;
            }
        }

write_instruction:
        char bit_print_buf[18] = { 0 };
        for (uint i = 0; i < 16; i++) {
            bit_print_buf[15 - i] = ((instruction.u >> i) & 1) + '0';
        }
        bit_print_buf[16] = '\n';
        bit_print_buf[17] = '\0';
        fprintf(f_output, bit_print_buf);
    }
    return 0;
}

int preprocessor(FILE* f_tmp, FILE* f_input)
{
    char input_buf[INPUT_BUFSIZE];
    char tmp_buf[INPUT_BUFSIZE + 1];
    int ROM_line_num = 0;

    for (int line_num = 1; fgets(input_buf, INPUT_BUFSIZE, f_input) != NULL; line_num++) {
        char* ibuf_p = input_buf;
        char* tbuf_p = tmp_buf;
        char curr = *ibuf_p;
        char next;

        while ((next = *++ibuf_p)) {
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
            *--ibuf_p = '\0';
            if (*--tbuf_p != ')') {
                fprintf(stderr, "ERR: Invalid label on line: %d,\n%s\n", line_num, input_buf);
                return -1;
            }
            *tbuf_p = '\0';
            insertSymbol(&tmp_buf[1], ROM_line_num, true);
            break;
        case '@':
            *tbuf_p = '\0';
            char* strtol_p;
            strtol(&tmp_buf[1], &strtol_p, 10);
            if (&tmp_buf[1] == strtol_p) {
                *tbuf_p = '\0';
                insertSymbol(&tmp_buf[1], 0, false);
            }
            [[fallthrough]];
        default:
            *tbuf_p++ = '\n';
            *tbuf_p = '\0';
            fprintf(f_tmp, tmp_buf);
            ROM_line_num++;
        }
    }

    int symtab_next_unlabeled = SYMTAB_MIN_UNLABELED;
    for (uint i = PREDEF_SYMTAB_LEN; i < symtab_next_entry; i++) {
        if (!symtab[i].value.s.labeled) {
            if (symtab_next_unlabeled >= SYMTAB_MAX_UNLABELED) {
                fprintf(stderr, "ERR: Maximum unlabeled symbol count %d reached", SYMTAB_MAX_UNLABELED);
            }
            symtab[i].value.s.data = symtab_next_unlabeled++;
        }
    }
    return 0;
}

int insertSymbol(char* name, uint16_t value, bool labeled)
{
    bool symbol_in_table = false;
    char* name_buf;
    uint symtab_i;

    for (symtab_i = 0; symtab_i < symtab_next_entry; symtab_i++) {
        if (strcmp(name, symtab[symtab_i].name) == 0) {
            symbol_in_table = true;
            break;
        }
    }

    if (symbol_in_table) {
        if (symtab[symtab_i].value.s.labeled) {
            return labeled ? -1 : 0;
        }
        if (labeled) {
            symtab[symtab_i].value.s.data = value;
            symtab[symtab_i].value.s.labeled = true;
        }
        return 0;
    }

    if (symtab_next_entry >= symtab_len) {
        symtab_len *= 2;
        symtab = realloc(symtab, symtab_len * sizeof (symbol_t));
        if (symtab == NULL) {
            fprintf(stderr, "ERR: could not allocate memory for the symbol table");
            return -1;
        }
    }

    name_buf = malloc(strlen(name) + 1);
    strcpy(name_buf, name);

    symtab[symtab_next_entry].name = name_buf;
    symtab[symtab_next_entry].value.s.data = value;
    symtab[symtab_next_entry].value.s.labeled = labeled;
    symtab_next_entry++;

    return 0;
}

void freeSymtab()
{
    for (uint i = PREDEF_SYMTAB_LEN; i < symtab_len; i++) {
        free(symtab[i].name);
    }
    free(symtab);
}

void printSymTab(uint16_t start_index)
{
    char bit_print_buf[17] = { 0 };
    char nameDashes[128] = { 0 };
    uint max_name_len = 6;
    char* name_buf;

    for (uint i = 0; i < symtab_next_entry; i++) {
        uint name_len = strlen(symtab[i].name);
        if (name_len > max_name_len) {
            max_name_len = name_len;
        }
    }
    for (uint i = 0; i < max_name_len + 38; i++) {
        nameDashes[i] = '_';
    }

    printf("_%s_\n", nameDashes);
    printf("| %-5s | %-5s | %-17s | %*s |\n",
           "i", "val", "bits (labeled=1)", -max_name_len, "symbol");
    for (uint i = start_index; i < symtab_next_entry; i++) {
        for (uint j = 0; j < 16; j++) {
            bit_print_buf[15 - j] = ((symtab[i].value.u >> j) & 1) + '0';
        }
        if (symtab[i].name == 0) {
            name_buf = "(nil)";
        } else {
            name_buf = symtab[i].name;
        }
        printf("| %-5d | %-5d | %c %s | %*s |\n",
               i, symtab[i].value.s.data, bit_print_buf[0], &bit_print_buf[1], -max_name_len, name_buf);
    }
    printf("|%s|\n", nameDashes);
}

