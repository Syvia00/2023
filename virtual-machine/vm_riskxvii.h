#ifndef VM_RISKXVII_H
#define VM_RISKXVII_H

#define INST_MEM_SIZE 1024
#define DATA_MEM_SIZE 1024

struct blob {
char inst_mem[INST_MEM_SIZE];
char data_mem[DATA_MEM_SIZE];
};

extern int InstBinaryToDecimal(int* arr, int start, int end);
extern int binaryToDecimal(int binary);
extern int intToBinary(int* arr, int start, int end);
extern int checkNegative(int binary);
extern int storeValue(char* memory, int bit, int address);
extern void printAllRd(int* registers, int pc);


#endif