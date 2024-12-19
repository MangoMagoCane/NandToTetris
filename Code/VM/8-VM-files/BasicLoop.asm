// --START--
// push constant 0
@0
D=A
@SP
M=M+1
A=M-1
M=D
// pop local 0
@SP
AM=M-1
D=M
@R13
M=D
@0
D=A
@LCL
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
// push local 0
@0
D=A
@LCL
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
// pop local 0
@SP
AM=M-1
D=M
@R13
M=D
@0
D=A
@LCL
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
// if-goto LOOP
@SP
AM=M-1
D=M
@__global__$LOOP
D;JNE
// push local 0
@0
D=A
@LCL
A=D+M
D=M
@SP
M=M+1
A=M-1
M=D
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
