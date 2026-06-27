// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

(loop)
    @KBD
    D=M
    @isBlack
    M=D

    @8192
    D=A
    @i
    M=D

    @isBlack
    D=M
    @drawBlack
    D;JNE
    @drawWhite
    D;JEQ

    (drawBlack)
        @i
        M=M-1
        D=M

        @SCREEN
        A=D+A
        M=-1

        @i
        D=M
        @drawBlack
        D;JNE

    @loop
    0;JMP

    (drawWhite)
        @i
        M=M-1
        D=M

        @SCREEN
        A=D+A
        M=0

        @i
        D=M
        @drawWhite
        D;JNE

    @loop
    0;JMP
