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
  int wt_data
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
  int DATA1;
  int DATA2;
};

void create_bin(string file_name) {
	//start of file 02

	ifstream file_;
	string testcode; // s파일의 code 받기용
	string data = ""; //코드를 바이너리로 변환해서 저장

	int data_sign = 0; //data 영역 인식용
	int data_item = 0; //data 영역에서 받는 인자수
	int text_sign = 0; //text 영역 인식용
	int text_item = 0; // text부분에서 인수받는 수

	string label_temp = ""; //label 저장용
	string op_temp = ""; //opcode 저장용
	string reg_temp = ""; //레지스터 rd 또는 rt가 위치 변환을 위해 저장

	string la_temp = ""; // la용 처리용 temp
	int la_item = 0; // la 처리용 item

	map<string, unsigned int> label_map;
	map<string, string> label_connect_map;
	unsigned int data_adr = 0x10000000;
	unsigned int text_adr = 0x00400000;

	map <string, string> op;
	map <string, string> funct;
	map <string, string> type;

	//op.clear();
	//funct.clear();
	//type.clear();

	op.insert(pair<string, string>("addiu", "001001"));
	op.insert(pair<string, string>("addu", "000000"));
	op.insert(pair<string, string>("and", "000000"));
	op.insert(pair<string, string>("andi", "001100"));
	op.insert(pair<string, string>("beq", "000100"));
	op.insert(pair<string, string>("bne", "000101"));
	op.insert(pair<string, string>("j", "000010"));
	op.insert(pair<string, string>("jal", "000011"));
	op.insert(pair<string, string>("jr", "000000"));
	op.insert(pair<string, string>("lui", "001111"));
	op.insert(pair<string, string>("lw", "100011"));
	op.insert(pair<string, string>("nor", "000000"));
	op.insert(pair<string, string>("or", "000000"));
	op.insert(pair<string, string>("ori", "001101"));
	op.insert(pair<string, string>("sltiu", "001011"));
	op.insert(pair<string, string>("sltu", "000000"));
	op.insert(pair<string, string>("sll", "000000"));
	op.insert(pair<string, string>("srl", "000000"));
	op.insert(pair<string, string>("sw", "101011"));
	op.insert(pair<string, string>("subu", "000000"));

	funct.insert(pair<string, string>("addu", "100001"));
	funct.insert(pair<string, string>("and", "100100"));
	funct.insert(pair<string, string>("jr", "001000"));
	funct.insert(pair<string, string>("nor", "100111"));
	funct.insert(pair<string, string>("or", "100101"));
	funct.insert(pair<string, string>("sltu", "101011"));
	funct.insert(pair<string, string>("sll", "000000"));
	funct.insert(pair<string, string>("srl", "000010"));
	funct.insert(pair<string, string>("subu", "100011"));

	type.insert(pair<string, string>("addiu", "i"));
	type.insert(pair<string, string>("addu", "r"));
	type.insert(pair<string, string>("and", "r"));
	type.insert(pair<string, string>("andi", "i"));
	type.insert(pair<string, string>("beq", "i"));
	type.insert(pair<string, string>("bne", "i"));
	type.insert(pair<string, string>("j", "j"));
	type.insert(pair<string, string>("jal", "j"));
	type.insert(pair<string, string>("jr", "r"));
	type.insert(pair<string, string>("lui", "i"));
	type.insert(pair<string, string>("lw", "i"));
	type.insert(pair<string, string>("nor", "r"));
	type.insert(pair<string, string>("or", "r"));
	type.insert(pair<string, string>("ori", "i"));
	type.insert(pair<string, string>("sltiu", "i"));
	type.insert(pair<string, string>("sltu", "r"));
	type.insert(pair<string, string>("sll", "r"));
	type.insert(pair<string, string>("srl", "r"));
	type.insert(pair<string, string>("sw", "i"));
	type.insert(pair<string, string>("subu", "r"));
	type.insert(pair<string, string>("la", "la"));

	///////////////////////////////////////////////////////////////

	//label table 생성 및 data,text size 계산
	file_.open(file_name, ios::in | ios::_Nocreate);
	if (file_.is_open()) {
		while (file_ >> testcode) {

			//data, text 영역 구분
			if (testcode == ".data") {
				data_sign = 1;
			}
			else if (testcode == ".text") {
				data_sign = 0;
				text_sign = 1;
			}

			//data label 받기, data size 계산
			if (data_sign == 1) {

				if (testcode.substr(testcode.size() - 1, 1) == ":") {
					label_map.insert(pair<string, unsigned int>(testcode.substr(0, testcode.size() - 1), data_adr));
				}

				if (testcode == ".word") {
					data_item = 1;
				}
				else if (data_item == 1) {

					if (testcode.substr(0, 2) == "0x") {
						testcode = to_string(strtoul(testcode.c_str(), NULL, 16));
					}
					write_mem(data_adr, atoi(testcode.c_str()));
					data_adr += 4;
					data_item = 0;
				}
			}

			//text label 받기, data size 계산
			if (text_sign == 1) {

				if (testcode.substr(testcode.size() - 1, 1) == ":") {
					if (label_temp != "") {
						label_connect_map.insert(pair<string, string>(label_temp, label_temp + "_end"));
						label_map.insert(pair<string, unsigned int>(label_temp + "_end", text_adr - 4));
					}
					label_map.insert(pair<string, unsigned int>(testcode.substr(0, testcode.size() - 1), text_adr));
					label_temp = testcode.substr(0, testcode.size() - 1);
				}

				// la 분리
				if (testcode == "la") {
					la_item = 2;
				}
				else if (la_item == 2) {
					--la_item;
				}
				else if (la_item == 1) {
					--la_item;
					//label 값 읽어오기
					if (label_map.find(testcode) != label_map.end()) {
						la_temp = to_string(label_map.find(testcode)->second);
					}

					if (testcode.substr(0, 2) == "0x") {
						testcode = strtoul(testcode.c_str(), NULL, 16);
					}

					text_adr += 4;

					if ((0x0000FFFF & atoi(testcode.c_str())) != 0) {
						text_adr += 4;
					}
				}
				if (op.find(testcode) != op.end()) {
					text_adr += 4;
				}
			}
		}
		if (label_temp != "") {
			label_connect_map.insert(pair<string, string>(label_temp, label_temp + "_end"));
			label_map.insert(pair<string, unsigned int>(label_temp + "_end", text_adr - 4));
		}
		data = print_bin(text_adr - 0x00400000, 32) + "\n" + print_bin(data_adr - 0x10000000, 32) + "\n";
	}
	file_.close();

	//textaddress 초기화
	text_adr = 0x00400000;

	//text영역 instruction 받기
	file_.open(file_name, ios::in | ios::_Nocreate);
	if (!file_.is_open())
		cout << "not exist file" << endl; // 파일 없을 시 오류 반환
	else
	{
		while (file_ >> testcode) {

			// 첫 개행문자 제거
			/*if (data.substr(0, 1) == "\n") {
			data.erase(0, 1);
			}*/

			// "," 제거
			if (testcode.substr(testcode.size() - 1, 1) == ",") {
				testcode.erase(testcode.size() - 1);
			}

			// la 분리
			if (testcode == "la") {
				la_item = 2;
			}
			else if (la_item == 2) {
				la_temp = testcode.substr(1);
				--la_item;
			}
			else if (la_item == 1) { // (주의)data의 array로 받을 때 +연산 고려 안 했음
									 /*for (int m = testcode.size() - 1; m == 0;) {
									 if (testcode.substr(m, 1) == "+") {
									 testcode = to_string(read_mem(label_map.find(testcode.substr(0, m))->second + atoi(testcode.substr(m).c_str())));
									 }
									 }*/
				--la_item;
				//label 값 읽어오기
				if (label_map.find(testcode) != label_map.end()) {
					testcode = to_string(label_map.find(testcode)->second);
					//cout << read_mem(label_map.find(testcode)->second) << endl;
				}

				if (testcode.substr(0, 2) == "0x") {
					testcode = to_string(strtoul(testcode.c_str(), NULL, 16));
				}

				//label 값 비트연산해서 instruction 추가하기
				data = data + op.find("lui")->second + "00000" + print_bin(atoi(la_temp.c_str()), 5)
					+ print_bin(atoi(testcode.c_str()) >> 16, 16) + "\n";
				text_adr += 4;

				if ((0x0000FFFF & atoi(testcode.c_str())) != 0) {
					data = data + op.find("ori")->second + print_bin(atoi(la_temp.c_str()), 5) + print_bin(atoi(la_temp.c_str()), 5)
						+ print_bin(0xFFFF & atoi(testcode.c_str()), 16) + "\n";
					text_adr += 4;
				}
			}

			// opcode 6자리 입력, 받을 인자 갯수 지정
			if (op.find(testcode) != op.end()) {
				data = data + op.find(testcode)->second;

				text_adr += 4;
				op_temp = testcode;

				if (type.find(op_temp)->second == "r")
					text_item = 3;
				else if (type.find(op_temp)->second == "i")
					text_item = 3;
				else if (type.find(op_temp)->second == "j")
					text_item = 1;
			}

			else if (text_item != 0) {
				int a;
				int len;

				if (testcode.substr(0, 2) == "0x") {
					testcode = to_string(strtoul(testcode.c_str(), NULL, 16));
				}

				if (op_temp == "lui" && text_item == 3) {
					a = atoi(testcode.substr(1).c_str());
					len = 5;
					data = data + "00000";
					reg_temp = print_bin(a, len);
					--text_item;
				}
				else if ((op_temp == "beq" || op_temp == "bne") && text_item == 1) {
					len = 16;
					if (label_map.find(testcode) != label_map.end()) {
						data = data + reg_temp + print_bin((label_map.find(testcode)->second - text_adr) >> 2, len) + "\n";
					}
					else {
						if (testcode.substr(0, 2) == "0x") {
							testcode = to_string(strtoul(testcode.c_str(), NULL, 16));
						}
						a = atoi(testcode.substr(0).c_str());
						data = data + reg_temp + print_bin(a, len) + "\n";
					}
					--text_item;
				}
				else if (op_temp == "jr") {
					a = atoi(testcode.substr(1).c_str());
					len = 5;
					data = data + print_bin(a, len) + "000000000000000" + funct.find(op_temp)->second + "\n";
					text_item = text_item - 2;
				}
				else if ((op_temp == "sll" || op_temp == "srl") && text_item == 3) {
					a = atoi(testcode.substr(1).c_str());
					len = 5;
					data = data + "00000";
					reg_temp = print_bin(a, len);
				}
				else if ((op_temp == "sll" || op_temp == "srl") && text_item == 1) {
					a = atoi(testcode.substr(0).c_str());
					len = 5;
					data = data + reg_temp + print_bin(a, len) + funct.find(op_temp)->second + "\n";
				}
				else if ((op_temp == "lw" || op_temp == "sw") && text_item == 2) {
					int j = 0;
					while (true) {
						if (testcode.substr(j, 1) == "(") {
							break;
						}
						else {
							++j;
						}
					}
					//register 넣기
					a = atoi(testcode.substr(j + 2, testcode.size() - (j + 3)).c_str());
					len = 5;
					data = data + print_bin(a, len) + reg_temp;
					//offset 넣기
					a = atoi(testcode.substr(0, j).c_str());
					len = 16;
					data = data + print_bin(a, len) + "\n";
					text_item = text_item - 2;
				}
				else if ((type.find(op_temp)->second == "i" || "r") && text_item == 3) {
					a = atoi(testcode.substr(1).c_str());
					len = 5;
					reg_temp = print_bin(a, len);
				}
				else if (type.find(op_temp)->second == "i" && text_item == 1) {
					a = atoi(testcode.substr(0).c_str());
					len = 16;
					data = data + reg_temp + print_bin(a, len) + "\n";
				}
				else if (type.find(op_temp)->second == "j") {
					len = 26;
					if (label_map.find(testcode) != label_map.end()) {
						data = data + print_bin(((label_map.find(testcode)->second & 0x0FFFFFFC) >> 2), len) + "\n";
					}
					else {
						a = atoi(testcode.substr(0).c_str());
						data = data + print_bin(a, len) + "\n";
					}
				}
				else if (type.find(op_temp)->second == "r" && text_item == 1) {
					a = atoi(testcode.substr(1).c_str());
					len = 5;
					data = data + print_bin(a, len) + reg_temp + "00000" + funct.find(op_temp)->second + "\n";
				}
				else {
					a = atoi(testcode.substr(1).c_str());
					len = 5;
					data = data + print_bin(a, len);
				}

				--text_item;

				if (text_item <= 0) {
					op_temp.clear();
					text_item = 0;
				}
			}
		}
		//cout << data << endl; // 확인용
		map<string, unsigned int>::iterator it;
		for (it = label_map.begin(); it != label_map.end(); it++) {
			//cout << it->first << " " << it->second << endl;
			if (it->first == "main_end") {
				EOM = it->second;
			}
		}
	}

	file_.close();

	ofstream output;

	output.open("binary.txt", ios::out);
	output << data;
	output.close();
	//end of file 02
}


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

struct STAGE_RES IF(unsigned int PC)
{
	struct STAGE_RES IF_ID;
	// 메모리에서 명렁어 읽어오기
	string instruction;
	string instruction = print_bin(I_mem[PC], 32); //convert instruction integer into binary string


	PC = PC + 4;
	IF_ID.PC = PC; //Save PC value to IF_ID_Register

	//	type_checker(atoi(instruction.substr(0, 5).c_str()));

	////타입에 따라 담는 내용을 변경해줄 필요?
	//// IF/ID Res에 담기
	//IF_ID_Res[0] = atoi(bin.substr(0, 5).c_str());
	//IF_ID_Res[1] = atoi(bin.substr(6, 10).c_str());
	//IF_ID_Res[2] = atoi(bin.substr(11, 15).c_str());
	//IF_ID_Res[3] = atoi(bin.substr(16, 20).c_str());
	//IF_ID_Res[4] = atoi(bin.substr(21, 25).c_str());
	//IF_ID_Res[5] = atoi(bin.substr(26, 31).c_str());
	IF_ID.instr = instruction; //Save PC value to IF_ID_Register
	return IF_ID;
}

void ID(STAGE_REG IF_ID)
{
	//case 별로 처리
	if (IF_ID_Res[0].substr(0, 5) == "000000" && IF_ID_Res[0].substr(0, 5) == "000000")
	{
		ID_EX_Res[1] = data_mem[IF_ID_Res[0].substr(6, 10)];
		ID_EX_Res[2] = data_mem[IF_ID_Res[0].substr(11, 15)];
		ID_EX_Res[3] = data_mem[IF_ID_Res[0].substr(16, 20)];
	}
}
	

void EX(STAGE_REG ID_EX)
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

void MEM(STAGE_REG EX_MEM)
{
	//load/store 예외처리

	for (int i = 0; i < 7; i++) {
		MEM_WB_Res[i] = EX_MEM_Res[i];
	}
}

void WB(STAGE_REG MEM_WB)
{
	if (MEM_WB.mem_wt == 1) {
		Reg(MEM_WB.rd) == MEM_WB.wt_data;
	}
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
		IF_ID=IF(PC);
	}
}
