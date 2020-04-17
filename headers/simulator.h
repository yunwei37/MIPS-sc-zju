#pragma once

#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
extern "C"{
#include "compiler.h"
#include "ctype.h"
}

#define MAXMEMSIZE 65536

#define MAXEXECINSTS 128

using namespace std;

struct memshow {
    char address[16];
    char hextext[16];
    char asciitext[64];
};

class simulator {
private:
	int memsize;
	code *memory;
	code regfile[32];
	code pc;
	code ir;
	int initPCaddr;
	~simulator();
    code breakpoint;
    code changedMemaddr;

public:
	vector<memshow> memoryText;
	simulator();
	simulator(int size);
	// return the steps execute
	int run();
	// if success. return 1, else return 0;
	int step();
	void reset();
    void setMemory(int address, code data, bool decomp = false);

	code getPC();
	void setPC(code c);

	code getIR();
	code getReg(int index);
    code getchangedMemAddr();
	code getMem(int address);

	void loadbinary();

    int getSize();
    code getBreakpoint() const;
    void setBreakpoint(const code &value);

    int exportMemory(string fname);
    int loadMemory(string fname);
};


#endif
