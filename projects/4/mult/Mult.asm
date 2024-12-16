// The inputs of this program are the values stored in R0 and R1 (RAM[0] and RAM[1]). 
// The program computes the product R0 * R1 and stores the result in R2 (RAM[2]). 
// Assume that R0 ≥ 0, R1 ≥ 0, and R0 * R1 < 32768 (your program need not test these conditions). 
// Your code should not change the values of R0 and R1.

  // R2 = 0
  @R2
  M=0
  // counter = 0
  @counter
  M=0
  // if (counter >= R1) goto END 
(LOOP)
  @counter
  D=M
  @R0
  D=D-M
  @END
  D;JGE
  // else R2 += R1 counter++ goto LOOP
  @R1
  D=M
  @R2  
  M=D+M
  @counter  
  M=M+1
  @LOOP  
  0;JMP

(END)
  @END
  0;JMP
  