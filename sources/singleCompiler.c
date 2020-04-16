#include "../headers/compiler.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// the instruction properties load from file
typedef struct {
	char inst[8];
	char type;
    code op;
    code func;
} instProp;

// used to look up for func code and op code
static instProp instDict[MAXINSTCOUNT];

// record the real number of instructions available
static int instCount;

char* regMapDict[32] = {
	"$zero","$at", "$v0", "$v1", "$a0", "$a1", "$a2",
	"$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5",
	"$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4",
	"$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1",
	"$gp", "$sp", "$fp", "$ra"
};

// return the index of the instruction in the dict
// return -1 if not found
static int lookUpInst(char * inst) {
	for (int i = 0; i < instCount; ++i) {
		if (strcmp(instDict[i].inst,inst) == 0) {
			return i;
		}
	}
	strcpy(errorMessage, "instruction not support");
	return -1;
}

// return the type of the instruction
// the inst name is copy to the name buffer
char findInstName(code op, code func, char* name) {
    for (int i = 0; i < instCount; ++i) {
        if (instDict[i].op == op && instDict[i].func == func) {
            strcpy(name, instDict[i].inst);
            return instDict[i].type;
        }
    }
    return 0;
}

// check if the reg index is in the right range
static int checkReg(int num) {
	if (num < 0 || num > 31) {
		strcpy(errorMessage, "register out of range");
		return 1;
	}
	return 0;
}

// remove space in a string
 void removeSpace(char* s) {
	char* p = s;
	while (*s) {
		if (isspace(*s))	s++;
		else *(p++) = *(s++);
	}
	*p = 0;
}

code RtypeToBinary(char* inst, int rd, int rs, int rt) {
	int index = lookUpInst(inst);
	if (index == -1) {
		return ERRORCODE;
	}
	if (checkReg(rs) | checkReg(rt) | checkReg(rd)) {
		return ERRORCODE;
	}
	code r = 0;
	r |= instDict[index].func;
	r |= rd << 11;
	r |= rt << 16;
	r |= rs << 21;
	r |= instDict[index].op << 26;
	return r;
}

code ItypeToBinary(char* inst, int rt, int rs, int imm) {
	int index = lookUpInst(inst);
	if (index == -1) {
		return ERRORCODE;
	}
	if (checkReg(rs) | checkReg(rt)) {
		return ERRORCODE;
	}
	if (imm > 32767 || imm < -32768) {
		strcpy(errorMessage, "immediate value out of range");
		return ERRORCODE;
	}
	code i = 0;
	code immc = imm&0x0000ffff;
	i |= instDict[index].op << 26;
	i |= rt << 16;
	i |= rs << 21;
	i |= immc;
	return i;
}

code JtypeToBinary(char* inst, int addr) {
	int index = lookUpInst(inst);
	if (index == -1) {
		return ERRORCODE;
	}
	if (addr > 67108863 || addr < 0) {
		strcpy(errorMessage, "jump address value out of range");
		return ERRORCODE;
	}
	code j = 0;
	j |= instDict[index].op << 26;
	j |= addr;
	return j;
}

static int mapReg(char* para) {
	char* pe;
	char regName[10];
	char parabuffer[50];
	int rni = 0;
	char* parabak = para;
	while (*para) {
		rni = 0;
		while (*para && *para != '$') para++;
		if (*para == '$') {
			pe = para;
			while (*pe && *pe != ',') regName[rni++] = *pe++;
			regName[rni] = 0;
			if (isalpha(regName[1])) {
				int i;
				for (i = 0; i < 32; ++i) {
					if (strcmp(regName, regMapDict[i]) == 0) {
						break;
					}
				}
				if (i >= 32) {
					strcpy(errorMessage, "invalid register name");
					return 1;
				}
				*para = 0;
				strcpy(parabuffer, parabak);
				para = parabuffer + strlen(parabuffer);
				sprintf(para, "$%d", i);
				para += strlen(para);
				strcpy(para, pe);
				strcpy(parabak, parabuffer);
				para = parabak;
			}
			else {
				para = pe;
			}
		}
	}
	return 0;
}

static  int solvelabel(char* labelname, char type) {
	label* lp = searchLable(labelname);
	if (lp == NULL) {
		strcpy(errorMessage, "label not found");
		return 1;
	}
	if (type == 'I') {
		if (currentState == 0) {
			sprintf(labelname, "%d", (lp->address - (bin.text.length+1)*4 - currentbaseAddress)/4);
		}
		else {
			sprintf(labelname, "%d", (lp->address - (bin.data.length+1)*4 - currentbaseAddress)/4);
		}
	}
	else if (type == 'J') {
		sprintf(labelname, "%d", lp->address + currentbaseAddress);
	}
	return 0;
}

code instToBinary(char* instruction) {
	// remove space
	while (isspace(*instruction)) instruction++;

	char inst[50];
	if (sscanf(instruction, "%s", inst) != 1) {
		strcpy(errorMessage, "code syntax error");
		return ERRORCODE;
	}
	if (strlen(inst) > 50) {
		printf("terminated.\n");
		exit(0);
	}
	instruction += strlen(inst);
	int index = lookUpInst(inst);
	if (index == -1) {
		return ERRORCODE;
	}

	removeSpace(instruction);

	if (mapReg(instruction)) {
		return ERRORCODE;
	}

	code c = ERRORCODE;
	if (instDict[index].type == 'R') {
		int rs, rd, rt;
		if (sscanf(instruction, "$%d,$%d,$%d", &rd, &rs, &rt) != 3) {
			strcpy(errorMessage, "code syntax error");
			return ERRORCODE;
		}
		c = RtypeToBinary(inst, rd, rs, rt);
	}
	if (instDict[index].type == 'S') {
		c = RtypeToBinary(inst, 0, 0, 0);
	}
	if (instDict[index].type == 'M') {
		int rs, dat, rt;
		if (sscanf(instruction, "$%d,%d($%d)", &rt, &dat, &rs) != 3) {
			strcpy(errorMessage, "code syntax error");
			return ERRORCODE;
		}
		c = ItypeToBinary(inst, rt, rs,dat);
	}
	if (instDict[index].type == 'I' || instDict[index].type == 'B') {
		int rs, imm, rt;
		
		char* p = instruction + strlen(instruction) - 1;
		while (*p != ','  && p> instruction) p--;
		if (p == instruction) {
			strcpy(errorMessage, "code syntax error");
			return ERRORCODE;
		}
		p++;
		if (isalpha(*p)) {
			int r = solvelabel(p,'I');
			if (r) {
				return ERRORCODE;
			}
		}

		if (sscanf(instruction, "$%d,$%d,%d", &rt, &rs, &imm) != 3) {
			strcpy(errorMessage, "code syntax error");
			return ERRORCODE;
		}
		if (instDict[index].type == 'I') {
			c = ItypeToBinary(inst, rt, rs, imm);
		}
		else {
			c = ItypeToBinary(inst, rs, rt, imm);
		}

	}
	if (instDict[index].type == 'J') {
		int addr;
		if (isalpha(instruction[0])) {
			int r = solvelabel(instruction,'J');
			if (r) {
				return ERRORCODE;
			}
		}
		if (sscanf(instruction, "%d", &addr) != 1) {
			strcpy(errorMessage, "code syntax error");
			return ERRORCODE;
		}
		c = JtypeToBinary(inst, addr);
	}
	return c;
}

int loadCodeDict() {
	FILE* codeRecord = fopen("code.txt","r");
	if (codeRecord == NULL) {
		return -1;
	}
	instProp instruction;
	int i = 0;
    while (fscanf(codeRecord, "%s %c %u %u",
		&(instruction.inst), &(instruction.type), &(instruction.op), &(instruction.func)) == 4) {
		instDict[i++] = instruction;
	}
	fclose(codeRecord);
	instCount = i;
	return i;
}

char* getErrorMessage() {
	return errorMessage;
}
