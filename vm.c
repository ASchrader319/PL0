//Austin Schrader
//HW1
#include <stdio.h>
//#include <stdlib.h>


#define ARRAY_SIZE 500

// P-machine registers
int BP;  // Base pointer
int SP;  // Stack pointer
int PC;  // Program counter
int IR[3];  // Instruction register (OP, L, M)
int halt; //halt flag

int NUM_RECORDS = 0;
int RECORDS_TOTAL = 0;

// Process address space (PAS)
int PAS[ARRAY_SIZE];
int RECORDS[ARRAY_SIZE];

// Helper functions to access the stack frame
int base(int l, int base) {
    while (l > 0) {
        base = PAS[base];
        l--;
    }
    return base;
}
//print helper function
//prints the registers and the stack
void printHelper() {
    printf("%d\t%d\t%d\t%d\t%d\t", IR[1],IR[2],PC, BP, SP);
    int i = SP - RECORDS_TOTAL + 1;
    int recordIndex = 0;
    int count = 0;

    while (i <= SP) {
        printf("%d ", PAS[i]);
        count++;
        // //prints a separator after the last parameter
        if(RECORDS[recordIndex]!=0 && count==RECORDS[recordIndex] && i != SP) {
            printf(" | ");
            recordIndex++;
            count = 0;
        }
        
        i++;
    }

    printf("\n");
    
}





int main(int argc, char *argv[]) {
    // registers initialization
    BP = ARRAY_SIZE;
    SP = BP - 1;
    PC = 0;
    halt = 0;
    

    // PAS initialization
    for (int i = 0; i < ARRAY_SIZE; i++) {
        PAS[i] = 0;
        RECORDS[i] = 0;
    }

    // Open the input file
    //FILE *file = fopen(argv[1], "r");
    FILE *file = fopen("elf.txt", "r");
    if (file == NULL) {
        printf("Failed to open the input file.\n");
        return 1;
    }
    
    // Read the input file
    int programSize = 0;
    while (fscanf(file, "%d %d %d", &PAS[programSize], &PAS[programSize + 1], &PAS[programSize + 2]) == 3) {
        programSize += 3;
    }
    fclose(file);

    BP = programSize;
    SP = BP - 1;
    PC = 0;
    printf("\t\tPC\tBP\tSP\tstack\n");
    printf("Initial Values:\t%d\t%d\t%d\n", PC, BP, SP);


    // Execute the input program
    while (PC >= 0 && PC < programSize && !halt) {
        // Fetch cycle
        // op
        IR[0] = PAS[PC];
        // L
        IR[1] = PAS[PC + 1];
        // M
        IR[2] = PAS[PC + 2];
        PC += 3;
        // Execute cycle
        int opcode = IR[0];
        int level = IR[1];
        int operand = IR[2];
        printf("\t");
        switch (opcode) {
            case 1:  // LIT
                printf("LIT\t");
                SP++;
                PAS[SP] = operand;
                RECORDS[NUM_RECORDS]++;
                RECORDS_TOTAL++;
                printHelper();
                break;
            case 2:  // OPR
                switch (operand) {
                    case 0:  // RTN
                        printf("RTN\t");
                        SP = BP - 1;
                        BP = PAS[SP + 2];
                        PC = PAS[SP + 3];
                        RECORDS_TOTAL-=RECORDS[NUM_RECORDS];
                        RECORDS[NUM_RECORDS] = 0;
                        NUM_RECORDS--;
                        break;
                    case 1:  // ADD
                        printf("ADD\t");
                        PAS[SP - 1] = PAS[SP - 1] + PAS[SP];
                        SP--;
                        RECORDS[NUM_RECORDS]--;
                        RECORDS_TOTAL--;
                        break;
                    case 2:  // SUB
                        printf("SUB\t");
                        PAS[SP - 1] = PAS[SP - 1] - PAS[SP];
                        SP--;
                        RECORDS[NUM_RECORDS]--;
                        RECORDS_TOTAL--;
                        break;
                    case 3:  // MUL
                        printf("MUL\t");
                        PAS[SP - 1] = PAS[SP - 1] * PAS[SP];
                        SP--;
                        RECORDS[NUM_RECORDS]--;
                        RECORDS_TOTAL--;
                        break;
                    case 4:  // DIV
                        printf("DIV\t");
                        PAS[SP - 1] = PAS[SP - 1] / PAS[SP];
                        SP--;
                        RECORDS[NUM_RECORDS]--;
                        RECORDS_TOTAL--;
                        break;
                    case 5:  // EQL
                        printf("EQL\t");
                        PAS[SP - 1] = PAS[SP - 1] == PAS[SP];
                        SP--;
                        RECORDS[NUM_RECORDS]--;
                        RECORDS_TOTAL--;
                        break;
                    case 6:  // NEQ
                        printf("NEQ\t");
                        PAS[SP - 1] = PAS[SP - 1] != PAS[SP];
                        SP--;
                        RECORDS[NUM_RECORDS]--;
                        RECORDS_TOTAL--;
                        break;
                    case 7:  // LSS
                        printf( "LSS\t");
                        PAS[SP - 1] = PAS[SP - 1] < PAS[SP];
                        SP--;
                        RECORDS[NUM_RECORDS]--;
                        RECORDS_TOTAL--;
                        break;
                    case 8:  // LEQ
                        printf("LEQ\t");
                        PAS[SP - 1] = PAS[SP - 1] <= PAS[SP];
                        SP--;
                        RECORDS[NUM_RECORDS]--;
                        RECORDS_TOTAL--;
                        break;
                    case 9:  // GTR
                        printf("GTR\t");
                        PAS[SP - 1] = PAS[SP - 1] > PAS[SP];
                        SP--;
                        RECORDS[NUM_RECORDS]--;
                        RECORDS_TOTAL--;
                        break;
                    case 10: // GEQ
                        printf("GEQ\t");
                        PAS[SP - 1] = PAS[SP - 1] >= PAS[SP];
                        SP--;
                        RECORDS[NUM_RECORDS]--;
                        RECORDS_TOTAL--;
                        break;
                    default:
                        printf("Invalid op code\n");
                        break;
                }
                if(operand != 0){
                    printHelper();
                }
                else{
                    printHelper();
                }
                break;
            case 3:  // LOD
                printf("LOD\t");
                SP++;
                PAS[SP] = PAS[base(level, BP) + operand];
                RECORDS[NUM_RECORDS]++;
                RECORDS_TOTAL++;
                printHelper();
                break;
            case 4:  // STO
                printf("STO\t");
                PAS[base(level, BP) + operand] = PAS[SP];
                SP--;
                RECORDS[NUM_RECORDS]--;
                RECORDS_TOTAL--;
                printHelper();
                break;
            case 5:  // CAL
                printf("CAL\t");
                printHelper();
                PAS[SP + 1] = base(level, BP);  // Static link (SL)
                PAS[SP + 2] = BP;  // Dynamic link (DL)
                PAS[SP + 3] = PC;  // Return address (RA)
                BP = SP + 1;
                PC = operand;
                NUM_RECORDS++;
                break;
            case 6:  // INC
                printf("INC\t");
                SP += operand;
                RECORDS[NUM_RECORDS]+=operand;
                RECORDS_TOTAL+=operand;
                printHelper();
                break;
            case 7:  // JMP
                printf("JMP\t");
                PC = operand;
                printHelper();
                break;
            case 8:  // JPC
                printf("JPC\t");
                if (PAS[SP] == 0) {
                    PC = operand;
                }
                SP--;
                RECORDS[NUM_RECORDS]--;
                RECORDS_TOTAL--;
                printHelper();
                break;
            case 9:  // SYS
                if (operand == 1) {
                    printf("\nOutput result is: %d\n", PAS[SP]);
                    printf("\tSYS\t");
                    SP--;
                    RECORDS[NUM_RECORDS]--;
                    RECORDS_TOTAL--;
                } else if (operand == 2) {
                    SP++;
                    RECORDS[NUM_RECORDS]++;
                    RECORDS_TOTAL++;
                    printf("Please enter an integer: ");
                    scanf("%d", &PAS[SP]);
                    printf("\tSYS\t");
                } else if (operand == 3) {
                    printf("SYS\t");
                    halt = 1;  // End of program
                }
                //print the registers and the stack
                printHelper();
                break;
            default:
                printf("Invalid opcode\n");
                break;
            
        }
        
    
    }

    return 0;
}