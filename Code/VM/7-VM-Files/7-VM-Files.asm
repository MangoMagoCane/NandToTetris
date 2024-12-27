// --PROGRAM START--
@261
D=A
@SP
M=D
@Sys.init
0;JMP
//   --SimpleAdd START--
// push constant 7
@7
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 8
@8
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
//   --StaticTest START--
// push constant 111
@111
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 333
@333
D=A
@SP
M=M+1
A=M-1
M=D
// push constant 888
@888
D=A
@SP
M=M+1
A=M-1
M=D
// pop static 8
@SP
AM=M-1
D=M
@StaticTest.8
M=D
// pop static 3
@SP
AM=M-1
D=M
@StaticTest.3
M=D
// pop static 1
@SP
AM=M-1
D=M
@StaticTest.1
M=D
// push static 3
@StaticTest.3
D=M
@SP
M=M+1
A=M-1
M=D
// push static 1
@StaticTest.1
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
// push static 8
@StaticTest.8
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
//   --StackTest START--
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
M=D+M
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
M=-M
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
M=!M
//   --BasicTest START--
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
M=D+M
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
M=M-D
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
M=D+M
// sub
@SP
AM=M-1
D=M
A=A-1
M=M-D
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
M=D+M
//   --PointerTest START--
// push constant 3030
@3030
D=A
@SP
M=M+1
A=M-1
M=D
// pop pointer 0
@SP
AM=M-1
D=M
@THIS
M=D
// push constant 3040
@3040
D=A
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
M=D+M
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
M=D+M
// -- PROGRAM END--
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
