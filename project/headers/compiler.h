#pragma once
#ifndef _COMPILER_H_
#define _COMPILER_H_
#define _CRT_SECURE_NO_WARNINGS

#define MAXINSTCOUNT 100
#define ERRORCODE 0xffffffff

// the MIPS 32-bits code
typedef unsigned int code;

struct segment{
    int length;
    int origin;
    code* codes;
};

struct binarys {
    struct segment text;
    struct segment data;
};


typedef struct {
    char name[31];
    int address;
} label;

/*---------------------------------------------*/
/* convert one instruction to binary
 * eg: add $1,$2,$3
 *     addi $4,$0,128
 * all the registers is represented by "$index"
 * imm or dis is always digits
 * if code == ERRORCODE,
 * use getErrorMessage() to find the error report
 */
code instToBinary(char* instruction);

code RtypeToBinary(char* inst, int rd, int rs, int rt);
code ItypeToBinary(char* inst, int rt, int rs, int imm);
code JtypeToBinary(char* inst, int addr);

// get the error message
char* getErrorMessage();

// load the code file
int loadCodeDict();
/*---------------------------------------------*/
/* compile the assamble file text;
 * text with labels, ".text" and ".data" segments 
 * annotations start with '#'
 *
 */


int compileText(char* text);

/*-------------------------------------------------*/
/*
 * other function or varities needed
 */
// remove space in a string
void removeSpace(char* s);

extern char errorMessage[80];
extern int currentbaseAddress;
extern int currentState;

extern struct binarys bin;
extern char* regMapDict[32];

extern int compiledflag;

extern char compileOutput[2048];

//search the label in the label stack
label* searchLable(char* name);

char findInstName(code op, code func, char* name);
/*-------------------------------------------------*/
/*
 * decompiler:
 * covert code to instruction
 */
int decompileCode(code c,char* buffer);


#endif
