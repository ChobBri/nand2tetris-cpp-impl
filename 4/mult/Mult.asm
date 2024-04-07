// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
// The algorithm is based on repetitive addition.

// Init R2 to 0
  @R2
  M=0

  @R1
  D=M
  @i
  M=D
(LOOP)
  @i
  D=M
  @DONE
  D;JEQ

  @i
  M=M-1
  @R0
  D=M
  @R2
  D=D+M
  @R2
  M=D
  @LOOP
  0;JMP
(DONE)


