#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <map>
using namespace std;

unsigned int PC;
int s = 100;
char *I_mem = (char *)malloc(s);
char *data_mem = (char *)malloc(s);

//각 단계 매개 레지스터
int IF_ID_Res[20];
int ID_EX_Res[20];
int EX_MEM_Res[20];
int MEM_WB_Res[20];

string sample = "00000000000000000000000000000000";


struct STAGE_RES
{
  string instr;
  int NPC;
  int opcode;
  int REG1;
  int REG2;
  int REG3;
  int shift;
  int funct;
  int IMM;
  int ALU_OUT;
  int BR_TARGET;
  int MEM_OUT;
  //control
  int reg_wt;
  int PC;
  int mem_wt;
  int mem_rd;
  int sign_ex;
  int ALUSrc;
  int branch;
  int jump;
  int Regdst;
};

//addu 명령어 실행해보기

int type_checker(int op)
{
	if (op == 0) //R type
		return 2;
	else if ((op == 2) || (op == 3)) //J type
		return 1;
	else    // I type
		return 0;
}

string print_bin(unsigned int num, int len) {
	int length = len - 1;
	string result;
	for (length; length >= 0; --length)
	{
		result.append(to_string((num >> length) & 1));
	}
	return result;
}

void IF(unsigned int PC)
{

	// 메모리에서 명렁어 읽어오기
	string instruction;
	string instruction = print_bin(I_mem[PC],32);
	

	PC = PC + 4;
	IF_ID().PC = PC;

	//	type_checker(atoi(instruction.substr(0, 5).c_str()));

	////타입에 따라 담는 내용을 변경해줄 필요?
	//// IF/ID Res에 담기
	//IF_ID_Res[0] = atoi(bin.substr(0, 5).c_str());
	//IF_ID_Res[1] = atoi(bin.substr(6, 10).c_str());
	//IF_ID_Res[2] = atoi(bin.substr(11, 15).c_str());
	//IF_ID_Res[3] = atoi(bin.substr(16, 20).c_str());
	//IF_ID_Res[4] = atoi(bin.substr(21, 25).c_str());
	//IF_ID_Res[5] = atoi(bin.substr(26, 31).c_str());
	IF_ID().instr = instruction;
}

void ID(int* IF_ID)
{
	//case 별로 처리
	if (IF_ID_Res[0].substr(0, 5) == "000000" && IF_ID_Res[0].substr(0, 5) == "000000")
	{
		ID_EX_Res[1] = data_mem[IF_ID_Res[0].substr(6, 10)];
		ID_EX_Res[2] = data_mem[IF_ID_Res[0].substr(11, 15)];
		ID_EX_Res[3] = data_mem[IF_ID_Res[0].substr(16, 20)];
	}
}
	

void EX(int* ID_EX)
{
	
	//case 별로 처리
	
	string type;
	
	if (ID_EX_Res[0].substr(0, 5) == "000000" && ID_EX_Res[0].substr(0, 5) == "000000")
	{
		ID_EX_Res[3] = ID_EX_Res[2] + ID_EX_Res[1];
	}

	for (int i = 0; i < 7; i++){
		EX_MEM_Res[i] = ID_EX_Res[i];
	}
}

void MEM(int* EX_MEM)
{
	//load/store 예외처리

	for (int i = 0; i < 7; i++) {
		MEM_WB_Res[i] = EX_MEM_Res[i];
	}
}

void WB(int* MEM_WB)
{
	//write_mem으로 레지스터에 데이터 입력
}

void main()
{
	struct STAGE_RES IF_ID;
	struct STAGE_RES ID_EX;
	struct STAGE_RES EX_MEM;
	struct STAGE_RES MEM_WB;
	while (true)
	{
		WB(MEM_WB);
		MEM(EX_MEM);
		EX(ID_EX);
		ID(IF_ID);
		IF(PC);
	}
}
