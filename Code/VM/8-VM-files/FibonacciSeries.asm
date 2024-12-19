// --START--
@256
D=A
@SP
M=D
// push argument 1
@1
D=A
@ARG
A=D+M
D=M
@SP
M=M+1
A=M-1
M=D
// pop pointer 1
@SP
AM=M-1
D=M
@THAT
M=D
// push constant 0
@0
D=A
@SP
M=M+1
A=M-1
M=D
// pop that 0
@SP
AM=M-1
D=M
@R13
M=D
@0
D=A
@THAT
D=D+M
@R14
M=D
@R13
D=M
@R14
A=M
M=D
// push constant 1
@1
D=A
@SP
M=M+1
A=M-1
M=D
// pop that 1
@SP
AM=M-1
D=M
@R13
M=D
@1
D=A
@THAT
D=D+M
@R14
M=D
@R13
D=M
@R14
A=M
M=D
// push argument 0
@0
D=A
@ARG
A=D+M
D=M
@SP
M=M+1
A=M-1
M=D
// push constant 2
@2
D=A
@SP
M=M+1
A=M-1
M=D
// sub
@SP
AM=M-1
D=M
A=A-1
M=M-D
// pop argument 0
@SP
AM=M-1
D=M
@R13
M=D
@0
D=A
@ARG
D=D+M
@R14
M=D
@R13
D=M
@R14
A=M
M=D
// label LOOP
(__global__$LOOP)
// push argument 0
@0
D=A
@ARG
A=D+M
D=M
@SP
M=M+1
A=M-1
M=D
// if-goto COMPUTE_ELEMENT
@SP
AM=M-1
D=M
@__global__$COMPUTE_ELEMENT
D;JNE
// goto END
@__global__$END
0;JMP
// label COMPUTE_ELEMENT
(__global__$COMPUTE_ELEMENT)
// push that 0
@0
D=A
@THAT
A=D+M
D=M
@SP
M=M+1
A=M-1
M=D
// push that 1
@1
D=A
@THAT
A=D+M
D=M
@SP
M=M+1
A=M-1
M=D
// add
@SP
AM=M-1
D=M
A=A-1
M=D+M
// pop that 2
@SP
AM=M-1
D=M
@R13
M=D
@2
D=A
@THAT
D=D+M
@R14
M=D
@R13
D=M
@R14
A=M
M=D
// push pointer 1
@THAT
D=M
@SP
M=M+1
A=M-1
M=D
// push constant 1
@1
D=A
@SP
M=M+1
A=M-1
M=D
// add
@SP
AM=M-1
D=M
A=A-1
M=D+M
// pop pointer 1
@SP
AM=M-1
D=M
@THAT
M=D
// push argument 0
@0
D=A
@ARG
A=D+M
D=M
@SP
M=M+1
A=M-1
M=D
// push constant 1
@1
D=A
@SP
M=M+1
A=M-1
M=D
// sub
@SP
AM=M-1
D=M
A=A-1
M=M-D
// pop argument 0
@SP
AM=M-1
D=M
@R13
M=D
@0
D=A
@ARG
D=D+M
@R14
M=D
@R13
D=M
@R14
A=M
M=D
// goto LOOP
@__global__$LOOP
0;JMP
// label END
(__global__$END)
// --END--
(end)
@end
0;JMP
(t_jmp)
@0
D=!A
A=M-1
M=D
@R13
A=M
0;JMP
(f_jmp)
@0
D=A
A=M-1
M=D
@R13
A=M
0;JMP
