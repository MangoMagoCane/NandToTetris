// push constant 17
@17
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 17
@17
D=A
@SP
M=M+1
A=M-1
M=D
// eq
@StackTest_0
D=A
@R13
M=D
@SP
AM=M-1
D=M
A=A-1
D=M-D
@t_jmp
D;JEQ
@f_jmp
0;JMP
(StackTest_0)
// push constant 17
@17
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 16
@16
D=A
@SP
M=M+1
A=M-1
M=D
// eq
@StackTest_1
D=A
@R13
M=D
@SP
AM=M-1
D=M
A=A-1
D=M-D
@t_jmp
D;JEQ
@f_jmp
0;JMP
(StackTest_1)
// push constant 16
@16
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 17
@17
D=A
@SP
M=M+1
A=M-1
M=D
// eq
@StackTest_2
D=A
@R13
M=D
@SP
AM=M-1
D=M
A=A-1
D=M-D
@t_jmp
D;JEQ
@f_jmp
0;JMP
(StackTest_2)
// push constant 892
@892
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 891
@891
D=A
@SP
M=M+1
A=M-1
M=D
// lt
@StackTest_3
D=A
@R13
M=D
@SP
AM=M-1
D=M
A=A-1
D=M-D
@t_jmp
D;JLT
@f_jmp
0;JMP
(StackTest_3)
// push constant 891
@891
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 892
@892
D=A
@SP
M=M+1
A=M-1
M=D
// lt
@StackTest_4
D=A
@R13
M=D
@SP
AM=M-1
D=M
A=A-1
D=M-D
@t_jmp
D;JLT
@f_jmp
0;JMP
(StackTest_4)
// push constant 891
@891
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 891
@891
D=A
@SP
M=M+1
A=M-1
M=D
// lt
@StackTest_5
D=A
@R13
M=D
@SP
AM=M-1
D=M
A=A-1
D=M-D
@t_jmp
D;JLT
@f_jmp
0;JMP
(StackTest_5)
// push constant 32767
@32767
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 32766
@32766
D=A
@SP
M=M+1
A=M-1
M=D
// gt
@StackTest_6
D=A
@R13
M=D
@SP
AM=M-1
D=M
A=A-1
D=M-D
@t_jmp
D;JGT
@f_jmp
0;JMP
(StackTest_6)
// push constant 32766
@32766
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 32767
@32767
D=A
@SP
M=M+1
A=M-1
M=D
// gt
@StackTest_7
D=A
@R13
M=D
@SP
AM=M-1
D=M
A=A-1
D=M-D
@t_jmp
D;JGT
@f_jmp
0;JMP
(StackTest_7)
// push constant 32766
@32766
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 32766
@32766
D=A
@SP
M=M+1
A=M-1
M=D
// gt
@StackTest_8
D=A
@R13
M=D
@SP
AM=M-1
D=M
A=A-1
D=M-D
@t_jmp
D;JGT
@f_jmp
0;JMP
(StackTest_8)
// push constant 57
@57
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 31
@31
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 53
@53
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
M=M+D
// push constant 112
@112
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
// neg
@SP
A=M-1
M=!M
// and
@SP
AM=M-1
D=M
A=A-1
M=M&D
// push constant 82
@82
D=A
@SP
M=M+1
A=M-1
M=D
// or
@SP
AM=M-1
D=M
A=A-1
M=M|D
// not
@SP
A=M-1
M=-M
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

