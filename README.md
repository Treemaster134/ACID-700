**_ACID - 700_**

Advanced Computer Interpreting Dumbness

\



## Memory:

$h0 - $hFFF: ROM

$h1000 - $hFFFF: RAM


## Registers:

X: General purpose register (Used by some instructions as in and output)

Y: General purpose register (Unusable?)

Z: General purpose register (Used by some instructions as input)

FR: Flag register

| **Name**           | **Info**                                                                                                                     | **Bit &** |
| ------------------ | ---------------------------------------------------------------------------------------------------------------------------- | --------- |
| OVERFLOW           |                                                                                                                              | 0         |
| ZERO               |                                                                                                                              | 1         |
| NEGATIVE           |                                                                                                                              | 2         |
| VIDEO MODE         | 0 = 40x30 Character terminal1 = 100x80 16-bit color                                                                          | 3         |
| PAUSED             | 1: Cpu pauses execution until this is reset. Gets set to 0 when vblank occurs.                                               | 4         |
| TEXT + COLOR VIDEO | 0: See VIDEO MODE1: If video mode is also set to 1 this results in a 100x70 color display with 3 lines of text at the bottom | 5         |


## Input/Output

| **Port** | **Purpose**       |
| -------- | ----------------- |
| 0        | Keyboard in       |
| 1        | Tape 1 In/Out     |
| 2        | Tape 2 In/Out     |
| 3        | Unused            |
| 4        | Terminal output   |
| 5        | Peripheral port 1 |
| 6        | Peripheral port 2 |

Note: Peripheral ports can be used for mouse and joystick input, however only joystick input has been implemented.

Note: Port 3 was originally used for audio, but is no longer in use as a better audio system has been implemented.


## ACIDIC Language documentation

h = hex

i = integer

f = float

$ = address

& = address stored at another adress. For example: LODX \&hF5 would get the address stored at F5, then use that value as the address to another value which gets put in the X register.

%= immediate value

\# = Jump/branch/call label

\



## INSTRUCTION SET

|              |                                                           |                     |               |
| ------------ | --------------------------------------------------------- | ------------------- | ------------- |
| **Mnemonic** | **Description**                                           | **Arg 1**           | **Arg 2**     |
| NOP          | No Operation                                              |                     |               |
| SFR          | Set flag register to immediate value                      | %imm                |               |
| GFR          | Copy flag register to address                             | $dest               |               |
| CITF         | Convert integer value to a float                          | $src/dest\&src/dest |               |
| CFTI         | Convert floating point value to an integer (floor)        | $src/dest\&src/dest |               |
| RAND         | Generates a random number, result is placed in register X | %min$min\&min       | %max$max\&max |
| COMP         | Compare two values via subtraction                        | $min\&min           | %max$max\&max |
| LODX         | Load value into X register                                | %imm$src\&src       |               |
| LODY         | Load value into Y register                                | %imm$src\&src       |               |
| LODZ         | Load value into Z register                                | %imm$src\&src       |               |
| STOX         | Copy X register to address                                | $dest\&dest         |               |
| STOY         | Copy Y register to address                                | $dest\&dest         |               |
| STOZ         | Copy Z register to address                                | $dest\&dest         |               |
| LODI         | Load immediate value to address                           | %imm                |               |
| LODF         | Identical to LODI                                         | %imm                |               |
| MOVE         | Copy value from one address to another                    | $src\&src           | $dest\&dest   |
| SWAP         | Swap the values stored at two addresses                   | $add1\&add1         | $add2\&add2   |
| ADDI         | Integer addition                                          | $dest\&dest         | $src\&src     |
| ADDF         | Floating point addition                                   | $dest\&dest         | $src\&src     |

\
\
\
\
\


|      |                                                                                                                                                            |             |               |
| ---- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------- | ------------- |
| SUBI | Integer subtraction                                                                                                                                        | $dest\&dest | $src\&src     |
| SUBF | Floating point subtraction                                                                                                                                 | $dest\&dest | $src\&src     |
| MULI | Integer multiplication                                                                                                                                     | $dest\&dest | $src\&src     |
| MULF | Floating point multiplication                                                                                                                              | $dest\&dest | $src\&src     |
| DIVI | Integer division                                                                                                                                           | $dest\&dest | $src\&src     |
| DIVF | Floating point division                                                                                                                                    | $dest\&dest | $src\&src     |
| XOR  | Bitwise XOR                                                                                                                                                | $dest\&dest | %imm$src\&src |
| OR   | Bitwise OR                                                                                                                                                 | $dest\&dest | %imm$src\&src |
| AND  | Bitwise AND                                                                                                                                                | $dest\&dest | %imm$src\&src |
| NOT  | Bitwise NOT                                                                                                                                                | $dest\&dest | %imm$src\&src |
| ARSH | Arithmetic Right ShiftShift the data stored at arg1 by \<arg2> bits                                                                                        | $dest\&dest | %imm$src\&src |
| ALSH | Arithmetic Left ShiftShift the data stored at arg1 by \<arg2> bits                                                                                         | $dest\&dest | %imm$src\&src |
| LRSH | Logical Right ShiftShift the data stored at arg1 by \<arg2> bits                                                                                           | $dest\&dest | %imm$src\&src |
| LLSH | Logical Left ShiftShift the data stored at arg1 by \<arg2> bits                                                                                            | $dest\&dest | %imm$src\&src |
| RRSH | Rotate Right ShiftShift the data stored at arg1 by \<arg2> bits                                                                                            | $dest\&dest | %imm$src\&src |
| RLSH | Rotate Left ShiftShift the data stored at arg1 by \<arg2> bits                                                                                             | $dest\&dest | %imm$src\&src |
| PUSH | Push the data stored at src to the stack                                                                                                                   | $src\&src   |               |
| POP  | Pop data from the top of the stack to dest                                                                                                                 | $dest\&dest |               |
| BRNU | Uncoditional branch (relative jump)Adds the offset from dest to the PC. Label offsets are calculated by the assembler.                                     | %dest#LABEL |               |
| BRNP | Branch if the Zero and Negative flags are both set to 0 (relative jump)Adds the offset from dest to the PC. Label offsets are calculated by the assembler. | %dest#LABEL |               |

|       |                                                                                                                                                                                                                                                                  |             |               |
| ----- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------- | ------------- |
| BRNZ  | Branch if the Zero flag is set to 1 (relative jump)Adds the offset from dest to the PC. Label offsets are calculated by the assembler.                                                                                                                           | %dest#LABEL |               |
| BRNN  | Branch if the Negative flag is set to 1 (relative jump)Adds the offset from dest to the PC. Label offsets are calculated by the assembler.                                                                                                                       | %dest#LABEL |               |
| JMPU  | Uncoditional branch The PC is set to the value of dest.                                                                                                                                                                                                          | %dest#LABEL |               |
| JMPP  | Branch if the Zero and Negative flags are both set to 0 The PC is set to the value of dest.                                                                                                                                                                      | %dest#LABEL |               |
| JMPZ  | Branch if the Zero flag is set to 1 The PC is set to the value of dest.                                                                                                                                                                                          | %dest#LABEL |               |
| JMPN  | Jump if the Negative flag is set to 1 The PC is set to the value of dest.                                                                                                                                                                                        | %dest#LABEL |               |
| CALL  | The same as BRNU but before the branch the current PC value is pushed to the call stack.                                                                                                                                                                         | %dest       |               |
| OUT   | Output data using one of the built in I/O ports                                                                                                                                                                                                                  | %port       | %imm$src\&src |
| IN    | Input data using one of the built in I/O ports                                                                                                                                                                                                                   | %port       | $dest\&dest   |
| RET   | Return from a CALL or CALLJ                                                                                                                                                                                                                                      |             |               |
| WVB   | Pauses the system until the next VBLANK                                                                                                                                                                                                                          |             |               |
| CALLJ | The same as JMPU but before the branch the current PC value is pushed to the call stack.                                                                                                                                                                         |             |               |
| SOUND | Instructs the sound chip to start playing sound.Arg1: Start address of the sound dataArg2: The length of the dataZ Register: Bit 0 decides whether or not the sound will loop, bits 1 and 2 decide the channel (0, 1 or 2).X Register: The volume from 0 to 100. | %start      | %length       |
| COS   | Places the cos value of the radian angle arg2 into dest.                                                                                                                                                                                                         | $dest\&dest | %imm$src\&src |
| SIN   | Places the sin value of the radian angle arg2 into dest.                                                                                                                                                                                                         | $dest\&dest | %imm$src\&src |

\



## Asset files

You can include other files in your program as well, and these will simply be appended to the end of the assembled file. You include a file like this:

ASSET examplefile.bin

Which can then be accessed like this (for example):

MOVE $h1000 $a0

ADDI $h1000 %s0

This would move the first value from the file into the address h1000, and then add the size of the file to it this value.

The 0 in a0 and s0 means the first file, if you had more files then you would replace the 0 with the index of the file you wanted to access, like 1 for the second file, 2 for the third and so on.

When calculating the address of the files the assembler will assume that your program is stored in ram starting at address h3040, if you wish to change this (Like when writing your own boot rom) then you can download the assemblers source code and modify it. I might add the ability for the user to specify their own start address in the future.

\
\


I hope you enjoy using this program
