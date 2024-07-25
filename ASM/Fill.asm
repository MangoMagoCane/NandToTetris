// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

(KEYWAIT_OFF)
  @KBD
  D=M
  @SET_SCREEN_VARS
  D;JNE
  @KEYWAIT_OFF
  0;JMP

(SET_SCREEN_VARS)
    @FILL_COLOR
    M=-1
    @KEYWAIT_ON
    D=A
    @JMP_LOCATION
    M=D
    @FILL_SCREEN
    0;JMP

(RESET_SCREEN_VARS)
    @FILL_COLOR
    M=0
    @KEYWAIT_OFF
    D=A
    @JMP_LOCATION
    M=D
    @FILL_SCREEN
    0;JMP

(FILL_SCREEN)
  @SCREEN
  D=A
  @addr
  M=D
(FILL_LOOP) foo
  @FILL_COLOR
  D=M
  @addr
  A=M
  M=D
  @addr
  D=M
  MD=D+1
  @KBD
  D=D-M
  @FILL_LOOP
  D;JNE

  @JMP_LOCATION
  A=M
  0;JMP

(KEYWAIT_ON) // doggy 
  @KBD
  D=M
  @RESET_SCREEN_VARS
  D;JEQ
  @KEYWAIT_ON
  0;JMP

(END)
  @END
  0;JMP