#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <map>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//global variables
int res[32];
size_t s = sizeof(char) * 4 * 1024 * 1024 * 1024;
unsigned char *mem = (unsigned char *)malloc(s);
unsigned int PC;
unsigned int PC_temp = 0;
unsigned int EOM = 0; // end of main
int END_warn = 0;
int text_size = 0;


using namespace std;
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
struct STAGE_RES
{
	string instr;
	int NPC;
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
	int wt_data;
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

struct STAGE_RES IF(unsigned int PC)
{
	struct STAGE_RES IF_ID;
	// 메모리에서 명렁어 읽어오기
	string instruction;
	string instruction = print_bin(mem[PC], 32); //convert instruction integer into binary string
	
	PC = PC + 4;
	IF_ID.PC = PC; //Save PC value to IF_ID_Register
	IF_ID.instr = instruction; //Save PC value to IF_ID_Register
	
	return IF_ID;
}

void ID(STAGE_RES IF_ID)
{
}


void EX(STAGE_RES ID_EX)
{
	////

	///////// R type
	if (ID_EX.funct == 0x21) //ADDU (2)
	{
		ID_EX.ALU_OUT = ID_EX.DATA1 + ID_EX.DATA2;
		res[ID_EX.rd] = ID_EX.ALU_OUT;
	}
	else if (ID_EX.funct == 0x24) //AND (3)
	{
		res[rd] = res[rs] & res[rt];
	}
	else if (ID_EX.funct == 0x27) //12 nor
	{
		res[rd] = (~(res[rs] | res[rt]));
	}
	else if (ID_EX.funct == 0x25) //13 or
	{
		res[rd] = (res[rs] | res[rt]);
	}
	else if (ID_EX.funct == 0x2b) //16 sltu
	{
		res[rd] = (res[rs] < res[rt] ? 1 : 0);
	}
	else if (ID_EX.funct == 0) //17 sll
	{
		res[rd] = res[rt] << shamt;
	}
	else if (ID_EX.funct == 2) //18 srl
	{
		res[rd] = res[rt] >> shamt;
	}
	else if (ID_EX.funct == 0x23) //20 subu
	{
		res[rd] = res[rs] - res[rt];
	}

	//////// I type
	if (ID_EX.opcode == 9) //ADDIU (1)
	{
		res[rt] = res[rs] + imm;
	}
	else if (ID_EX.opcode == 0xc) //ANDI (4)
	{
		res[rt] = res[rs] & imm;
	}
	else if (ID_EX.opcode == 4) //BEQ (5)
	{
		if (res[rs] == res[rt])
		{
			PC = PC + 4 * (imm);
		}
	}
	else if (ID_EX.opcode == 5) //BNE (6)
	{
		if (res[rs] != res[rt])
		{
			PC = PC + 4 * (imm);
		}
	}
	else if (ID_EX.opcode == 0xf) //LUI (10)
	{
		res[rt] = imm << 16;
	}
	else if (ID_EX.opcode == 0x23) //11 lw
	{
		res[rt] = read_mem(res[rs] + imm);
	}
	
	else if (ID_EX.opcode == 0xd) //14 ori
	{
		res[rt] = (res[rs] | imm);
	}
	else if (ID_EX.opcode == 0xb) //15 sltiu
	{
		if (res[rs] < imm) {
			res[rt] = 1;
		}
		else {
			res[rt] = 0;
		}
	}
	else if (ID_EX.opcode == 0x2b) //19 sw
	{
		write_mem(res[rs] + imm, res[rt]);
	}
}

void MEM(STAGE_RES EX_MEM)
{
}

void WB(STAGE_RES MEM_WB)
{
	if (MEM_WB.mem_wt == 1) {
		res[MEM_WB.rd] == MEM_WB.wt_data;
	}
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

unsigned int convert210(string bin_num) {
	int pow = 1;
	unsigned int result = 0;
	for (int i = bin_num.length() - 1; i >= 0; --i)
	{
		if (bin_num.at(i) == '1')
			result = result + pow;
		pow = pow * 2;
	}
	return result;
}

int write_mem(int address, unsigned int data) { //write 4byte data at address
	for (int i = 0; i < 4; i++) {
		mem[address + i] = convert210(print_bin(data, 32).substr(i * 8, 8));
	}
	return 0;
}

unsigned int read_mem(int address) { //read 4byte from address
	unsigned int result = 0;

	for (int i = 0; i < 4; i++) {
		result += (mem[address + i] << (8 * (3 - i)));
	}

	return result;
}

int type_checker(int op) {
	if (op == 0) //R type
		return 2;
	else if ((op == 2) || (op == 3)) //J type
		return 1;
	else    // I type
		return 0;
}

int bin_parser(string oneline) {
	if (oneline.length() != 32)
	{
		cout << "bin line error" << endl;
		return 1;
	}
	string op_str = oneline.substr(0, 6);
	int op_int = convert210(op_str);
	int type = type_checker(op_int);
	if (type == 2) // R type
	{
		unsigned int funct_int = convert210(oneline.substr(26, 6));
		unsigned int rs_int = convert210(oneline.substr(6, 5));
		unsigned int rt_int = convert210(oneline.substr(11, 5));
		unsigned int rd_int = convert210(oneline.substr(16, 5));
		unsigned int shamt_int = convert210(oneline.substr(21, 5));
	}
	else if (type == 1)
	{
		unsigned int jumptarget_int = convert210(oneline.substr(6, 26));
	}
	else
	{

		unsigned int rs_int = convert210(oneline.substr(6, 5));
		unsigned int rt_int = convert210(oneline.substr(11, 5));
		int imm_int = convert210(oneline.substr(16, 16));
		if (imm_int >> 15 == 1) {
			imm_int = imm_int | 0xFFFF0000;
		}
	}
	return 0;
}

void save_ins(int* text_length, int* data_length, unsigned char * mem, int num_instruc = 0) {
	int line_count = 0;
	ifstream ifile;
	char line[200]; // 한 줄씩 읽어서 임시로 저장할 공간
	ifile.open("binary.txt");  // 파일 열기
							   //cout << "num_instruc" << num_instruc << endl;
	if (ifile.is_open())
	{
		while (ifile.getline(line, sizeof(line))) // 한 줄씩 읽어 처리를 시작한다.
		{
			line_count += 1;

			if (line_count == 1) {
				*text_length = convert210(line);


			}
			else if (line_count == 2) {
				*data_length = convert210(line);

			}
			else {
				string str_line(line);
				for (int i = 0; i < 4; i++)
				{
					mem[0x400000 + (line_count - 3) * 4 + i] = convert210(str_line.substr(0 + i * 8, 8));
				}
			}
		}
	}
	ifile.close(); // 파일 닫기
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
* PC: pointer of PC
* registers: array of the registers(R0~R31) of which size of element is 4 bytes(=32bits).
*
* print values of PC and registers
*/
void print_reg(unsigned int *PC, int *registers)
{
	int i;

	printf("Current register values :\n");
	printf("-----------------------------------------\n");
	printf("PC: 0x%.8X\n", *PC);

	printf("Registers:\n");
	for (i = 0; i<32; i++) {
		printf("R%d: 0x%.8X\n", i, registers[i]);
	}

	printf("\n");
}

/*
* mem: base address of emulated memory
* addr_begin: begin address of the memory range that we want to print
* addr_end: end address of the memory range that we want to print
*
* print memory range from addr_begin to addr_end
*/
void print_mem(unsigned char *mem, unsigned int addr_begin, unsigned int addr_end)
{
	unsigned int cur;
	int i;

	printf("Memory content [0x%.8X..0x%.8X]\n", addr_begin, addr_end);
	cur = addr_begin;
	while (cur <= addr_end) {
		printf("0x%.8X: 0x%.2X", cur, mem[cur]);
		for (i = 1; i<4; i++) {
			printf("%.2X", mem[cur + i]);
		}
		cur += i;
		printf("\n");
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//assembly file -> binary file
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
			else if (la_item == 1) {

				--la_item;
				//label 값 읽어오기
				if (label_map.find(testcode) != label_map.end()) {
					testcode = to_string(label_map.find(testcode)->second);
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

int main(int argc, char *argv[], char *envp[]) {
	//메모리 초기화
	for (int i = 0; i < s; i++) {
		mem[i] = 0;
	}

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
	if (!(d_exist) || num_instruc == 0) {
		if (!(0x400000 <= PC && PC < (0x400000 + text_size))) {
			PC = PC_temp;
		}

		print_reg(&PC, res);

		if (memory_range[2] != 0) {
			print_mem(mem, memory_range[0], memory_range[1]); //print_mem(reinterpret_cast<unsigned char*>(mem), start, end);
		}
	}

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
		IF_ID = IF(PC);
	}

	free(mem);
	return 0;
}
