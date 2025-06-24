#include "Core.h"

Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;

    // FIXME, initialize register file here.
    for(int i = 0; i < 32; i++){
        core->data_mem[0] = 0;
    }
    core->data_mem[0] = 16;
    core->data_mem[8] = 128;
    core->data_mem[16] = 8;
    core->data_mem[24] = 4;

    // FIXME, initialize data memory here.
    core->reg_file[8] = 16;
    core->reg_file[10] = 4;
    core->reg_file[11] = 0;
    core->reg_file[22] = 1;
    core->reg_file[24] = 0;
    core->reg_file[25] = 4;

    return core;
}

// FIXME, implement this function
bool tickFunc(Core *core)
{
    // Steps may include
    // (Step 1) Reading instruction from instruction memory
    unsigned instruction = core->instr_mem->instructions[core->PC / 4].instruction;
    unsigned mask = 1;
    unsigned mask1 = 1;
    Signal opcode = 0;
    Signal rd = 0;
    Signal funct3 = 0;
    Signal funct6 = 0;
    Signal funct7 = 0;
    Signal rs_1 = 0;
    Signal rs_2 = 0;
    Signal imme12 = 0;
    Signal imme6 = 0;
    Signal imme12b = 0;
    int temp;
    mask1 <<= 7;
    if(mask1 & instruction){
        temp = mask1;
        imme12b = (temp << 3);
    }
    temp = 0;
    mask1 <<= 1;
    for(int i = 0; i < 4; i++){
        if(mask1 & instruction){
            temp += mask1;
        }
        mask1 <<= 1;
    }
    temp >>= 8;
    imme12b += temp;
    mask1 <<= 13;
    temp = 0;
    for(int i = 0; i < 6; i++){
        if(mask1 & instruction){
            temp += mask1;
        }
        mask1 <<= 1;
    }
    temp >>= 21;
    imme12b += temp;

    if(instruction & mask1){
        imme12b = ImmeGen(imme12b);
    }
    for(int i = 0; i < 7; i++){
        if(instruction & mask)
            opcode += mask;
        mask <<= 1; 
    }
    for(int i = 0; i < 5; i++){
        if(instruction & mask)
            rd += mask;
        mask <<= 1; 
    }
    rd >>= 7;
    for(int i = 0; i < 3; i++){
        if(instruction & mask)
            funct3 += mask;
        mask <<= 1; 
    }
    funct3 >>= 12;
    for(int i = 0; i < 5; i++){
        if(instruction & mask)
            rs_1 += mask;
        mask <<= 1; 
    }
    rs_1 >>= 15;
    for(int i = 0; i < 5; i++){
        if(instruction & mask)
            rs_2 += mask;
        mask <<= 1; 
    }
    rs_2 >>= 20;
    for(int i = 0; i < 7; i++){
        if(instruction & mask)
            funct7 += mask;
        mask <<= 1; 
    }
    mask >>= 7;
    funct7 >>= 25;
    imme6 = rs_2;
    if(instruction & mask)
            imme6 += mask;
    mask <<= 1;
    imme12 = imme6;
    for(int i = 0; i < 6; i++){
        if(instruction & mask)
            funct6 += mask;
            imme12 += mask;
        mask <<= 1; 
    }
    imme12 >>= 26;
    funct6 >>= 26;

    ControlSignals controlSigs;
    ControlUnit(opcode,&controlSigs);

    Signal reg1 = core->reg_file[rs_1];
    Signal reg2 = core->reg_file[rs_2];
    Signal imme;
    Signal aluInput;
    Signal aluCntr;
    Signal aluResult;
    Signal zero;
    Signal selectPC;
    Signal newPC;
    if(opcode == 99)
        imme = imme12b;
    else if(opcode == 19)
        imme = imme6;
    else
        imme = imme12;
    aluInput = MUX(controlSigs.ALUSrc,reg2,imme);
    aluCntr = ALUControlUnit(controlSigs.ALUOp,funct7,funct3);
    ALU(reg1,aluInput,aluCntr,&aluResult,&zero);
    selectPC = ((!zero) & controlSigs.Branch);
    //imme = ShiftLeft1(imme);
    newPC = Add(imme,core->PC);
    core->PC = MUX(selectPC,(core->PC += 4),newPC);
    if(!selectPC){
        core->reg_file[rd] = MUX(controlSigs.MemtoReg,aluResult,core->data_mem[aluResult]);
    }
        ++core->clk;
    // Are we reaching the final instruction?
    if (core->PC > core->instr_mem->last->addr)
    {
        return false;
    }
    return true;
}

// FIXME (1). Control Unit. Refer to Figure 4.18.
void ControlUnit(Signal input,
                 ControlSignals *signals)
{
    // For R-type
    if (input == 51)
    {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2;
    }
    else if (input == 19)
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2;
    }
    else if (input == 3)
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 1;
        signals->RegWrite = 1;
        signals->MemRead = 1;
        signals->MemWrite = 1;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    //for sd
    else if (input == 35)
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 0;
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 1;
        signals->ALUOp = 0;
    }
    //for beq
    else if (input == 99)
    {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 1;
        signals->ALUOp = 1;
    }
}

// FIXME (2). ALU Control Unit. Refer to Figure 4.12.
Signal ALUControlUnit(Signal ALUOp,
                      Signal Funct7,
                      Signal Funct3)
{
    // For add
    if (ALUOp == 2 && Funct7 == 0 && Funct3 == 0)
    {
        return 2;
    }
    else if (ALUOp == 2 && Funct7 == 32 && Funct3 == 0)
    {
        return 6;
    }
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 1)
    {
        return 3;
    }
    // For and
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 7)
    {
        return 0;
    }
    // For or
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 6)
    {
        return 1;
    }
    else if (ALUOp == 2 && Funct3 == 0)
    {
        return 2;
    }
    // For ld and sd
    else if (ALUOp == 0)
    {
        return 2;
    }  
    // For beq
    else if (ALUOp == 1)
    {
        return 6;
    }

}

// FIXME (3). Imme. Generator
Signal ImmeGen(Signal input)
{
    int temp = 1;
    temp <<= 11;
    for(int i = 0; i < 43; i++){
        input += temp;
        temp <<= 1;
    }
    return(input);
}

// FIXME (4). ALU
void ALU(Signal input_0,
         Signal input_1,
         Signal ALU_ctrl_signal,
         Signal *ALU_result,
         Signal *zero)
{
    // For addition
    if (ALU_ctrl_signal == 2)
    {
        *ALU_result = (input_0 + input_1);
    }
    else if (ALU_ctrl_signal == 3)
    {
        *ALU_result = (input_0 << input_1);
    }
    // For beq and sub
    else if (ALU_ctrl_signal == 6)
    {
        *ALU_result = (input_0 - input_1);
    }
    // For and
    else if (ALU_ctrl_signal == 0)
    {
        *ALU_result = (input_0 && input_1);
    }
    // For or
    else if (ALU_ctrl_signal == 1)
    {
        *ALU_result = (input_0 | input_1);
    }

    if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    
}

// (4). MUX
Signal MUX(Signal sel,
           Signal input_0,
           Signal input_1)
{
    if (sel == 0) { return input_0; } else { return input_1; }
}

// (5). Add
Signal Add(Signal input_0,
           Signal input_1)
{
    return (input_0 + input_1);
}

// (6). ShiftLeft1
Signal ShiftLeft1(Signal input)
{
    return input << 1;
}
