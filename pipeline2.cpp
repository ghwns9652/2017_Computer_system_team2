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
	string instr="00000000000000000000000000000000";
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
};

STAGE_REG IF(void) //PC를 인자로 주면 PC값이 변경되지 않는 문제점이 있었다!
{
	struct STAGE_REG IF_ID;
	// 메모리에서 명렁어 읽어오기
	string instruction;
	instruction = print_bin( read_mem(mem[PC]), 32); // take instruction from memory

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
	cout << ins << endl;
	result.DATA1 = reg[convert210(ins.substr(6, 5))];
	result.DATA2 = reg[convert210(ins.substr(11, 5))];
	result.REG2 = convert210(ins.substr(11, 5)); //i didn't used REG1
	result.REG3 = convert210(ins.substr(16, 5)); // either REG2 or REG3 become REG destination at EX stage
	result.jump = 0;

	if (ins_type == 0) { //I
		result.IMM = sign_extend(convert210(ins.substr(16, 16))); //need sign extended
		result.ALUSrc = 1;
		if (opcode == 0x4 || opcode == 0x5) { // Branch instruction
			result.reg_wt = 0;
			result.Regdst = 0;
			result.mem_rd = 0;
			result.mem_wt = 0;
			result.mem2reg = 0;
			result.branch = 1;
			result.ALUSrc = 0;
		}
		else if (opcode == 0x23) { // Load word  -- read only rs and write on rt
			result.reg_wt = 1;
			result.Regdst = 1;
			result.mem_rd = 1;
			result.mem_wt = 0;
			result.mem2reg = 1;
			result.branch = 0;
			result.ALUSrc = 1;
		}
		else if (opcode == 0x2B) { // Store word -- read both rs rt
			result.reg_wt = 0;
			result.Regdst = 0;
			result.mem_rd = 0;
			result.mem_wt = 1;
			result.mem2reg = 0;
			result.branch = 0;
			result.ALUSrc = 1;
		}
		else { //else case (include LUI)
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

		result.shift = convert210(ins.substr(21, 5)); // we really need this?
		result.funct = convert210(ins.substr(26, 6)); // we really need this?
		int funct = result.funct;

		if (funct == 0x8) { //FEAR OF JR
			//how to do?
			result.jump = 1; //?
		}
	}
	//end of control sign make

	//start of ID stage specific

	//For JUMP
	if (result.jump == 1) {
		if (opcode == 0x2) { //J code
			PC = convert210(ins.substr(6, 26)) << 2;
		}
		else if (opcode == 0x3) { //JAL code
			reg[31] = IF_ID.NPC;
			PC = convert210(ins.substr(6, 26)) << 2;
		}
		else if (opcode == 0x0) { //JR
			PC = reg[convert210(ins.substr(6, 5))]; //need check again;
		}
		result.stall_sign = 1; // flush 1 cycle sign
		return result;
	}
	//For BRANCH
	if (result.branch == 1) {
		//WELCOME TO THE HELL BRANCH WORLD!
	}

	return result;
}

STAGE_REG EX(STAGE_REG ID_EX)
{
	////
	STAGE_REG result;
	///////// R type
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
	}

	// result에 전달해서 리턴

	return result;
}

STAGE_REG MEM(STAGE_REG EX_MEM)
{

	if (EX_MEM.mem_wt == 1) {
		mem[EX_MEM.ALU_OUT] = EX_MEM.DATA2;
		EX_MEM.DATA2 = 0;
	}

	if (EX_MEM.mem_rd == 1) {
		EX_MEM.reg_data = mem[EX_MEM.ALU_OUT];
		EX_MEM.DATA2 = 0;
	}
	return EX_MEM;
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

int run_bin(int num_instruc, int d_exist, unsigned int* memory_range) {
	//initialize
	PC = 0x400000;
	for (int i = 0; i < 32; i++)
	{
		reg[i] = 0;
	}
	STAGE_REG IF_ID;
	STAGE_REG ID_EX;
	STAGE_REG EX_MEM;
	STAGE_REG MEM_WB;

	//save instruction in memory
	int text_size_ptr = 0;
	int data_size_ptr = 0;
	int loop_count = 0;

	save_ins(&text_size_ptr, &data_size_ptr, mem, num_instruc);
	text_size = text_size_ptr;
	cout << text_size_ptr << endl;
	//read instruction from memory
	//for (int line = 0; line < text_size_ptr; line ++)
	while (0x400000 <= PC && PC < (0x400000 + text_size_ptr))
	{
		if ((loop_count >= num_instruc) & (num_instruc != -1))
			break;
		string str_line = "";
		for (int i = 0; i < 4; i++)
		{
			str_line.append(print_bin(mem[PC + i], 8));
		}
		// PC 이전값 저장
		/* main의 끝을 위한 부분이었다
		if (PC != PC_temp) {
			END_warn = 0;
		}
		PC_temp = PC;

		if (END_warn == 1 && PC == EOM) {
			break;
		}
		else if (END_warn == 1 && PC == (0x400000 + text_size_ptr - 4)) {
			break;
		}

		if (PC == EOM || PC == (0x400000 + text_size_ptr - 4)) {
			END_warn = 1;
		}
		else {
			//END_warn = 0;
			PC += 4;
		}*/

		WB(MEM_WB);
		MEM_WB = MEM(EX_MEM);
		EX_MEM = EX(ID_EX);
		ID_EX = ID(IF_ID);
		IF_ID = IF();
		// something PC error between here
		if (d_exist) {
			print_reg(&PC, reg);
			if (memory_range[2] != 0) {
				print_mem(mem, memory_range[0], memory_range[1]);  //print_mem(reinterpret_cast<unsigned char*>(mem), start, end);
			}
		}
		loop_count += 1;
	}
	return 0;
}

int main(int argc, char *argv[], char *envp[]) {
	//메모리 초기화
	/*for (int i = 0; i < s; i++) {
		mem[i] = 0;
	}*/

	unsigned int memory_range[3]; // memory range
	memory_range[2] = 0;
	int d_exist = 0; // existance of -d
	int num_instruc = -1; // number of instruction to execute
	int assem_index = -1;

	for (int i = 0; i < argc; ++i) // assign command numbers to variables
	{
		//cout << argv[i] << endl;
		if (strcmp(argv[i], "-m") == 0)
		{
			//memory_range = argv[i + 1]; // save range of memory to check
			string memory_str = string(argv[i + 1]);
			int determinant = memory_str.find(':');
			if (determinant != string::npos)
			{
				memory_range[0] = stoi(memory_str.substr(0, determinant), NULL, 16);
				memory_range[1] = stoi(memory_str.substr(determinant + 1, memory_str.length() - determinant - 1), NULL, 16);
				memory_range[2] = 1;
				//cout << "memory_range[0] : " << memory_range[0] << endl;
				//cout << "memory_range[1] : " << memory_range[1] << endl;
			}

		}
		else if (strcmp(argv[i], "-d") == 0)
		{
			d_exist = 1; // save existance of -d
		}
		else if (strcmp(argv[i], "-n") == 0)
		{
			num_instruc = stoi(argv[i + 1]);// save number of instruction to execute
		}
		else if (string(argv[i]).find(".s") != string::npos) //(string(argv[i]).find(".s") != string(argv[i]).length()-1)
		{
			assem_index = i;
		}
	}
	if (assem_index == -1)
	{
		//cout << argv[argc - 1] << endl;
		cout << "No assembly file" << endl;
		return 0;
	}

	create_bin(argv[assem_index]);
	run_bin(num_instruc, d_exist, memory_range);

	if (!(d_exist) || num_instruc == 0) {
		print_reg(&PC, reg);
		if (memory_range[2] != 0) {
			print_mem(mem, memory_range[0], memory_range[1]); //print_mem(reinterpret_cast<unsigned char*>(mem), start, end);
		}
	}
	free(mem);
	return 0;
}
