// push constant 3030
@3030
D=A
@SP
M=M+1
A=M-1
M=D
// pop pointer 0
@THIS
D=M
@SP
M=M+1
A=M-1
M=D
// push constant 3040
@3040
D=A
@SP
M=M+1
A=M-1
M=D
// pop pointer 1
@THAT
D=M
@SP
M=M+1
A=M-1
M=D
// push constant 32
@32
D=A
@SP
M=M+1
A=M-1
M=D
// pop this 2
@SP
AM=M-1
D=M
@R13
M=D
@2
D=A
@THIS
D=D+M
@R14
M=D
@R13
D=M
@R14
A=M
M=D
// push constant 46
@46
D=A
@SP
M=M+1
A=M-1
M=D
// pop that 6
@SP
AM=M-1
D=M
@R13
M=D
@6
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
// push pointer 0
@THIS
D=M
@SP
M=M+1
A=M-1
M=D
// push pointer 1
@THAT
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
M=M+D
// push this 2
@2
D=A
@THIS
A=D+M
D=M
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
// push that 6
@6
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
M=M+D
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
