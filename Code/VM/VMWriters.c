#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utilities.h"

// command pop 2
#define C_2 "@SP\nAM=M-1\nD=M\nA=A-1\n" 
#define ADD C_2 "M=M+D\n"
#define SUB C_2 "M=M-D\n"
#define NEG "@SP\nA=M-1\nM=!M\n"
#define AND C_2 "M=M&D\n"
#define OR  C_2 "M=M|D\n"
#define NOT "@SP\nA=M-1\nM=-M\n"

#define COMP_STRUC "@%s_%d\nD=A\n@R13\nM=D\n" C_2 "D=M-D\n@t_jmp\nD;%s\n@f_jmp\n0;JMP\n(%s_%d)\n"

#define PUSH_D  "@SP\nM=M+1\nA=M-1\nM=D\n"
#define POP_D   "@SP\nAM=M-1\nD=M\n"
#define POP_R13 POP_D "@R13\nM=D\n"

#define PUSH_CALLEE_SAVED "@%d\nD=A\n@%s\nA=D+M\nD=M\n" PUSH_D
#define POP_CALLEE_SAVED  POP_R13 "@%d\nD=A\n@%s\nD=D+M\n@R14\nM=D\n@R13\nD=M\n@R14\nA=M\nM=D\n"

#define PUSH_POINTER "@%s\nD=M\n" PUSH_D
#define POP_POINTER  POP_D "@%s\nM=D\n"

#define PUSH_TEMP "@%d\nD=M\n" PUSH_D
#define POP_TEMP  POP_D "@%d\nM=D\n"

#define PUSH_CONSTANT "@%d\nD=A\n" PUSH_D

#define MAX_STATIC_COUNT 240
#define PUSH_STATIC "@%s.%d\nD=M\n" PUSH_D
#define POP_STATIC  POP_D "@%s.%d\nM=D\n"

#define END_LOOP        "(end)\n@end\n0;JMP\n"
#define COMP_TRUE_JUMP  "(t_jmp)\n@0\nD=!A\nA=M-1\nM=D\n@R13\nA=M\n0;JMP\n"
#define COMP_FALSE_JUMP "(f_jmp)\n@0\nD=A\nA=M-1\nM=D\n@R13\nA=M\n0;JMP\n"

#define END_INSTRUCTS END_LOOP COMP_TRUE_JUMP COMP_FALSE_JUMP "\n"

enum push_pop {
    PUSH, POP
};

enum segment_indices {
    CALLEE, POINTER, TEMP, STATIC
};

static FILE* writer_fp;
static char* writer_fn_p;

void setWriterFile(FILE* fp, char* filename)
{
    writer_fp = fp;
    writer_fn_p = filename;
}

void endWriterFile()
{
    fprintf(writer_fp, "// --END--\n");
    fprintf(writer_fp, END_INSTRUCTS);
}

int writeArithLogical(char* command_p)
{
    static uint logical_count = 0;
    static char* lookup_logical[6][2] = {
        { "eq", "JEQ" }, { "lt", "JLT" }, { "gt", "JGT" },
    };
    static char* lookup_arith[6][2] = {
        { "add", ADD }, { "sub", SUB }, { "neg", NEG },
        { "and", AND }, { "or",  OR },  { "not", NOT }
    };

    fprintf(writer_fp, "// %s\n", command_p);
    for (uint i = 0; i < 6; ++i) {
        if (strcmp(command_p, lookup_arith[i][0]) == 0) {
            fprintf(writer_fp, "%s", lookup_arith[i][1]);
            return 0;
        }
    }
    for (uint i = 0; i < 6; ++i) {
        if (strcmp(command_p, lookup_logical[i][0]) == 0) {
            fprintf(writer_fp, COMP_STRUC, writer_fn_p, logical_count, lookup_logical[i][1], writer_fn_p, logical_count);
            logical_count++;
            return 0;
        }
    }

    return -1;
}

int writePushPop(char* command_p, char* segment_p, char* index_p)
{
    static uint static_count = 0;
    static char* lookup_pointer[2] = { "THIS", "THAT" };
    static char* lookup_cs[4][2] = { // callee saved
        {"local", "LCL"}, {"argument", "ARG"}, {"this", "THIS"}, {"that", "THAT"}
    };
    static char* lookup_seg[4][2] = {
        { PUSH_CALLEE_SAVED, POP_CALLEE_SAVED }, { PUSH_POINTER, POP_POINTER },
        { PUSH_TEMP, POP_TEMP }, { PUSH_STATIC, POP_STATIC }
    };

    char* index_end_p;
    int index_val = strtol(index_p, &index_end_p, 10);
    if (index_val < 0 || *index_end_p != '\0') {
        return -1;
    }

    uint seg_val;
    if (strcmp(command_p, "push") == 0) {
        seg_val = PUSH;
    } else if (strcmp(command_p, "pop") == 0) {
        seg_val = POP;
    } else {
        return -1;
    }

    fprintf(writer_fp, "// %s %s %d\n", command_p, segment_p, index_val);
    for (uint i = 0; i < 4; ++i) {
        if (strcmp(segment_p, lookup_cs[i][0]) == 0) {
            fprintf(writer_fp, lookup_seg[CALLEE][seg_val], index_val, lookup_cs[i][1]);
            return 0;
        }
    }
    if (strcmp(segment_p, "constant") == 0) {
        if (seg_val == POP) return -1;
        fprintf(writer_fp, PUSH_CONSTANT, index_val);
    } else if (strcmp(segment_p, "pointer") == 0) {
        if (index_val > 1) return -1;
        fprintf(writer_fp, lookup_seg[POINTER][seg_val], lookup_pointer[index_val]);
    } else if (strcmp(segment_p, "temp") == 0) {
        if (index_val > 7) return -1;
        fprintf(writer_fp, lookup_seg[TEMP][seg_val], index_val + 5);
    } else if (strcmp(segment_p, "static") == 0) {
        if (static_count > MAX_STATIC_COUNT) return -1;
        fprintf(writer_fp, lookup_seg[STATIC][seg_val], writer_fn_p, index_val, writer_fn_p, index_val);
        static_count++;
    } else {
        return -1;
    }

    return 0;
}

// c-2: @SP AM=M-1 D=M A=A-1
// add: c-2 M=M+D
// sub: c-2 M=M-D
// neg: @SP A=M-1 M=!M

// and: c-2 M=M&D
// or:  c-2 M=M|D
// not: @SP A=M-1 M=-M

// comparison structure (eq, gt, lt), R13=jump number:
//  @{instruction_num+14} D=A @R13 M=D c-2 D=M-D
//  @t_jmp D;{JEQ, JGT, JLT} @f_jmp 0;jmp {jump dest}

// push-D: @SP M=M+1 A=M-1 M=D
// pop-D: @SP AM=M-1 D=M
// pop-R13: {pop-D} @R13 M=D

// {LCL, ARG, THIS, THAT}
// push basic-4 i: @{i} D=A @{SEG} A=D+M D=M {push-D}
// pop  basic-4 i: {pop-R13} @{i} D=A @{SEG} D=D+M @R14 M=D @R13 D=M @R14 A=M M=D

// 0 = THIS, 1 = THAT
// push pointer i: @{THIS, THAT} D=M {push-D}
// pop  pointer i: {pop-D} @{THIS, THAT} M=D

// 0<=i<=7
// push temp i: @{i+5} D=M {push-D}
// pop  temp i: {pop-D} @{i+5} M=D

// push constant i: @{i} D=A {push-D}

// limit 240
// push static i: @{FileName.i} D=M push-D
// pop  static i: {pop-D} @{FileName.i} M=D

// (end)
// @end
// 0;jmp
// (t_jmp)
// @0 // only works if @SP is RAM[0]
// D=!A
// A=M-1
// M=D
// @R13
// A=M
// 0;jmp
// (f_jmp)
// @0
// D=A
// A=M-1
// M=D
// @R13
// A=M
// 0;jmp
