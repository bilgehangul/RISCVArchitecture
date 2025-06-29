#include "Parser.h"

// FIXME, implement this function.
// Here shows an example on how to translate "add x10, x10, x25"
void loadInstructions(Instruction_Memory *i_mem, const char *trace)
{
    printf("Loading trace file: %s\n", trace);

    FILE *fd = fopen(trace, "r");
    if (fd == NULL)
    {
        perror("Cannot open trace file. \n");
        exit(EXIT_FAILURE);
    }

    // Iterate all the assembly instructions
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    Addr PC = 0; // program counter points to the zeroth location initially.
    int IMEM_index = 0;
    while ((read = getline(&line, &len, fd)) != -1)
    {
        // Assign program counter
        i_mem->instructions[IMEM_index].addr = PC;

        // Extract operation
        char *raw_instr = strtok(line, " ");
        if (strcmp(raw_instr, "add") == 0 ||
            strcmp(raw_instr, "sub") == 0 ||
            strcmp(raw_instr, "sll") == 0 ||
            strcmp(raw_instr, "srl") == 0 ||
            strcmp(raw_instr, "xor") == 0 ||
            strcmp(raw_instr, "or")  == 0 ||
            strcmp(raw_instr, "and") == 0)
        {
            parseRType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
	    }
        else if (strcmp(raw_instr, "ld") == 0 ||
            strcmp(raw_instr, "addi") == 0)
        {
            parseIType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
	    }
        else if (strcmp(raw_instr, "slli") == 0) 
        {
            parseShiftType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
	    }
        else if (strcmp(raw_instr, "bne") == 0) 
        {
            parseBType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
	    }
        IMEM_index++;
        PC += 4;
    }

    fclose(fd);
}

void parseRType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;

    if (strcmp(opr, "add") == 0)
    {
        opcode = 51;
        funct3 = 0;
        funct7 = 0;
    }
    if (strcmp(opr, "sub") == 0)
    {
        opcode = 51;
        funct3 = 0;
        funct7 = 32;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    reg[strlen(reg)-1] = '\0';
    unsigned rs_2 = regIndex(reg);

    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= (funct7 << (7 + 5 + 3 + 5 + 5));
}

void parseIType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;
    unsigned rd;
    unsigned imm;
    unsigned rs_1;
    char *reg;
    if (strcmp(opr, "ld") == 0)
    {
        opcode = 3;
        funct3 = 3;
        reg = strtok(NULL, ", ");
        rd = regIndex(reg);

        reg = strtok(NULL, "(");
        imm = atoi(reg);

        reg = strtok(NULL, ")");
        rs_1 = regIndex(reg);
    }

    else if (!(strcmp(opr, "addi")))
    {
        opcode = 19;
        funct3 = 0;
        reg = strtok(NULL, ", ");
        rd = regIndex(reg);

        reg = strtok(NULL, ", ");
        rs_1 = regIndex(reg);

        reg = strtok(NULL, "\0");
        imm = atoi(reg);
    }
    
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (imm << (7 + 5 + 3 + 5));
}

void parseShiftType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;

    if (strcmp(opr, "slli") == 0)
    {
        opcode = 19;
        funct3 = 1;
        funct7 = 0;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, "\0");
    unsigned imm = atoi(reg);

    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (imm << (7 + 5 + 3 + 5));
    instr->instruction |= (funct7 << (7 + 5 + 3 + 5 + 6));
}

void parseBType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode;
    unsigned funct3;

    if (strcmp(opr, "bne") == 0)
    {
        opcode = 99;
        funct3 = 1;
    }

    char *reg;
    reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_2 = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned imm = atoi(reg);

    int immArr[12];
    unsigned rep = imm;
    for(int i = 0; i < 12; i++){
        immArr[i] = (rep%2);
        rep = rep/2;
    }

    instr->instruction |= opcode;
    instr->instruction |= (immArr[10] << 7);
    instr->instruction |= (immArr[3] << (7 + 4));
    instr->instruction |= (immArr[2] << (7 + 3));
    instr->instruction |= (immArr[1] << (7 + 2));
    instr->instruction |= (immArr[0] << (7 + 1));
    instr->instruction |= (funct3 << (7 + 1 + 4));
    instr->instruction |= (rs_1 << (7 + 1 + 4 + 3));
    instr->instruction |= (rs_2 << (7 + 1 + 4 + 3 + 5));
    instr->instruction |= (immArr[9] << (7 + 1 + 4 + 3 + 5 + 6 + 4));
    instr->instruction |= (immArr[8] << (7 + 1 + 4 + 3 + 5 + 5 + 4));
    instr->instruction |= (immArr[7] << (7 + 1 + 4 + 3 + 5 + 4 + 4));
    instr->instruction |= (immArr[6] << (7 + 1 + 4 + 3 + 5 + 3 + 4));
    instr->instruction |= (immArr[5] << (7 + 1 + 4 + 3 + 5 + 2 + 4));
    instr->instruction |= (immArr[4] << (7 + 1 + 4 + 3 + 5 + 1 + 4));
    instr->instruction |= (immArr[11] << (31));
}

int regIndex(char *reg)
{
    unsigned i = 0;
    for (i; i < NUM_OF_REGS; i++)
    {
        if (strcmp(REGISTER_NAME[i], reg) == 0)
        {
            break;
        }
    }

    return i;
}
