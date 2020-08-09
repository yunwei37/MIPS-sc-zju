#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <string.h>
#include<stdio.h>
#include<ctype.h>
#include "../headers/compiler.h"

#define MINSTACKSIZE 30
#define MAXLINES 4096

// used for resolving the labels
// use linear list if the number labels is small
typedef struct {
	int maxlength;
	int pointer;
	label* content;
} stack;

static stack labels;

struct binarys bin;

char errorMessage[80];

char compileOutput[2048] = {0};

int compiledflag = 0;

static int textStartlineno = -1;
static int dataStartlineno = -1;

// used for tracking the address
static int validtextCount = 0;
static int validdataCount = 0;

// record the lines need to be convert to MIPS code 
static int lineCount = 0;
static char* lines[MAXLINES];

// record the line in the input file
static int inputLineCount = 0;
static int lineMap[MAXLINES];

typedef struct {
	int address;
	int lineno;
} segmentOri;
segmentOri orivertex[100];
static int oriCount = 0;

// 0 is parsing text, 1 is parsing data
int currentState = 0;
int currentbaseAddress = 0;

static int initlablesStack() {
	labels.content = (label*)malloc(sizeof(label) * MINSTACKSIZE);
	if (labels.content == NULL) {
        strcat(compileOutput,"malloc fail!\n");
        //exit(1);
		return 1;
	}
	labels.pointer = 0;
	labels.maxlength = MINSTACKSIZE;
	return 0;
}

static void destrylabelStack() {
	free(labels.content);
}

static void addLabel(label a) {
	if (labels.pointer < labels.maxlength) {
		labels.content[labels.pointer].address = a.address;
		strcpy(labels.content[labels.pointer++].name, a.name);
	}
	else {
		label* previous = labels.content;
		label* newc = (label*)malloc(sizeof(label) * labels.maxlength * 2);
		if (newc == NULL) {
            strcat(compileOutput,"malloc fail!\n");
			exit(1);
		}
		for (int i = 0; i < labels.maxlength; ++i) {
			newc[i].address = previous[i].address;
			strcpy(newc[i].name, previous[i].name);
		}
		free(previous);
		labels.content = newc;
		labels.maxlength = labels.maxlength * 2;
		addLabel(a);
	}
}

label* searchLable(char* name) {
	for (int i = 0; i < labels.pointer; ++i) {
		if (strcmp(labels.content[i].name, name) == 0) {
			return &(labels.content[i]);
		}
	}
	return NULL;
}

// line is not empty
// solve the labels
static code convertcodeline(char* line){
	code c = instToBinary(line);
	return c;
}

static code convertdataline(char* line) {
	code c = 0;
	if (sscanf(line,"%x",&c) != 1) {
		strcpy(errorMessage, ".data syntax error");
		return ERRORCODE;
	}
	return c;
}


// change the start pos of each line
static char* firstParseline(char* line, int lineno){
	char *p;
	p = line;
	// remove annotations
	while((*p)&&((*p)!='#')) p++;
	if(*p=='#'){
		*p = 0;
	}

	p = line;
	if ((p = strstr(line, ".text"))) {
		p += 5;
		if (textStartlineno == -1) {
			textStartlineno = lineno;
			validtextCount = 0;
			currentState = 0;
		}
		else {
			strcpy(errorMessage, ".text label syntex error");
			return NULL;
		}
	}else if ((p = strstr(line, ".data"))) {
		p += 5;
		if (dataStartlineno == -1) {
			dataStartlineno = lineno;
			validdataCount = 0;
			currentState = 1;
		}
		else {
			strcpy(errorMessage, ".data label syntex error");
			return NULL;
		}
	}else if ((p = strstr(line, ".origin"))) {
		p += 7;
		segmentOri o;
		o.lineno = lineno;
		while (isspace(*p)) p++;
		int n = sscanf(p, "%x", &(o.address));
		currentbaseAddress = o.address;
		while (*p&&!isspace(*p)) p++;
		if (n != 1) {
			strcpy(errorMessage, ".origin label syntex error");
			return NULL;
		}
		else {
			orivertex[oriCount++] = o;
		}
	}

	if(p == NULL)
		p = line;
	while (isspace(*p)) p++;

	// find labels and remove them
	char* p1;
	char labelName[32] = "";
	while ((p1 = strchr(p, ':'))) {
		while ((isalpha(*(p1-1))|| isalnum(*(p1-1)))&& p1-1 >= p)	p1--;
		//if (p1!= p) p1++;
		char* temp = strchr(p1, ':');
		*temp = 0;
		int n = sscanf(p1, "%s", labelName);
        if (n == 0 || strlen(labelName) == 0) {
			strcpy(errorMessage, "label syntex error");
			return NULL;
		}
		p1 = temp+1;
		p = p1;

		label a;
		if (currentState == 0) {
			a.address = validtextCount * 4 + currentbaseAddress;
		}
		else {
			a.address = validdataCount * 4 + currentbaseAddress;
		}
		strcpy(a.name, labelName);
		addLabel(a);
	}

	while (isspace(*p)) p++;
	if (*p != 0) {
		if (currentState == 0) {
			validtextCount++;
		}
		else {
			char stylebuffer[30];
			sscanf(p, "%s", stylebuffer);
			if (strcmp(stylebuffer, ".word") == 0) {
				p += strlen(stylebuffer);
				removeSpace(p); // remove space
				while (*p != 0) {
					lineMap[lineCount] = inputLineCount;
					char* instbuffer = malloc(sizeof(char) * strlen(p) + 32);
					if (instbuffer == NULL) {
						strcat(compileOutput, "malloc fail!\n");
						exit(1);
					}
					strcpy(instbuffer, p);
					lines[lineCount++] = instbuffer;
					validdataCount++;
					while (*p&&*p!=',') p++; // move the digits in the line
					if (*p == ',') {
						*p = 0;
						p++;
					}
				}
 			}
			else if (strcmp(stylebuffer, ".asciiz") == 0) {
				p += strlen(stylebuffer);
				while (isspace(*p)) p++;
				if (*p != '"') {
					strcpy(errorMessage, "asciiz syntex error");
					return NULL;
				}
				p++;
				char cbuffer[4];
				code c;
				int ccount = 0;
				while (*p && *p != '"') {
					if (ccount == 4) {
						char* wordbuffer = malloc(sizeof(char) * 24);
						if (wordbuffer == NULL) {
							strcat(compileOutput, "malloc fail!\n");
							exit(1);
						}
						memcpy(&c, cbuffer, sizeof(char) * 4);
						sprintf(wordbuffer, "%x", c);
						lineMap[lineCount] = inputLineCount;
						lines[lineCount++] = wordbuffer;
						validdataCount++;
						ccount = 0;
					}
					else {
						cbuffer[ccount++] = *p++;
					}
				}
				if (*p == '"') {
					p++;
				}
				else {
					strcpy(errorMessage, "asciiz syntex error");
					return NULL;
				}
				if (ccount != 0) {
					c = 0;
					char* wordbuffer = malloc(sizeof(char) * 24);
					if (wordbuffer == NULL) {
						strcat(compileOutput, "malloc fail!\n");
						exit(1);
					}
					memcpy(&c, cbuffer, sizeof(char) * ccount);
					sprintf(wordbuffer, "%x", c);
					lineMap[lineCount] = inputLineCount;
					lines[lineCount++] = wordbuffer;
					validdataCount++;
				}
			}
		}
	}

	return p;
}

static int Passline(char* text){
	char* line;
	line = strtok(text, "\n");
	int state = 0 ;

	while (line) {
		inputLineCount++;
		while (isspace(*line)) line++;
		if (*line != 0) {
			line = firstParseline(line, lineCount);
			if (line == NULL) {
                char errorbuffer[256];
                printf(compileOutput,"ERROR: line number %d\n message %s\n", inputLineCount, errorMessage);
                strcat(compileOutput,errorbuffer);
				state = 1;
			}else if (*line != 0) {
				char* instbuffer = malloc(sizeof(char) * strlen(line) + 32);
				strcpy(instbuffer, line);
				if (instbuffer == NULL) {
					strcat(compileOutput, "malloc fail!\n");
					exit(1);
				}
				lines[lineCount] = instbuffer;
				lineMap[lineCount] = inputLineCount;
				lineCount++;
			}
		}
		line = strtok(NULL, "\n");
	}

	if (textStartlineno == -1) {
        strcat(compileOutput,"ERROR: text segment not found\n");
		return 1;
	}

	bin.text.origin = 0;
	bin.text.length = 0;
	currentbaseAddress = 0;
	currentState = 0;
	bin.text.codes = malloc(sizeof(code) * validtextCount + 10);
	if (bin.text.codes == NULL) {
        strcat(compileOutput,"malloc fail!\n");
		exit(1);
	}
	code c = 0;
	int j = 0;
	int segmentEndLineno = 0;
	if (textStartlineno < dataStartlineno) {
		segmentEndLineno = dataStartlineno;
	}
	else {
		segmentEndLineno = lineCount;
	}
	for (int i = textStartlineno; i < segmentEndLineno; ++i) {
		if (strlen(lines[i]) != 0) {
			if (j < oriCount && orivertex[j].lineno <= i) {
				if (orivertex[j].lineno == textStartlineno) {
					bin.text.origin = orivertex[j].address;
				}
				currentbaseAddress = orivertex[j].address;
				j++;
			}
			c = convertcodeline(lines[i]);
			free(lines[i]);
			if (c == ERRORCODE) {
                char errorbuffer[256];
                sprintf(errorbuffer,"ERROR: text segment line number %d\n message %s\n", lineMap[i], errorMessage);
                strcat(compileOutput,errorbuffer);
                state = 1;
			}
			bin.text.codes[bin.text.length++] = c;
		}
	}

	currentState = 1;
	if (dataStartlineno == -1) {
        strcat(compileOutput,"Warning: data segment not found\n");
		return state;
	}

	bin.data.origin = 1024;
	currentbaseAddress = 1024;
	bin.data.length = 0;
	bin.data.codes = malloc(sizeof(code) * validdataCount + 10);
	if (bin.data.codes == NULL) {
        strcat(compileOutput,"malloc fail!\n");
		exit(1);
	}
	c = 0;
	//j = 0;
	if (textStartlineno < dataStartlineno) {
		segmentEndLineno = lineCount;
	}
	else {
		segmentEndLineno = textStartlineno;
	}
	for (int i = dataStartlineno; i < segmentEndLineno; ++i) {
		if (j < oriCount && orivertex[j].lineno <= i) {
			if (orivertex[j].lineno == dataStartlineno) {
				bin.data.origin = orivertex[j].address;
			}
			currentbaseAddress = orivertex[j].address;
			j++;
		}
		if (strlen(lines[i]) != 0) {
			c = convertdataline(lines[i]);
			free(lines[i]);
			if (c == ERRORCODE) {
                char errorbuffer[256];
                sprintf(errorbuffer,"ERROR: data segment line number %d\n message %s\n", lineMap[i], errorMessage);
                strcat(compileOutput,errorbuffer);
                state = 1;
			}
			bin.data.codes[bin.data.length++] = c;
		}
	}
	return state;
}

int  compileText(char* text) {
    if(initlablesStack()){
        return 1;
    }
	if (loadCodeDict() == -1) {
        strcat(compileOutput,"ERROR: load instruction file fail\n");
		return 1;
	}

    compiledflag = 0;

    compileOutput[0] = 0;
    errorMessage[0] = 0;

	lineCount = 0;
	inputLineCount = 0;

	oriCount = 0;
	currentState = 0;
	currentbaseAddress = 0;

	textStartlineno = -1;
	dataStartlineno = -1;
	validtextCount = 0;
	validdataCount = 0;

	int r = Passline(text);
	if (r == 1) {
        strcat(compileOutput,"compile terminate.\n");
		return 1;
	}
	destrylabelStack();
    strcpy(compileOutput,"compile successfully.\n");
    compiledflag = 1;
	return 0;
	//return b;
}
