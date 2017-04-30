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
    int DATA1 = 0;
    int DATA2 = 0;
    int ALUOp = 0;

    int reg_wt = 0;
    int mem_wt = 0;
    int mem_rd = 0;
    int mem2reg = 0;
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
이 뒤로는 모드 controll signal 인데,
reg_wt는 레지스터에 쓰는 여부를 결정
mem_wt는 메모리에 쓰는 여부를 결정
mem_rd는 메모리를 읽는 여부를 결정
mem2reg는 write back의 여부를 결정
ALUsrc는 imm과 DATA2 중에서 어느 부분을 ALU에 넣어줄지 결정
branch에는 branch 여부와 bne인지 beq인지를 확인하여 넣어준다.
jump에는 jump의 여부를 결정
Regdst는 reg2와 reg3중에서 어디가 목표 register인지 정해준다.
flush는 양수일 경우 flush의 횟수를, 음수일 경우, noop의 횟수를 결정한다.
AFTER_WB은 WB stage가 끝나고 WB 있었던 instruction의 PC값을 저장하고 pipeline  상태창에 출력하기 위해 만든 변수(NPC만 존재)
각 단계의 register는 _fix 로 끝나는 함수에 의해서 필요없는 신호를 0으로 초기화해준다.

출력의 형태는 -p 옵션이 있을때, 상단의 pipeline 출력에는 해당 스테이지를 마친 명령어의 시작하는 주소값이 나와있으며
따라서 하단의 register 값들은 각 pipeline 출력에 나온 PC 값들에 해당하는 명령어들이 각 스테이지를 마친 후의 레지스터 상태가 출력된다. 
ex) pipeline 상태창에 5번째칸 WB stage에 0x400008이 들어있으면 0x400008에 해당하는 instruction이 WB stage를 마친 결과를 출력함.

stall을 위한 빈 명령어는 32개가 전부 0인 명령어를 넣어(sll $0, $0, 0) 만들었다.

종료 조건은 유의미한 instruction을 가리키지 않을 때 PC값은 그 무의미한 곳을 가리키면서 종료된다.
