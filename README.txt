/*README*/
/*2017-04-30 COMPUTER SYSTEM TEAM 4 - 김정훈, 김호준, 우정완 */

각 스테이지 사이의 레지스터의 역할로 다음의 구조체를 사용한다.
stuct STAGE_REG
    string instr = "00000000000000000000000000000000"
    unsigned int NPC = 0;
    int opcode = 0;
    int REG1 = 0;
    int REG2 = 0;
    int REG3 = 0;
    int shift = 0;
    int funct = 0;
    int IMM = 0;
    int rd = 0; 
    int ALU_OUT = 0;
    int BR_TARGET = 0;
    int MEM_OUT = 0;
    int reg_data = 0;
    int DATA1 = 0;
    int DATA2 = 0;
    int ALUOp = 0;

    int reg_wt = 0;
    int mem_wt = 0;
    int mem_rd = 0;
    int mem2reg = 0;
    int sign_ex = 0;
    int ALUSrc = 0;
    int branch = 0;
    int jump = 0;
    int Regdst = 0;
    int flush = 0;
    

이중에서 doc 파일에 없는 것들에 대해 추가적으로 설명을 하면 아래와 같다.
opcode, REG1, REG2, REG3, shift, funct 는 IF 단계에서 나누어 저장시켜 전달하는 것들이다.
rd 는 EX 단계에서 REG2 와 REG3 중 하나를 Regdst 정보를 바탕으로 정하여 저장하는 공간이다.
BR_TARGET에는 EX 단계에서 계산한 branch 주소값을 저장하여 MEM 단계에서 이로 분기한다.
DATA1과 DATA2는 ID 단계에서 read한 register의 값을 저장해주는 공간이다.
ALUOp는 ALU_controller 함수에서 ALU의 작동 방식을 고르기 위해 필요한 값이다.
이 뒤로는 모두 controll signal 인데, 
