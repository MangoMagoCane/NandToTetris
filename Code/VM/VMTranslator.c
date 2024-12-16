#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

// c-2: @SP AM=M-1 D=M A=A-1
// add: c-2 M=M+D
// sub: c-2 M=M-D
// neg: @SP A=M-1 M=!M

// and: c-2 M=M&D
// or:  c-2 M=M|D
// notL @SP A=M-1 M=-M

// logical structure (eq, gt, lt), R13=jump number:
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


token_t pointer_segment_tokens[] = {
    "local", { "@LCL" },
    "argument", { "@ARG" },
    "this", { "@THIS" },
    "that", { "@THAT" }
};

int main(int argc, char* argv) {
}



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
