#include "../headers/simulator.h"
#include <fstream>
#define _CRT_SECURE_NO_WARNINGS

// function code for R type instructions
const int add_func = 32;
const int slt_func = 42;
const int sltu_func = 43;
const int and_func = 36;
const int or_func = 37;
const int xor_func = 38;
const int nor_func = 39;
const int sub_func = 34;
const int mul_func = 2;

// op code 
const int r_op = 0;
const int sltiu_op = 11;
const int slti_op = 10;
const int addi_op = 8;
const int andi_op = 12;
const int ori_op = 13;
const int xori_op = 14;
const int sw_op = 43;
const int lw_op = 35;
const int bne_op = 5;
const int beq_op = 4;
const int j_op = 2;
const int jal_op = 3;
const int mul_op = 28;

simulator::simulator() {
	simulator(MAXMEMSIZE);
}

simulator::~simulator() {
	delete memory;
}

code simulator::getBreakpoint() const
{
    return breakpoint;
}

void simulator::setBreakpoint(const code &value)
{
    breakpoint = value;
}

int simulator::exportMemory(string fname)
{
    ofstream out(fname, ios::out|ios::binary);
    out.write((const char*)memory, sizeof(code)*memsize);
    out.close();
    return 1;
}

int simulator::loadMemory(string fname)
{
    ifstream in(fname, ios::in|ios::binary);
    in.read((char*)memory, sizeof(code)*memsize);
    in.close();
    for(int i=0;i<memsize;++i){
        setMemory(i*4,memory[i]);
    }
    return 1;
}

simulator::simulator(int size) {
    if (memsize >= MAXMEMSIZE) {
        memsize = MAXMEMSIZE;
    }
    else {
        memsize = size;
    }
    memory = new code[memsize +32];
	for (int i = 0; i < memsize; ++i) {
		memory[i] = 0;
	}
	for (int i = 0; i < 32; ++i) {
		regfile[i] = 0;
	}
	pc = 0;
	ir = 0;
    changedMemaddr = 0;
    breakpoint = ERRORCODE;
	initPCaddr = 0;
	memoryText.clear();
	memoryText.resize(memsize);
	loadbinary();
}

// return the steps execute
int simulator::run() {
	int count = 0;
    while (step()){
         ++count;
        if(pc == breakpoint)
            break;
        if(count == MAXEXECINSTS)
            break;
    }
	return count;
}

// if success. return 1, else return 0;
int simulator::step() {
    ir = memory[pc/4];
	code func = ir & 0x0000003f;
	code op = ir >> 26;
	code rs = (ir >> 21) & 0x0000001f;
	code rt = (ir >> 16) & 0x0000001f;
	code rd = (ir >> 11) & 0x0000001f;
	code imm = ir & 0x0000ffff;
	int immse = (int)((short)((unsigned short)imm));
	code immue = (code)((unsigned short)imm);
	code addr = ir & 0x03ffffff;
	pc = pc + 4;
	switch (op) {
	case r_op:
		switch (func) {
		case add_func:
			regfile[rd] = regfile[rs] + regfile[rt];
			break;
		case slt_func:
			if (*(int*)(&regfile[rs]) < *(int*)(&regfile[rt])) {
				regfile[rd] = 1;
			}
			else {
				regfile[rd] = 0;
			}
			break;
		case sltu_func:
			if (regfile[rs] < regfile[rt]) {
				regfile[rd] = 1;
			}
			else {
				regfile[rd] = 0;
			}
			break;
		case and_func:
			regfile[rd] = regfile[rs] & regfile[rt];
			break;
		case or_func:
			regfile[rd] = regfile[rs] | regfile[rt];
			break;
		case xor_func:
			regfile[rd] = regfile[rs] ^ regfile[rt];
			break;
		case nor_func:
			regfile[rd] = ~(regfile[rs] | regfile[rt]);
			break;
		case sub_func:
			regfile[rd] = regfile[rs] - regfile[rt];
			break;
		case mul_func:
			regfile[rd] = regfile[rs] * regfile[rt];
			break;
		}
		break;
	case sltiu_op:
		if (regfile[rs] < immue) {
			regfile[rt] = 1;
		}
		else {
			regfile[rt] = 0;
		}
		break;
	case slti_op:
		if (*(int*)(&regfile[rs]) < immse) {
			regfile[rt] = 1;
		}
		else {
			regfile[rt] = 0;
		}
		break;
	case addi_op:
		regfile[rt] = regfile[rs] + immse;
		break;
	case andi_op:
		regfile[rt] = regfile[rs] & immse;
		break;
	case ori_op:
		regfile[rt] = regfile[rs] | immse;
		break;
	case xori_op:
		regfile[rt] = regfile[rs] ^ immse;
		break;
	case sw_op:
        setMemory((immse + regfile[rs]),regfile[rt]);
		break;
	case lw_op:
        regfile[rt] = memory[(immse + regfile[rs])/4];
		break;
	case bne_op:
        if (regfile[rs] != regfile[rt]) {
			pc += immse << 2;
		}
		break;
	case beq_op:
        if (regfile[rs] == regfile[rt]) {
			pc += immse << 2;
		}
		break;
	case j_op:
        pc = (pc & 0xf0000000) | (addr << 2);
		break;
	case jal_op:
		regfile[31] = pc;
        pc = (pc & 0xf0000000) | (addr << 2);
		break;
	case mul_op:
		regfile[rd] = regfile[rs] * regfile[rt];
		break;
	default:
		return 0;
	}
	return 1;
}

void simulator::reset() {
	for (int i = 0; i < 32; ++i) {
		regfile[i] = 0;
	}
	ir = 0;
	pc = initPCaddr;
}

void simulator::setMemory(int address, code data, bool decomp) {
    address/=4;
	memory[address] = data;
    sprintf(memoryText[address].hextext, "0x%08x\t", memory[address]);
    char* convertascii = (char*)&memory[address];
    for(int i = 0;i<4;++i){
        if(isascii(convertascii[i])&& isprint(convertascii[i])){
            memoryText[address].asciitext[i] = convertascii[i];
        }else{
            memoryText[address].asciitext[i] = '.';
        }
    }
    if(decomp == false){
        memoryText[address].asciitext[4] = '\n';
        memoryText[address].asciitext[5] = 0;
    }else{
        memoryText[address].asciitext[4] = '\t';
        if(decompileCode(data,memoryText[address].asciitext + 5)){
            memoryText[address].asciitext[5] = 0;
        }
        int len = strlen(memoryText[address].asciitext);
        memoryText[address].asciitext[len] = '\n';
        memoryText[address].asciitext[len+1] = 0;
    }
    changedMemaddr = address;
}

code simulator::getPC() {
	return pc;
}

void simulator::setPC(code c) {
	pc = c;
}

code simulator::getIR() {
	return ir;
}

code simulator::getReg(int index) {
    return regfile[index];
}

code simulator::getchangedMemAddr()
{
    code n = changedMemaddr;
    changedMemaddr = 0;
    return n;
}

void simulator::loadbinary() {
	for (int i = 0; i < memsize; ++i) {
        setMemory(i*4, 0);
        sprintf(memoryText[i].address,"0x%08x\t",i*4);
	}
    if(compiledflag){
        initPCaddr = bin.text.origin;
        for (int i = 0; i < bin.text.length; ++i) {
            setMemory(i*4 + bin.text.origin, bin.text.codes[i],true);
        }
        for (int i = 0; i < bin.data.length; ++i) {
            setMemory(i*4 + bin.data.origin, bin.data.codes[i]);
        }
    }
    changedMemaddr = 0;
    reset();
}

int simulator::getSize()
{
    return memsize;
}

code simulator::getMem(int address) {
	return memory[address];
}
