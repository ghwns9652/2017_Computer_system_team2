int memory[];
int reg[];
int IF_ID_reg[];
int ID_EX_reg[];
int EX_MEM_reg[];
int MEM_WB_reg[];

void WB(int a = MEM_WB_reg[1], int b = MEM_WB_reg[2]) {
	if (MEM_WB_reg[0] == 1) {
		reg[a] = b;
	}
}

void MEM(int a = MEM_WB_reg[]) {

}

void EX() {

}

void ID() {

}

void IF() {

}

int main() {
	WB();
	MEM();
	EX();
	ID();
	IF();
}
