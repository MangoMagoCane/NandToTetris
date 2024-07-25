#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

int checkExtension(char* filename, char** extension);  
int preprocessor(FILE* f_tmp, FILE* f_input);
int processor(FILE* f_output, FILE* f_input);
int insertSymbol(char* name, uint16_t value, bool labeled);
void freeSymtab(); 
void printSymTab(uint16_t start_index);

typedef unsigned int uint;

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

// typedef union{
//     struct {
//         struct {
//             uint GT : 1;
//             uint EQ : 1;
//             uint LT : 1;
//         } jump;
//         struct {
//             uint M : 1;
//             uint D : 1;
//             uint A : 1;
//         } dest;
//         uint comp: 6;
//         uint a : 1;
//         uint opcode : 3;
//     } s;
//     uint16_t u;
// } instruction;
typedef union instruction_t {
    struct {
        uint jump : 3;
        uint dest: 3;
        uint comp: 6;
        uint a : 1;
        uint opcode : 3;
    } s;
    uint16_t u;
} instruction_t;

struct comp_t {
    char* asmbly;
    uint bits : 6;
} comptab[] = {
    "0",   0b101010,
    "1",   0b111111,
    "-1",  0b111010,
    "D",   0b001100,
    "A",   0b110000,
    "!D",  0b001101,
    "!A",  0b110001,
    "-D",  0b001111,
    "-A",  0b110011,
    "D+1", 0b011111,
    "A+1", 0b110111,
    "D-1", 0b001110,
    "A-1", 0b110010,
    "D+A", 0b000010,
    "D-A", 0b010011,
    "A-D", 0b000111,
    "D&A", 0b000000,
    "D|A", 0b010101,
};

struct jump_t {
    char* asmbly;
    uint bits : 3;
} jumptab[] = {
    "\0",  0b000,
    "JGT", 0b001,
    "JEQ", 0b010,
    "JGE", 0b011,
    "JLT", 0b100,
    "JNE", 0b101,
    "JLE", 0b110,
    "JMP", 0b111
};

symbol_t predefined_symbols[] = {
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
#define COMPTAB_LEN sizeof(comptab) / sizeof(comptab[0])
#define JUMPTAB_LEN sizeof(jumptab) / sizeof(jumptab[0])

int symtab_len = SYMBOL_TABLE_START_LEN;
int symtab_next_entry;
symbol_t* symtab;

enum err {
    INVALID_ARG_CNT,
    INVALID_FILE,
    INVALID_FILE_EXTNSN,
    FAILED_MALLOC
};

char input_buf[INPUT_BUFSIZE]; 
char line_buf[LINE_BUFSIZE]; 

// TODO @INSTRUCTIONWITHNUMBER
int main(int argc, char* argv[]) {
    FILE* f_input;
    FILE* f_output;
    FILE* f_tmp;
    int retval = 0;
    char* extension;
    char* output_name;

    symtab_next_entry = PREDEF_SYMTAB_LEN;
    symtab = malloc(SYMBOL_TABLE_START_LEN * sizeof(symbol_t));
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
    if ((f_tmp = fopen("AsmblrTemp.txt", "r+")) == NULL) {
        fprintf(stderr, "ERR: could not create internal temporary file errno: %d\n", errno);
        retval = INVALID_FILE; 
        goto close_output;
    }

    if (preprocessor(f_tmp, f_input) != 0) {
        fprintf(stderr, "Preprocessor ERR");
        goto close_tmp;
    }
    // printSymTab(0);
    rewind(f_tmp);
    if (processor(f_output, f_tmp) != 0) {
        fprintf(stderr, "Processor ERR");
        goto close_tmp;
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

int processor(FILE* f_output, FILE* f_input) {
    instruction_t instruction;
    char* ibuf_p;
    char c;
    int comp;
    int jump;
    int i;
    char foo_buf[1024];
    
    for (int line_num = 1; fgets(input_buf, INPUT_BUFSIZE, f_input) != NULL; line_num++) {
        instruction.u = 0; 
        comp = 0;
        jump = 0;
        ibuf_p = input_buf;

        for (int i = 0; (c = input_buf[i]) != '\0'; i++) {
            if (c == '\n') {
                input_buf[i] = '\0';
                break;
            }
        }
        strcpy(foo_buf, input_buf);
        
        // A-instruction
        if (input_buf[0] == '@') {
            for (int i = 0; i < symtab_next_entry; i++) {
                if (strcmp(symtab[i].name, &input_buf[1]) == 0) {
                    instruction.u |= symtab[i].value.u;
                    instruction.u &= ~(1 << 15);
                }
            }
            goto write_instruction;
        } 
        
        // C-instruction
        instruction.s.opcode |= 0b111;
        for (i = 0; (c = input_buf[i]) != '\n' && c != '\0'; i++) {
            switch (c) {
            case '=':
                if (comp == 0) {
                    comp = i + 1;
                } else {
                    fprintf(stderr, "ERR: invalid line %snumber %d\n", input_buf, line_num);
                    return 1;
                }
                break;
            case ';':
                if (jump == 0) {
                    jump = i + 1;
                } else {
                    fprintf(stderr, "ERR: invalid line %snumber %d\n", input_buf, line_num);
                    return 1;
                }
                break;
            }
        }

        uint shift_val;
        // set dest
        if (comp > 0) {
            for (int i = 0; (c = input_buf[i]) != '='; i++) {
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
                    fprintf(stderr, "ERR: invalid line %snumber %d\n", input_buf, line_num);
                    return 1;
                }
                if ((instruction.s.dest) >> shift_val & 1) {
                    fprintf(stderr, "ERR: invalid line %snumber %d\n", input_buf, line_num);
                    return 1;
                }
                instruction.s.dest |= 1 << shift_val;
            }
        }

        input_buf[i] = '\0';
        // set jump 
        if (jump) {
            input_buf[jump-1] = '\0';
            for (int i = 0; i < JUMPTAB_LEN; i++) {
                if (strcmp(jumptab[i].asmbly, &input_buf[jump]) == 0) {
                    // printf("%-4s", &input_buf[jump]);
                    instruction.s.jump |= jumptab[i].bits;
                } 
            }
        }
        for (i = comp; (c = input_buf[i]) != '\n' && c != '\0'; i++) {
            if (c == 'M') {
                input_buf[i] = 'A'; 
                instruction.s.a |= 0b1;
            }
        }
        for (int i = 0; i < COMPTAB_LEN; i++) {
            if (strcmp(comptab[i].asmbly, &input_buf[comp]) == 0) {
                instruction.s.comp |= comptab[i].bits;
                break;
            } 
        }
write_instruction:
        // printf("%// d %s\n", instruction.s.dest, instruction.s.jump, foo_buf);
        char bit_print_buf[18] = {0};
        for (int i = 0; i < 16; i++) {
            bit_print_buf[15-i] = ((instruction.u >> i) & 1) + '0';
        }
        bit_print_buf[16] = '\n';
        bit_print_buf[17] = '\0';
        // printf("%-18s%s", foo_buf, bit_print_buf);
        fprintf(f_output, bit_print_buf);
    }
   return 0;
}

int preprocessor(FILE* f_tmp, FILE* f_input) {
    int symtab_next_var = SYMBOL_VAR_START_ADD;
    char tmp_buf[INPUT_BUFSIZE + 1], curr, next, c;
    char* ibuf_p;
    char* tbuf_p;
    int ROM_line_num = 0;
    
    for (int line_num = 1; fgets(input_buf, INPUT_BUFSIZE, f_input) != NULL; line_num++) {
        // printf("%s", input_buf);
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
                *--ibuf_p = '\0'; 
                if (*--tbuf_p != ')') {
                    fprintf(stderr, "ERR: invalid label on line %d,\n%s\n", line_num, input_buf);
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
    }

    for (int i = PREDEF_SYMTAB_LEN; i < symtab_next_entry; i++) {
        if (!symtab[i].value.s.labeled) {
            symtab[i].value.s.data = symtab_next_var++;
        }
    }
    return 0;
}

int insertSymbol(char* name, uint16_t value, bool labeled) {
    bool symbol_in_table = false;
    symbol_t* next_sym_p;
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
        symtab = realloc(symtab, symtab_len * sizeof(symbol_t));
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

void printSymTab(uint16_t start_index) {
    char bit_print_buf[17] = {0};
    char* name_buf;
    uint max_name_len = 0;
    uint name_len;

    for (int i = 0; i < symtab_next_entry; i++) {
        name_len = strlen(symtab[i].name);
        if (name_len > max_name_len) {
           max_name_len = name_len; 
        }
    }

    for (int i = start_index; i < symtab_next_entry; i++) {
        for (int j = 0; j < 16; j++) {
            bit_print_buf[15-j] = ((symtab[i].value.u >> j) & 1) + '0';
        }
        if (symtab[i].name == 0) {
            name_buf = "(nil)";
        } else {
            name_buf = symtab[i].name;
        }
        printf("%-4d | %-5d | %c %s | %*s | \n", i, symtab[i].value.s.data, bit_print_buf[0], &bit_print_buf[1], max_name_len * -1, name_buf);
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