#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../utilities.h"

// command pop 2
#define C_2 "@SP\nAM=M-1\nD=M\nA=A-1\n" 
#define ADD C_2 "M=D+M\n"
#define SUB C_2 "M=M-D\n"
#define NEG "@SP\nA=M-1\nM=-M\n"
#define AND C_2 "M=M&D\n"
#define OR  C_2 "M=M|D\n"
#define NOT "@SP\nA=M-1\nM=!M\n"

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

#define PUSH_STATIC "@%s.%d\nD=M\n" PUSH_D
#define POP_STATIC  POP_D "@%s.%d\nM=D\n"

#define END_LOOP        "(end)\n@end\n0;JMP\n"
#define COMP_TRUE_JUMP  "(t_jmp)\n@0\nD=!A\nA=M-1\nM=D\n@R13\nA=M\n0;JMP\n"
#define COMP_FALSE_JUMP "(f_jmp)\n@0\nD=A\nA=M-1\nM=D\n@R13\nA=M\n0;JMP\n"

// 256 + 5 = 261 (for the psuedo Sys.init call)
#define START_INSTRUCTS_DIR "@261\nD=A\n@SP\nM=D\n@Sys.init\n0;JMP\n"

#define END_INSTRUCTS   END_LOOP COMP_TRUE_JUMP COMP_FALSE_JUMP

#define CALL_SAVE_SEGS "@LCL\nD=M\n"  PUSH_D "@ARG\nD=M\n"  PUSH_D \
                       "@THIS\nD=M\n" PUSH_D "@THAT\nD=M\n" PUSH_D
#define CALL_FUNC "@%s$ret.%d\nD=A\n" PUSH_D CALL_SAVE_SEGS "@%d\nD=A\n@SP\nD=M-D\n@ARG\nM=D\n@SP\nD=M\n@LCL\nM=D\n@%s\n0;JMP\n(%s$ret.%d)\n"

#define RET_POP_SEGS "@R13\nAM=M-1\nD=M\n@THAT\nM=D\n@R13\nAM=M-1\nD=M\n@THIS\nM=D\n" \
                     "@R13\nAM=M-1\nD=M\n@ARG\nM=D\n@R13\nAM=M-1\nD=M\n@LCL\nM=D\n"
#define RETURN "@LCL\nD=M\n@R13\nM=D\n@5\nA=D-A\nD=M\n@R14\nM=D\n" \
               POP_D "@ARG\nA=M\nM=D\n@ARG\nD=M+1\n@SP\nM=D\n" RET_POP_SEGS "@R14\nA=M\n0;JMP\n"

// POP_D @ARG A=M M=D @ARG D=M+1 @SP M=D *ARG = pop(); SP = ARG + 1

#define LABEL   "(%s$%s)\n"
#define GOTO    "@%s$%s\n0;JMP\n"
#define IF_GOTO POP_D "@%s$%s\nD;JNE\n"

typedef enum _StackCommands {
    PUSH, POP
} StackCommands;

typedef enum _SegmentIndices {
    CALLEE, POINTER, TEMP, STATIC
} SegmentIndices;

#define MAX_STATIC_COUNT 240
#define WRITER_NAME_BUFSIZE 128

static FILE *writer_fp;
static char curr_file_name[WRITER_NAME_BUFSIZE];
static char curr_func_name[WRITER_NAME_BUFSIZE] = "__global__";
static bool is_directory = false;
static uint curr_call_i;

void setWriterOutputFile(FILE *fp, bool is_incoming_dir)
{
    writer_fp = fp;
    is_directory = is_incoming_dir;
}

void setWriterFileName(char *file_name_p)
{
    strncpy(curr_file_name, file_name_p, LENGTHOF(curr_file_name));
    fprintf(writer_fp, "//   --%s START--\n", file_name_p);
}

void setWriterFuncName(char* func_name_p)
{
    strncpy(curr_func_name, func_name_p, LENGTHOF(curr_func_name));
}

void WriteStart()
{
    fprintf(writer_fp, "// --PROGRAM START--\n");
    if (is_directory) {
        fprintf(writer_fp, START_INSTRUCTS_DIR);
    }
}

void WriteEnd()
{
    fprintf(writer_fp, "// -- PROGRAM END--\n");
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
            fprintf(writer_fp, COMP_STRUC, curr_file_name, logical_count, lookup_logical[i][1], curr_file_name, logical_count);
            logical_count++;
            return 0;
        }
    }

    return -1;
}

int writePushPop(char *command_p, char *segment_p, uint index_val)
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

    stack_command_t seg_val;
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
        if (seg_val == POP) {
            return -1;
        }
        fprintf(writer_fp, PUSH_CONSTANT, index_val);
    } else if (strcmp(segment_p, "pointer") == 0) {
        if (index_val > 1) {
            return -1;
        }
        fprintf(writer_fp, lookup_seg[POINTER][seg_val], lookup_pointer[index_val]);
    } else if (strcmp(segment_p, "temp") == 0) {
        if (index_val > 7) {
            return -1;
        }
        fprintf(writer_fp, lookup_seg[TEMP][seg_val], index_val + 5);
    } else if (strcmp(segment_p, "static") == 0) {
        if (static_count > MAX_STATIC_COUNT) {
            return -1;
        }
        fprintf(writer_fp, lookup_seg[STATIC][seg_val], curr_file_name, index_val, curr_file_name, index_val);
        static_count++;
    } else {
        return -1;
    }

    return 0;
}

int writeCall(char *func_name_p, uint arg_count)
{
    fprintf(writer_fp, "// call %s %d\n", func_name_p, arg_count);
    fprintf(writer_fp, CALL_FUNC, curr_func_name, curr_call_i, arg_count + 5, func_name_p, curr_func_name, curr_call_i);
    curr_call_i++;

    return 0;
}

int writeFunction(char *func_name_p, uint var_count)
{
    setWriterFuncName(func_name_p);
    fprintf(writer_fp, "(%s) // function %s %d\n", curr_func_name, func_name_p, var_count);

    if (var_count > 0) {
        fprintf(writer_fp, "D=0\n");
        for (uint i = 0; i < var_count; ++i) {
            fprintf(writer_fp, PUSH_D);
        }
    }
    curr_call_i = 0;

    return 0;
}

int writeReturn()
{
    fprintf(writer_fp, "// return\n");
    fprintf(writer_fp, RETURN);

    return 0;
}

int writeBranching(char *command_p, char *label_p)
{
    fprintf(writer_fp, "// %s %s\n", command_p, label_p);
    if (strcmp(command_p, "label") == 0) {
        fprintf(writer_fp, LABEL, curr_func_name, label_p);
    } else if (strcmp(command_p, "goto") == 0) {
        fprintf(writer_fp, GOTO, curr_func_name, label_p);
    } else if (strcmp(command_p, "if-goto") == 0) {
        fprintf(writer_fp, IF_GOTO, curr_func_name, label_p);
    } else {
        return -1;
    }

    return 0;
}

// c-2: @SP AM=M-1 D=M A=A-1
// add: c-2 M=D+M
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

// @{retAddr} D=A PUSH_D
// @LCL D=M PUSH_D
// @ARG D=M PUSH_D
// @THIS D=M PUSH_D
// @THAT D=M PUSH_D
// @5 D=A @{nArgs} D=D+A @SP D=M-D @ARG M=D
// @SP D=M @LCL M=D
// @{functionName} 0;JMP ({retAddr})

// function functionName nVars
// ({functionName}) 
// D=0 {PUSH_D nVars times} // set up local
// {functionBody}

// return
// @LCL D=M @R13 M=D            endFrame = LCL
// @5 A=D-A D=M @R14 M=D        retAddr = *(endFrame - 5)
// POP_D @ARG A=M M=D D=M+1 @SP M=D *ARG = pop(); SP = ARG + 1
// @R13 AM=M-1 D=M @THAT M=D    THAT = *(endFrame - 1)
// @R13 AM=M-1 D=M @THIS M=D    THIS = *(endFrame - 1)
// @R13 AM=M-1 D=M @ARG  M=D    ARG =  *(endFrame - 1)
// @R13 AM=M-1 D=M @LCL  M=D    LCL =  *(endFrame - 1)
// @R14 A=M 0;JMP               goto retAddr
// retAddr = {functionName$ret.i}

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

