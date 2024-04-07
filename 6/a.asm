// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/6/max/Max.asm

// Computes R2 = max(R0, R1)  (R0,R1,R2 refer to RAM[0],RAM[1],RAM[2])

   // D = R0 - R1
   @1
   D=M
   @3
   D=D-M
   // If (D > 0) goto ITSR0
   @2
   D;JGT
   // Its R1
   @5
   D=M
   @2
   M=D
   @3
   0;JMP
(ITSR0)
   @1           
   D=M
   @2
   M=D
(END)
   @4
   0;JMP
