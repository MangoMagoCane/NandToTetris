// push constant 10
@10
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
// push constant 21
@21
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 22
@22
D=A
@SP
M=M+1
A=M-1
M=D
// pop argument 2
@SP
AM=M-1
D=M
@R13
M=D
@2
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
// pop argument 1
@SP
AM=M-1
D=M
@R13
M=D
@1
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
// push constant 36
@36
D=A
@SP
M=M+1
A=M-1
M=D
// pop this 6
@SP
AM=M-1
D=M
@R13
M=D
@6
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
// push constant 42
@42
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 45
@45
D=A
@SP
M=M+1
A=M-1
M=D
// pop that 5
@SP
AM=M-1
D=M
@R13
M=D
@5
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
// push constant 510
@510
D=A
@SP
M=M+1
A=M-1
M=D
// pop temp 6
@SP
AM=M-1
D=M
@11
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
// push that 5
@5
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
M+MD
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
// sub
@SP
AM=M-1
D=M
A=A-1
M-MD
// push this 6
@6
D=A
@THIS
A=D+M
D=M
@SP
M=M+1
A=M-1
M=D
// push this 6
@6
D=A
@THIS
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
M+MD
// sub
@SP
AM=M-1
D=M
A=A-1
M-MD
// push temp 6
@11
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
