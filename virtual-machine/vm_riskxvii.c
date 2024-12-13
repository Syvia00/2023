#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "vm_riskxvii.h"

// binary to decimal for int in instruction array
int InstBinaryToDecimal(int* arr, int start, int end){
    int decimal = 0;
    for (int i = start; i < end; i++){
        decimal += arr[i]*pow(2,i-start);
    }
    return decimal;
}

// binary to decimal for normal integer
int binaryToDecimal(int binary){
    int decimal = 0;
    int num = binary;
    int time = 0;
    while (num) {
        int lastNum = num % 10;
        num = (num-lastNum) / 10;
        decimal += lastNum * pow(2,time);
        time ++;
    }
    return decimal;
}

// single int in instruction array to binary number
int intToBinary(int* arr, int start, int end){
    int decimal = 0;
    for (int i = start; i < end; i++){
        decimal += (arr[i] << (i-start));
    }
    return decimal;
}

// check nagetive of binary number
int checkNegative(int binary){
    if ((binary & (1 << 17)) != 0){
        return binary | ~((1 << 18) - 1);
    }
    else{
        return binary;
    }
}

//store value from memory to registers
int storeValue(char* memory, int bit, int address){
    int newBit = 0;
    for(int i = 0; i < bit; i++){
        unsigned int digit = (unsigned int)memory[address+bit-1-i] - 0;
        for (int j = 0; j < 8; j ++){
            newBit += (digit % 2) << (j+i*8);
            digit /= 2;
        }
    }
    return newBit;
}

// print all registers and pc
void printAllRd(int* registers, int pc){
    printf("PC = 0x%08x;\n", pc);
    for (int r = 0; r <32; r++){
        printf("R[%d] = 0x%08x;\n", r, registers[r]);
    }
}

int main(int argc, char** argv){

    // check command line argument number
    if (argc != 2){
        printf("Error: no file\n");
        return 0;
    }

    else{
        // initial value
        struct blob new;
        int registers[32];
        int pc = 0;
        int heap[128][64];
        int size = 0;

        // read from mi file
        FILE *ptr;
        ptr = fopen(argv[1],"rb");
        if (!ptr){
            fprintf (stderr, "Error: cannot open file\n");
            return 0;
        }
        else{
            // get instruction from binary file
            fread(&new,sizeof(struct blob),1,ptr);

            // main routine of instruction
            while(1){
                int inst[32] = {0};
                int check = 0;

                // convert hex to binary 
                for (int i = 0; i < 4; i ++){
                    unsigned int digit = (unsigned int)new.inst_mem[pc+i] - 0;
                    for (int j = 0; j < 8; j ++){
                        inst[i*8+j] = digit % 2;
                        digit /= 2;
                    }
                }

                // find type for instruction
                int opcode = 0;
                int func3 = 0;
                int func7 = 0;
                int rd = 0;
                int rs1 = 0;
                int rs2 = 0;
                int imm = 0;
                int address = 0;

                opcode = InstBinaryToDecimal(inst,0,7);

                /////// Type: R ///////
                if (opcode == binaryToDecimal(110011)){
                    func3 = InstBinaryToDecimal(inst,12,15);
                    func7 = InstBinaryToDecimal(inst,25,32);
                    rd = InstBinaryToDecimal(inst,7,12);
                    rs1 = InstBinaryToDecimal(inst,15,20);
                    rs2 = InstBinaryToDecimal(inst,20,25);
                    
                    // if rd is 0, then always be zero
                    if (rd == 0){
                        registers[rd] = 0;
                    }
                    /// add ///
                    else if (func3 == 0 && func7 == 0){
                        registers[rd] = registers[rs1] + registers[rs2];
                    }
                    /// sub ///
                    else if (func3 == 0 && func7 == binaryToDecimal(100000)){
                        registers[rd] = registers[rs1] - registers[rs2];
                    }
                    /// xor ///
                    else if (func3 == binaryToDecimal(100) && func7 == 0){
                        registers[rd] = registers[rs1] ^ registers[rs2];
                    }
                    /// or ///
                    else if (func3 == binaryToDecimal(110) && func7 == 0){
                        registers[rd] = registers[rs1] | registers[rs2];
                    }
                    /// and ///
                    else if (func3 == binaryToDecimal(111) && func7 == 0){
                        registers[rd] = registers[rs1] & registers[rs2];
                    }
                    /// sll ///
                    else if (func3 == binaryToDecimal(1) && func7 == 0){
                        registers[rd] = (unsigned int)registers[rs1] << registers[rs2];
                    }
                    /// srl ///
                    else if (func3 == binaryToDecimal(101) && func7 == 0){
                        registers[rd] = (unsigned int)registers[rs1] >> registers[rs2];
                    }
                    /// sra ///
                    else if (func3 == binaryToDecimal(101) && func7 == binaryToDecimal(100000)){
                        // the right most bit is moved to the left most after shifting
                        registers[rd] = registers[rs1] >> registers[rs2];
                        int rmb = registers[rd] % 2;
                        registers[rd] = registers[rd] >> 1;
                        registers[rd] += rmb << 31;
                    }
                    /// slt ///
                    else if (func3 == binaryToDecimal(10) && func7 == 0){
                        if (registers[rs1] < registers[rs2]){
                            registers[rd] = 1;
                        }
                        else{
                            registers[rd] = 0;
                        }
                    }
                    /// sltu ///
                    else if (func3 == binaryToDecimal(11) && func7 == 0){
                        if (registers[rs1] < registers[rs2]){
                            registers[rd] = (unsigned int)1;
                        }
                        else{
                            registers[rd] = (unsigned int)0;
                        }
                    }
                }

                /////// Type: I ///////
                else if (opcode == binaryToDecimal(10011)){
                    func3 = InstBinaryToDecimal(inst,12,15);
                    rd = InstBinaryToDecimal(inst,7,12);
                    rs1 = InstBinaryToDecimal(inst,15,20);
                    imm = intToBinary(inst,20,32);
                    // fill with imm to 32 bit
                    for (int b = 0; b < 19; b++){
                        imm += inst[31] << (12+b);
                    }
                    imm = checkNegative(imm);

                    // if rd is 0, then always be zero
                    if(rd == 0){
                        registers[rd] = 0;
                    }
                    /// addi ///
                    else if (func3 == 0){
                        registers[rd] = registers[rs1] + imm;
                    }
                    /// xori ///
                    else if (func3 == binaryToDecimal(100)){
                        registers[rd] = registers[rs1] ^ imm;
                    }
                    /// ori ///
                    else if (func3 == binaryToDecimal(110)){
                        registers[rd] = registers[rs1] | imm;
                    }
                    /// andi ///
                    else if (func3 == binaryToDecimal(111)){
                        registers[rd] = registers[rs1] & imm;
                    }
                    /// slti ///
                    else if (func3 == binaryToDecimal(10)){
                        if (registers[rs1] < imm){
                            registers[rd] = 1;
                        }
                        else{
                            registers[rd] = 0;
                        }
                    }
                    /// sltiu ///
                    else if (func3 == binaryToDecimal(11)){
                        if (registers[rs1] < imm){
                            registers[rd] = (unsigned int)1;
                        }
                        else{
                            registers[rd] = (unsigned int)0;
                        }
                    }
                }

                /////// Type: I ///////
                else if (opcode == binaryToDecimal(11)){
                    func3 = InstBinaryToDecimal(inst,12,15);
                    rd = InstBinaryToDecimal(inst,7,12);
                    rs1 = InstBinaryToDecimal(inst,15,20);
                    imm = intToBinary(inst,20,32);
                    for (int b = 0; b < 19; b++){
                        imm += inst[31] << (12+b);
                    }
                    imm = checkNegative(imm);

                    int address = registers[rs1] + imm;
                    // if rd is 0, then always be zero
                    if (rd == 0){
                        registers[rd] = 0;
                    }
                    // check address
                    else if (address < 0x0800){
                        char* memory;
                        if (address < 0x0400){
                            memory = new.inst_mem;
                        }
                        else{
                            memory = new.data_mem;
                            address = address - 0x0400;
                        }
                        /// lb ///
                        if (func3 == 000){
                            //sign extend 8 to 32
                            registers[rd] = storeValue(memory, 1, address);
                        }
                        /// lh ///
                        else if (func3 == binaryToDecimal(1)){
                            //sign extend 16 to 32
                            registers[rd] = storeValue(memory, 2, address);
                        }
                        /// lw ///
                        else if (func3 == binaryToDecimal(10)){
                            registers[rd] = storeValue(memory, 4, address);
                        }
                        /// lbu ///
                        else if (func3 == binaryToDecimal(100)){
                            //sign extend 8 to 32
                            registers[rd] = (unsigned int)storeValue(memory, 1, address);
                        }
                        /// lhu ///
                        else if (func3 == binaryToDecimal(101)){
                            //sign extend 16 to 32
                            registers[rd] = (unsigned int)storeValue(memory, 2, address);
                        }
                    }
                    else{
                        ////// Virtual Routine //////
                        /// Read Character ///
                        if (address == 0x0812){
                            char c;
                            scanf("%c", &c);
                            registers[rd] = (int)c;
                        }
                        /// Read Sign Integer ///
                        else if (address == 0x0816){
                            int x;
                            scanf("%d", &x);
                            registers[rd] = x;
                        }
                    }
                }

                /////// Type: I ///////
                else if (opcode == binaryToDecimal(1100111)){
                    func3 = InstBinaryToDecimal(inst,12,15);
                    rd = InstBinaryToDecimal(inst,7,12);
                    rs1 = InstBinaryToDecimal(inst,15,20);
                    imm = intToBinary(inst,20,32);
                    for (int b = 0; b < 19; b++){
                        imm += inst[31] << (12+b);
                    }
                    imm = checkNegative(imm);

                    /// jalr ///
                    if (func3 == 0){
                        if (rd != 0){
                            registers[rd] = pc + 4;
                        }
                        pc = checkNegative(registers[rs1]) + imm - 4;
                    }
                }

                /////// Type: U ///////
                else if (opcode == binaryToDecimal(110111)){
                    rd = InstBinaryToDecimal(inst,7,12);
                    imm = intToBinary(inst,12,32) << 12;
                    for (int b = 0; b < 12; b ++){
                        imm += 0 << b;
                    }
                    imm = checkNegative(imm);

                    /// lui ///
                    if (rd != 0){
                        registers[rd] = imm;
                    }
                }

                /////// Type: S ///////
                else if (opcode == binaryToDecimal(100011)){
                    func3 = InstBinaryToDecimal(inst,12,15);
                    rs1 = InstBinaryToDecimal(inst,15,20);
                    rs2 = InstBinaryToDecimal(inst,20,25);
                    imm = intToBinary(inst,7,12);
                    imm += intToBinary(inst,25,32) << 5;
                    for (int b = 0; b < 19; b++){
                        imm += inst[31] << (12+b);
                    }
                    imm = checkNegative(imm);

                    // check address size
                    address = registers[rs1] + imm;
                    if (address < 0x0800){
                        char* memory;
                        if (address < 0x0400){
                            memory = new.inst_mem;
                        }
                        else{
                            memory = new.data_mem;
                            address = address - 0x0400;
                        }
                        /// sb ///
                        if (func3 == 0){
                            // 8 bit
                            memory[address] = registers[rs2] & 0xff;
                        }
                        /// sh ///
                        else if (func3 == binaryToDecimal(1)){
                            // 16 bit
                            for (int a = 0; a < 2; a++){
                                memory[address+1-a] = (registers[rs2] >> a*8) & 0xff;
                            }
                        }
                        /// sw ///
                        else if (func3 == binaryToDecimal(10)){
                            // 32 bit
                            for (int a = 0; a < 4; a++){
                                memory[address+3-a] = (registers[rs2] >> a*8) & 0xff;
                            }
                        }
                    }
                    else{
                        ////// Virtual Routines //////

                        /// Write Character ///
                        if (address == 0x0800){
                            printf("%c", registers[rs2]);
                        }
                        /// Write Sign Integer ///
                        else if (address == 0x0804){
                            printf("%d", (signed int)registers[rs2]);
                        }
                        /// Write Unsigned Integer ///
                        else if (address == 0x0808){
                            printf("%0x", (unsigned int)registers[rs2]);
                        }
                        /// Halt ///
                        else if (address == 0x080c){
                            printf("CPU Halt Requested\n");
                            return 0;
                        }
                        /// Dump PC ///
                        else if (address == 0x0820){
                            printf("%08x", pc);
                        }
                        /// Dump Register Banks ///
                        else if (address == 0x0824){
                            printAllRd(registers,pc);
                        }
                        /// Dump Memory Word ///
                        else if (address == 0x0828){
                            if (rs2 < 0x0400){
                                printf("%02x",new.inst_mem[rs2]);
                            }
                            else{
                                printf("%02x",new.data_mem[rs2-0x0400]);
                            }
                        }
                        /// Heap Banks ///
                        // Malloc //
                        else if (address == 0x0830){
                            // check memory size whether can be allocated
                            if (size + registers[rs2] <= 8192){
                                int count = 0;
                                // check free state by checking the 1/0 store in heap array, 1 for malloc, 0 for free
                                for (int a = 0; a < 128; a++){
                                    for (int b = 0; b < 64; b++){
                                        if (count == registers[rs2] + size){
                                            break;
                                        }
                                        if (count >= size && heap[a][b] == 0){
                                            heap[a][b] = 1;
                                        }
                                        if (count == size){
                                            registers[28] = a*64 + 0xb700;
                                        }
                                        count ++;
                                    }
                                }
                                size += registers[rs2];
                            }
                            else{
                                registers[28] = 0;
                            }
                        }
                        // Free //
                        else if (address == 0x0834){
                            // check address store in registers[28]
                            int add = 0;
                            if (registers[28] >= 0xb700){
                                add = registers[28] - 0xb700;
                            }
                            if (size - add >= registers[rs2]){
                                // check malloc state by checking the 1/0 store in heap array, 1 for malloc, 0 for free
                                int count = 0;
                                for (int a = add/64; a < 128; a++){
                                    for (int b = 0; b < 64; b++){
                                        if (count == registers[rs2]){
                                            break;
                                        }
                                        heap[a][b] = 0;
                                        count ++;
                                    }
                                }
                                size -= registers[rs2];
                            }
                            else{
                                check = 1;
                            }
                        }
                        /// Reserved ///
                        else if (address >= 0x0850){
                            continue;
                        }
                    }
                }

                /////// Type: SB ///////
                else if (opcode == binaryToDecimal(1100011)){
                    func3 = InstBinaryToDecimal(inst,12,15);
                    rs1 = InstBinaryToDecimal(inst,15,20);
                    rs2 = InstBinaryToDecimal(inst,20,25);
                    // manage to correct imm
                    imm = intToBinary(inst,8,12) << 1;
                    imm += intToBinary(inst,25,31) << 5;
                    imm += intToBinary(inst,7,8) << 11;
                    imm += intToBinary(inst,31,32) << 12;
                    for (int b = 0; b < 18; b++){
                        imm += inst[31] << (13+b);
                    }
                    imm = checkNegative(imm);

                    /// beq ///
                    if (func3 == 0){
                        if (registers[rs1] == registers[rs2]){
                            pc = pc + imm - 4;
                        }
                    }
                    /// bne ///
                    else if (func3 == binaryToDecimal(1)){
                        if (registers[rs1] != registers[rs2]){
                            pc = pc + imm - 4;
                        }
                    }
                    /// blt ///
                    else if (func3 == binaryToDecimal(100)){
                        if (registers[rs1] < registers[rs2]){
                            pc = pc + imm - 4;
                        }
                    }
                    /// bltu ///
                    else if (func3 == binaryToDecimal(110)){
                        if (registers[rs1] < registers[rs2]){
                            pc = pc + (unsigned int)imm - 4;
                        }
                    }
                    /// bge ///
                    else if (func3 == binaryToDecimal(101)){
                        if (registers[rs1] >= registers[rs2]){
                            pc = pc + imm - 4;
                        }
                    }
                    /// bgeu ///
                    else if (func3 == binaryToDecimal(111)){
                        if (registers[rs1] >= registers[rs2]){
                            pc = pc + (unsigned int)imm - 4;
                        }
                    }
                }

                /////// Type: UJ ///////
                else if (opcode == binaryToDecimal(1101111)){
                    rd = InstBinaryToDecimal(inst,7,12);
                    // manage imm
                    imm = intToBinary(inst,21,31) << 1;
                    imm += intToBinary(inst,20,21) << 11;
                    imm += intToBinary(inst,12,20) << 12;
                    imm += intToBinary(inst,31,32) << 20;
                    for (int b = 0; b < 10; b++){
                        imm += inst[31] << (21+b);
                    }
                    imm = checkNegative(imm);

                    /// jal ///
                    if (rd != 0){
                        registers[rd] = pc + 4;
                    }
                    pc = pc + (int)(imm) - 4;
                }

                // error in instruction
                else{
                    printf("Instruction Not Implemented: 0x%08x\n", intToBinary(inst,0,32));
                    printAllRd(registers,pc);
                    return 0;
                }
                
                // each loop increase pc value
                pc += 4;

                //  point out of range
                if (pc >= 0x0400){
                    check = 1;
                }

                // illegal operation
                if (check == 1){
                    printf("Illegal Operation: 0x%08x\n", intToBinary(inst,0,32));
                    printAllRd(registers,pc);
                    return 0;
                }  
            }
        }
        fclose (ptr);
    }
}