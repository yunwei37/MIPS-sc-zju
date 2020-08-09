#include "../headers/compiler.h"
#include <string.h>
#include <stdio.h>

int decompileCode(code ir,char* buffer){
    // decode
    code func = ir & 0x0000003f;
    code op = ir >> 26;
    code rs = (ir >> 21) & 0x0000001f;
    code rt = (ir >> 16) & 0x0000001f;
    code rd = (ir >> 11) & 0x0000001f;
    code imm = ir & 0x0000ffff;
    int immse = (int)((short)((unsigned short)imm));
    //code immue = (code)((unsigned short)imm);
    code addr = ir & 0x03ffffff;


    char immbuffer[20];
    char instNameBuffer[20];
    if(op != 0 && op!= 28){
        func = 0;
    }
    char type = findInstName(op,func,instNameBuffer);
    if(type == 0){
        strcpy(errorMessage,"inst not found");
        return 1;
    }
    switch(type){
    case 'R':
        strcpy(buffer,instNameBuffer);
        strcat(buffer," ");
        strcat(buffer,regMapDict[rd]);
        strcat(buffer,",");
        strcat(buffer,regMapDict[rs]);
        strcat(buffer,",");
        strcat(buffer,regMapDict[rt]);
        break;
    case 'S':
        strcpy(buffer,instNameBuffer);
        break;
    case 'I':
        strcpy(buffer,instNameBuffer);
        strcat(buffer," ");
        strcat(buffer,regMapDict[rt]);
        strcat(buffer,",");
        strcat(buffer,regMapDict[rs]);
        strcat(buffer,",");
        sprintf(immbuffer," %d",immse);
        strcat(buffer,immbuffer);
        break;
    case 'J':
        strcpy(buffer,instNameBuffer);
        sprintf(immbuffer," %08x",addr);
        strcat(buffer,immbuffer);
        break;
    case 'M':
        strcpy(buffer,instNameBuffer);
        strcat(buffer," ");
        strcat(buffer,regMapDict[rt]);
        strcat(buffer,",");
        sprintf(immbuffer," %d(",immse);
        strcat(buffer,immbuffer);
        strcat(buffer,regMapDict[rs]);
        strcat(buffer,")");
        break;
    case 'B':
        strcpy(buffer,instNameBuffer);
        strcat(buffer," ");
        strcat(buffer,regMapDict[rs]);
        strcat(buffer,",");
        strcat(buffer,regMapDict[rt]);
        strcat(buffer,",");
        sprintf(immbuffer," %d",immse);
        strcat(buffer,immbuffer);
        break;
    default:
        strcpy(errorMessage,"inst not support");
        return 1;
    }

    return 0;
}
