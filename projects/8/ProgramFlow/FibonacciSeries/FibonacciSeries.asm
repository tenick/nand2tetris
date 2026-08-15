// push argument 1
@ARG
D=M
@1
D=D+A
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// pop pointer 1
@SP
M=M-1
A=M
D=M
@THAT
M=D
// push constant 0
@0
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop that 0
@THAT
D=M
@0
D=D+A
@POP_SEGMENT_I_VAR
M=D
@SP
M=M-1
A=M
D=M
@POP_SEGMENT_I_VAR
A=M
M=D
// push constant 1
@1
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop that 1
@THAT
D=M
@1
D=D+A
@POP_SEGMENT_I_VAR
M=D
@SP
M=M-1
A=M
D=M
@POP_SEGMENT_I_VAR
A=M
M=D
// push argument 0
@ARG
D=M
@0
D=D+A
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// push constant 2
@2
D=A
@SP
A=M
M=D
@SP
M=M+1
// sub
@SP
M=M-1
A=M
M=-M
@SP
M=M+1
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
M=D+M
@SP
M=M+1
// pop argument 0
@ARG
D=M
@0
D=D+A
@POP_SEGMENT_I_VAR
M=D
@SP
M=M-1
A=M
D=M
@POP_SEGMENT_I_VAR
A=M
M=D
// label LOOP
(LOOP)
// push argument 0
@ARG
D=M
@0
D=D+A
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// if-goto COMPUTE_ELEMENT
@SP
M=M-1
A=M
D=M
@COMPUTE_ELEMENT
D;JNE
// goto END
@END
0;JMP
// label COMPUTE_ELEMENT
(COMPUTE_ELEMENT)
// push that 0
@THAT
D=M
@0
D=D+A
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// push that 1
@THAT
D=M
@1
D=D+A
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// add
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
M=D+M
@SP
M=M+1
// pop that 2
@THAT
D=M
@2
D=D+A
@POP_SEGMENT_I_VAR
M=D
@SP
M=M-1
A=M
D=M
@POP_SEGMENT_I_VAR
A=M
M=D
// push pointer 1
@THAT
D=M
@SP
A=M
M=D
@SP
M=M+1
// push constant 1
@1
D=A
@SP
A=M
M=D
@SP
M=M+1
// add
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
M=D+M
@SP
M=M+1
// pop pointer 1
@SP
M=M-1
A=M
D=M
@THAT
M=D
// push argument 0
@ARG
D=M
@0
D=D+A
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// push constant 1
@1
D=A
@SP
A=M
M=D
@SP
M=M+1
// sub
@SP
M=M-1
A=M
M=-M
@SP
M=M+1
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
M=D+M
@SP
M=M+1
// pop argument 0
@ARG
D=M
@0
D=D+A
@POP_SEGMENT_I_VAR
M=D
@SP
M=M-1
A=M
D=M
@POP_SEGMENT_I_VAR
A=M
M=D
// goto LOOP
@LOOP
0;JMP
// label END
(END)
