// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

// consts
  @16
  D=A
  @rows
  M=D

  @32
  D=A
  @cols
  M=D

  @16384
  D=A
  @screen
  M=D

  @24576
  D=A
  @keyboard
  M=D

(INF_LOOP)
  @color
  M=0
  @keyboard
  A=M
  D=M
  @INIT
  D;JEQ
  @color
  M=-1
(INIT)
  @si
  M=0
(DRAW)

  @screen
  D=M
  @si
  D=D+M

  @sp
  M=D

  @color
  D=M

  @sp
  A=M
  M=D

  @si
  M=M+1

  @si
  D=M
  @8192
  D=D-A
  @DRAW  // loop back
  D;JLT
  @INF_LOOP
  0;JMP
