<!-- ABOUT THE PROJECT -->
## About The Project

C++ implementation of the [Nand2Tetris](https://www.nand2tetris.org/) project.

Reference material: [The Elements of Computing Systems: Building a Modern Computer From First Principles](https://www.nand2tetris.org/book)

<!-- Nand2Tetris -->
## Nand2Tetris

Nand2Tetris is an epic 12-part project for building a 16-bit computer (Hack platform) and a software application (Jack application) targetting the Hack platform.

The project is split into two parts where parts 1-6 focus on Hardware and 7-12 focus on Software.

**Hardware:**

1. Boolean Logic
2. Boolean Arithmetic
3. Memory
4. Machine Language
5. Computer Architecture
6. Assembler

**Software:**

7. Virtual Machine I: Processing
8. Virtual Machine II: Control
9. High-Level Language
10. Compiler I: Syntax Analysis
11. Compiler II: Code Generation
12. Operating System


## Code Structure

For presentation purposes, I have grouped my code component-wise instead of project-part-wise.

The components are:
* Hardware
* Assembler
* VM Translator
* Jack Compiler
* OS
* Software application (Tetris!)

Because my version of Tetris ended up being too big, I have two versions of the Tetris and OS folder: Unoptimized and optimized.

The optimized version is roughly half the size to stay under the 32K instructions limit.

## How to run

I have implemented Tetris in the Jack language. I mean, it is kind of in the name.

You will need to grab the CPU Emulator or VM Emulator from the Nand2Tetris website ([here](https://www.nand2tetris.org/software))

Run either the CPU Emulator or VM Emulator and load the Prog.hack file. This file contains the binary code for Tetris.
Under "Animate:", choose "No animation", then press "Run".

Your experience may vary. On my machine, the game runs very slowly. Although, on other days, the speed is acceptable. Weird...

I think VM Emulator runs the game faster than CPU Emulator.

## Manual build

I have created a Makefile to compile the .Jack files into .vm files, which compiles down to assembly, then to binary.

You will need g++ to compile the c++ files. Other than that, I believe you can use any make program.

The make should produce a Prog.hack file at the root of the repository.
