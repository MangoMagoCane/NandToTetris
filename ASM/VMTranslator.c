#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

typedef struct token_t {
    char* name;
    char* asmbly[10];
} token_t;

token_t stack_tokens[] = {
    "push", {"@SP", "A=M", "M=D", "@SP", "M=M+1"},
    "pop", {"@SP", "AM=M-1", "D=M", "@R13", "M=D"}
};
// $i
// D=A
// $SEG
// R13=D+M
// @SP
// AM=M-1
// D=M
// @R13
// M=D 


//GEN  $i, D=A, $SEG
//PUSH A=D+M, D=M
//POP  D=D+M @R13, M=D
token_t segment_tokens[] = {
    "local", {"@LCL"},
    "argument", {"@ARG"},
    "this", {"@THIS"},
    "that", {"@THAT"}
};
//pointer  PUSH: $SEG, D=A
//         POP:  $SEG, D=A, @R13, M=D
//pointer 0 this
//pointer 1 that

//constant PUSH: $i, D=A 
//  static PUSH: $Foo.i, D=A 
//         POP:  $Foo.i, D=A, @R13, M=D
//    temp PUSH: $i, D=A, @5, D=D+A // i < 8 
//         POP:  $i, D=A, @5, D=D+A, @R13, M=D

int main(int argc, char* argv) {

}