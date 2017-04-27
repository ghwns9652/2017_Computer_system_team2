#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <map>
#include "func.h"
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
using namespace std;
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
struct STAGE_REG
{
	string instr = "00000000000000000000000000000000";
	unsigned int NPC;
	int opcode;
	int REG1;
	int REG2;
	int REG3;
	int rd;
	int shift;
	int funct;
	int IMM;
	int ALU_OUT;
	int BR_TARGET;
	int MEM_OUT;
	int reg_data;
	//control
	int reg_wt;
	//int PC;
	int mem_wt;
	int mem_rd;
	int mem2reg;
	int sign_ex;
	int ALUSrc;
	int branch;
	int jump;
	int Regdst;
	int DATA1;
	int DATA2;
	int stall_sign;
	int ALUOp;
};

int ALU_controller(int ALUOp, int funct) {
	if (ALUOp == 0) {
		return 2;	//ADDIU, LW, SW
	}
	else if (ALUOp == 1) {
		return 6;	//BEQ, BNE, J, JAL
	}
	else if (ALUOp == 2) {
		if (funct == 0x21) {
			return 2;	//ADDU
		}
		else if (funct == 0x24) {
			return 0;	//AND
		}
		else if (funct == 8) {
			return 6;	//JR
		}
		else if (funct == 0x27) {
			return 12;	//NOR
		}
		else if (funct == 0x25) {
			return 1;	//OR
		}
		else if (funct == 0x2b) {
			return 7;	//SLTU
		}
		else if (funct == 0) {
			return 8;	//SLL
		}
		else if (funct == 2) {
			return 9;	//SRL
		}
		else if (funct == 0x23) {
			return 6;	//SUBU
		}
	}
}

STAGE_REG IF(void) //PC를 인자로 주면 PC값이 변경되지 않는 문제점이 있었다!
{
	struct STAGE_REG IF_ID;
	// 메모리에서 명렁어 읽어오기
	string instruction;
	//instruction = print_bin(read_mem(PC), 32); // take instruction from memory

	instruction = "00100100000000100000010000000000";

	PC = PC + 4;
	IF_ID.NPC = PC; //Save PC value to IF_ID_Register
	IF_ID.instr = instruction; //Save PC value to IF_ID_Register

	return IF_ID;
}

STAGE_REG ID(STAGE_REG IF_ID)
{
	STAGE_REG result;

	string ins = IF_ID.instr;
	int opcode = convert210(ins.substr(0, 6));
	int ins_type = type_checker(opcode);
	result.jump = 0;

	//for J code 
	if (ins_type == 1) {
		//need jump and flush
		result.jump = 1;
		result.reg_wt = 0;
		result.Regdst = 0;
		result.mem_rd = 0;
		result.mem_wt = 0;
		result.mem2reg = 0;
		result.branch = 0;
		result.ALUSrc = 0;
	}

	//always
	result.DATA1 = reg[convert210(ins.substr(6, 5))];
	result.DATA2 = reg[convert210(ins.substr(11, 5))];
	result.REG2 = convert210(ins.substr(11, 5)); //i didn't used REG1
	result.REG3 = convert210(ins.substr(16, 5)); // either REG2 or REG3 become REG destination at EX stage
	result.shift = convert210(ins.substr(21, 5));
	result.funct = convert210(ins.substr(26, 6));
	int funct = result.funct;
	result.IMM = sign_extend(convert210(ins.substr(16, 16))); //need sign extended
	
	if (ins_type == 0) { //I
		result.ALUSrc = 1;
		if (opcode == 0x4 || opcode == 0x5) { // Branch instruction
			result.reg_wt = 0;
			result.Regdst = 0;
			result.mem_rd = 0;
			result.mem_wt = 0;
			result.mem2reg = 0;
			result.branch = 1;
			result.ALUSrc = 0;
			result.ALUOp = 1;
		}
		else if (opcode == 0x23) { // Load word  -- read only rs and write on rt
			result.reg_wt = 1;
			result.Regdst = 1;
			result.mem_rd = 1;
			result.mem_wt = 0;
			result.mem2reg = 1;
			result.branch = 0;
			result.ALUSrc = 1;
			result.ALUOp = 0;
		}
		else if (opcode == 0x2B) { // Store word -- read both rs rt
			result.reg_wt = 0;
			result.Regdst = 0;
			result.mem_rd = 0;
			result.mem_wt = 1;
			result.mem2reg = 0;
			result.branch = 0;
			result.ALUSrc = 1;
			result.ALUOp = 0;
		}
		else { //else case (include LUI)
			if (opcode == 9) {
				result.ALUOp = 0;	//ADDIU
			}
			else if (opcode == 0xc) {
				result.ALUOp = 3;	//ANDI
			}
			else if (opcode == 0xf) {
				result.ALUOp = 4;	//LUI
			}
			else if (opcode == 0xd) {
				result.ALUOp = 5;	//ORI
			}
			else if (opcode == 0xb) {
				result.ALUOp = 6;	//SLTIU
			}
			result.reg_wt = 1;
			result.Regdst = 1;
			result.mem_rd = 0;
			result.mem_wt = 0;
			result.mem2reg = 0;
			result.branch = 0;
			result.ALUSrc = 1;
		}
	}

	if (ins_type == 2) { //R
		result.reg_wt = 1;
		result.Regdst = 0;
		result.mem_rd = 0;
		result.mem_wt = 0;
		result.mem2reg = 0;
		result.branch = 0;
		result.ALUSrc = 0;
		result.ALUOp = 2;	


		if (funct == 0x8) { //FEAR OF JR
			result.jump = 1;
		}
	}
	//end of control sign make

	//start of ID stage specific

	//For JUMP
	if (result.jump == 1) {
		if (opcode == 0x2) { //J code
			PC = convert210(ins.substr(6, 26)) << 2;
			result.ALUOp = 1;
		}
		else if (opcode == 0x3) { //JAL code
			reg[31] = IF_ID.NPC;
			PC = convert210(ins.substr(6, 26)) << 2; 
			result.ALUOp = 1;
		}
		else if (opcode == 0) {	//JR code
			PC = reg[31];
			result.ALUOp = 1;
		}
		result.stall_sign = 1; // flush 1 cycle sign
	}

	return result;
}

STAGE_REG EX(STAGE_REG ID_EX)
{
	////
	STAGE_REG result = ID_EX;
	///////// R type
	int ALU_ctrl = ALU_controller(ID_EX.ALUOp, ID_EX.funct);
	if (ID_EX.ALUSrc == 1) {
		ID_EX.DATA2 = ID_EX.IMM;
	}

	if (ALU_ctrl == 0) {
		result.ALU_OUT = ID_EX.DATA1 & ID_EX.DATA2;		//and
	}
	else if (ALU_ctrl == 1) {
		result.ALU_OUT = ID_EX.DATA1 | ID_EX.DATA2;	//or
	}
	else if (ALU_ctrl == 2) {
		result.ALU_OUT = ID_EX.DATA1 + ID_EX.DATA2;	//add
	}
	else if (ALU_ctrl == 6) {
		result.ALU_OUT = ID_EX.DATA1 - ID_EX.DATA2;	//sub 이렇게 빼는거 맞나 ????
	}
	else if (ALU_ctrl == 7) {
		result.ALU_OUT = int(ID_EX.DATA1 < ID_EX.DATA2);	//set-on-less-than
	}
	else if (ALU_ctrl == 8) {
		result.ALU_OUT = ID_EX.DATA2 << ID_EX.shift;	//sll
	}
	else if (ALU_ctrl == 9) {
		result.ALU_OUT = ID_EX.DATA2 >> ID_EX.shift;	//srl
	}
	else if (ALU_ctrl == 10) {
		result.ALU_OUT = ID_EX.IMM << 16;	//LUI
	}
	else if (ALU_ctrl == 12) {
		result.ALU_OUT = ~(ID_EX.DATA1 | ID_EX.DATA2);	//nor
	}

	if (ID_EX.Regdst == 0) {
		result.rd = ID_EX.REG3;
	}
	else if (ID_EX.Regdst == 1) {
		result.rd = ID_EX.REG2;
	}

	return result;

	/*if (ID_EX.ALUOp == 2) {
		if (ID_EX.funct == 0x21) //ADDU (2)
		{
			result.ALU_OUT = ID_EX.DATA1 + ID_EX.DATA2;
		}
		else if (ID_EX.funct == 0x24) //AND (3)
		{
			result.ALU_OUT = ID_EX.DATA1 & ID_EX.DATA2;
		}
		else if (ID_EX.funct == 0x27) //12 nor
		{
			result.ALU_OUT = (~(ID_EX.DATA1 | ID_EX.DATA2));
		}
		else if (ID_EX.funct == 0x25) //13 or
		{
			result.ALU_OUT = (ID_EX.DATA1 | ID_EX.DATA2);
		}
		else if (ID_EX.funct == 0x2b) //16 sltu
		{
			result.ALU_OUT = (ID_EX.DATA1 < ID_EX.DATA2 ? 1 : 0);
		}
		else if (ID_EX.funct == 0) //17 sll
		{
			result.ALU_OUT = ID_EX.DATA2 << ID_EX.shift;
		}
		else if (ID_EX.funct == 2) //18 srl
		{
			result.ALU_OUT = ID_EX.DATA2 >> ID_EX.shift;
		}
		else if (ID_EX.funct == 0x23) //20 subu
		{
			result.ALU_OUT = ID_EX.DATA1 - ID_EX.DATA2;
		}
	}
	
	//////// I type
	if (ID_EX.opcode == 9) //ADDIU (1)
	{
		result.ALU_OUT = ID_EX.DATA1 + ID_EX.IMM;
	}
	else if (ID_EX.opcode == 0xc) //ANDI (4)
	{
		result.ALU_OUT = ID_EX.DATA1 & ID_EX.IMM;
	}
	else if (ID_EX.opcode == 0xf) //LUI (10)
	{
		result.ALU_OUT = ID_EX.IMM << 16;
	}
	else if (ID_EX.opcode == 0xd) //14 ori
	{
		result.ALU_OUT = (ID_EX.DATA1 | ID_EX.IMM);
	}
	else if (ID_EX.opcode == 0xb) //15 sltiu
	{
		if (ID_EX.DATA1 < ID_EX.IMM) {
			result.ALU_OUT = 1;
		}
		else {
			result.ALU_OUT = 0;
		}
	}*/

	// result에 전달해서 리턴

}

STAGE_REG MEM(STAGE_REG EX_MEM)
{
	STAGE_REG result = EX_MEM;
	cout << result.reg_wt << endl;
	if (EX_MEM.mem_wt == 1) {
		mem[EX_MEM.ALU_OUT] = EX_MEM.DATA2;
	}

	if (EX_MEM.mem_rd == 1) {
		result.reg_data = mem[EX_MEM.ALU_OUT];
	}

	result.DATA2 = 0;

	return result;
}

void WB(STAGE_REG MEM_WB)
{
	if (MEM_WB.reg_wt == 1 && MEM_WB.mem2reg == 1) {
		reg[MEM_WB.rd] = MEM_WB.reg_data;
	}

	if (MEM_WB.reg_wt == 1 && MEM_WB.mem2reg == 0) {
		reg[MEM_WB.rd] = MEM_WB.ALU_OUT;
	}
}

int main()
{
	PC = 0x400000;

	struct STAGE_REG IF_ID;
	struct STAGE_REG ID_EX;
	struct STAGE_REG EX_MEM;
	struct STAGE_REG MEM_WB;

	int i = 0;
	while (i<=4)
	{
		WB(MEM_WB);
		MEM_WB = MEM(EX_MEM);
		EX_MEM = EX(ID_EX);
		ID_EX = ID(IF_ID);
		IF_ID = IF();
		i++;
	}

	print_reg(&PC, reg);
	return 0;
}