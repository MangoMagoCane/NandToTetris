#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

typedef struct token_t {
    char* name;
    char** asmbly;
} token_t;

token_t stack_tokens[] = {
    "push", {"@SP", "A=M", "M=D", "@SP", "M=M+1"}
};
// $i
// D=A
// $SEG
// D=D+M
// USE R13 ---- 
// $SP
// M=M-1
// A=M+1


int main(int argc, char* argv) {

}