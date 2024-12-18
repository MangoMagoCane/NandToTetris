// pop pointer 0
@SP
AM=M-1
D=M
@THIS
M=D
// pop pointer 1
@SP
AM=M-1
D=M
@THAT
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
// add
@SP
AM=M-1
D=M
A=A-1
M+MD
// sub
@SP
AM=M-1
D=M
A=A-1
M-MD
// add
@SP
AM=M-1
D=M
A=A-1
M+MD
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
