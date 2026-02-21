/*
 RS : PTD15, RW : PTD14, EN : PTD13, LDC7~4 : PTD12~9
 SW5~1 : PTD2,3,4,5,8
 SEG_A~G : PTE1~7
 SEG_COM6~3 : PTE8,9,14,15
 SEG_CLOCK : PTE16
 VR : PTC15
 BUZZER : PTC0
 LED8~5 : PTC1~4, LED3~1 : 7~9
 MOTOR PWM :( PTB14, 15 )// PTD0, 1
 */

#include "device_registers.h" // S32K144의 하드웨어 레지스터 정의 헤더
#include "clocks_and_modes.h" // 클럭 설정 관련 함수 헤더
#include "lcd1602A.h" // LCD1602A 제어용 헤더파일
#include "ADC.h" // ADC 초기화 및 측정 함수 포함
#include <stdio.h> // 표준 입출력 함수
#include <string.h> // 문자열 처리 함수
#include <stdlib.h> // 일반 유틸리티 함수, 예: 난수 생성 등

// 0~9까지 7세그먼트로 표현하기 위한 segment 패턴 배열
unsigned int FND_DATA[10] = {0x7E, 0x0C, 0xB6, 0x9E, 0xCC, 0xDA, 0xFA, 0x4E, 0xFE, 0xCE};

// 각 자릿수의 공통 핀 제어 값 (COM 핀)
unsigned int FND_SEL[4] = {0x0100, 0x0200, 0x4000, 0x8000};

int mode = 0; // 현재 모드를 저장하는 변수
int seg_num = 0; // 7세그먼트에 출력할 점수 등 값

#define MAX_MODE_NUM 6 // 총 메인 모드 수

// ===== 모드 정의 =====
#define MODE_ENTER_PASSWORD_0 0 // 비밀번호 입력 모드
#define MODE_LOCK_PHONE_0 1 // 잠금 모드 메인
#define MODE_LOCK_PHONE_1 10 // 잠금 완료 이후 상태
#define MODE_CHANGE_PASSWORD_0 2 // 비밀번호 변경 시작
#define MODE_CHANGE_PASSWORD_1 20 // 현재 비밀번호 확인 단계
#define MODE_CHANGE_PASSWORD_2 21 // 새 비밀번호 입력 단계
#define MODE_CHANGE_PASSWORD_3 22 // 새 비밀번호 재입력 단계
#define MODE_SET_TIMER_0 3 // 타이머 설정 메인
#define MODE_SET_TIMER_1 30 // 타이머 설정 내부
#define MODE_SET_TIMER_2 31 // 분 설정
#define MODE_SET_TIMER_3 32 // 초 설정 준비
#define MODE_SET_TIMER_4 33 // 초 설정
#define MODE_SET_TIMER_5 34 // 타이머 실행 중
#define MODE_SET_TIMER_6 35 // 타이머 종료 상태
#define MODE_PLAY_GAME1_0 4 // 게임1 메인 화면
#define MODE_PLAY_GAME1_1 40 // 게임1 시작 선택
#define MODE_PLAY_GAME1_2 41 // 게임1 종료 선택
#define MODE_PLAY_GAME1_3 42 // 게임1 실행 중
#define MODE_PLAY_GAME1_4 43 // 게임1 종료 후
#define MODE_PLAY_GAME2_0 5 // 게임2 메인 화면
#define MODE_PLAY_GAME2_1 50 // 게임2 시작 선택
#define MODE_PLAY_GAME2_2 51 // 게임2 종료 선택
#define MODE_PLAY_GAME2_3 52 // 게임2 실행 준비
#define MODE_PLAY_GAME2_4 53 // 게임2 실행 중
#define MODE_PLAY_GAME2_5 54 // 게임2 실패
#define MODE_PLAY_GAME2_6 55 // 게임2 예비 상태
#define MODE_REMOTE_0 6 // 원격 제어 메인(선풍기)
#define MODE_REMOTE_1 60 // 선풍기 정지 상태
#define MODE_REMOTE_2 61 // 팬 파워 1단계
#define MODE_REMOTE_3 62 // 팬 파워 2단계
#define MODE_REMOTE_4 63 // 팬 파워 3단계

// ===== 비밀번호 입력 관련 변수 =====
unsigned int psw_num = 5; // 비밀번호 입력 위치 (5번째부터 시작)

// 비밀번호 입력 키 코드 정의
#define PSW_0 0
#define PSW_1 1
#define PSW_2 2
#define PSW_3 3
#define PSW_4 4
#define PSW_5 5
#define PSW_6 6
#define PSW_7 7
#define PSW_8 8
#define PSW_9 9
#define PSW_CLEAR 30 // 비밀번호 초기화

char msg_mode0_psw[17] = "     ******     "; // 현재 입력된 비밀번호 문자열
char msg_mode0_psw_answer[17] = "     123412     "; // 정답 비밀번호 문자열

// 새 비밀번호 입력용 정의
#define NEW_PSW_0 10
#define NEW_PSW_1 11
#define NEW_PSW_2 12
#define NEW_PSW_3 13
#define NEW_PSW_4 14
#define NEW_PSW_5 15
#define NEW_PSW_6 16
#define NEW_PSW_7 17
#define NEW_PSW_8 18
#define NEW_PSW_9 19
#define NEW_PSW_CLEAR 31

// 새 비밀번호 재입력 확인용 정의
#define NEW_RE_PSW_0 20
#define NEW_RE_PSW_1 21
#define NEW_RE_PSW_2 22
#define NEW_RE_PSW_3 23
#define NEW_RE_PSW_4 24
#define NEW_RE_PSW_5 25
#define NEW_RE_PSW_6 26
#define NEW_RE_PSW_7 27
#define NEW_RE_PSW_8 28
#define NEW_RE_PSW_9 29
#define NEW_RE_PSW_CLEAR 32

char msg_mode1_psw_new[17] = "     ******     "; // 새 비밀번호 입력 문자열
char msg_mode1_psw_new_re[17] = "     ******     "; // 새 비밀번호 재입력 문자열

// ===== 게임1 피하기 관련 =====
#define AVOID_POOP_TO_START 40
#define AVOID_POOP_TO_EXIT 41
#define AVOID_POOP_START 42
#define AVOID_POOP_EXIT 43
#define AVOID_POOP_MOVE 44

unsigned int avoid_poop_player = 0; // 플레이어 위치 (0: 위, 1: 아래)
char avoid_poop_row0[17] = "                "; // 게임 첫째 줄 상태
char avoid_poop_row1[17] = "                "; // 게임 둘째 줄 상태

// ===== 게임2 타이밍 맞추기 관련 =====
int LED_life = 3; // LED 게임에서 남은 생명 수
int record[4] = {0}; // 각 LED에 대한 입력 기록 (타이밍 체크)
int LED_num = 0; // 현재 동작 중인 LED 번호

#define TIMING_TO_START 50 // 게임2 시작 선택
#define TIMING_TO_EXIT 51 // 게임2 종료 선택
#define TIMING_START 52 // 게임2 시작 상태
#define TIMING_EXIT 53 // 게임2 종료 상태
#define TIMING_BTN_1 54 // 버튼 1 입력
#define TIMING_BTN_2 55 // 버튼 2 입력
#define TIMING_BTN_3 56 // 버튼 3 입력
#define TIMING_BTN_4 57 // 버튼 4 입력

// ===== 타이머 설정 관련 =====
#define TIMER_SECOND 60
#define TIMER_START 61
#define TIMER_EXIT 62

int timer_min = 0; // 타이머 분
int timer_sec = 0; // 타이머 초

// ===== 팬 제어 모드 정의 =====
#define FAN_STOP 70 // 팬 정지
#define FAN_POWER_1 71 // 팬 속도 1
#define FAN_POWER_2 72 // 팬 속도 2
#define FAN_POWER_3 73 // 팬 속도 3
#define FAN_EXIT 74 // 팬 제어 종료

unsigned int fan_power = 0; // 현재 팬 속도 설정 값

// ===== 모드 이동 관련 키 정의 =====
#define MODE_LEFT 100 // 왼쪽 모드로 이동
#define MODE_RIGHT 101 // 오른쪽 모드로 이동
#define MODE_SELECT 102 // 현재 모드 선택
#define MODE_HOME 103 // 홈으로 이동

... // 기존 내용 생략

// ===== LPIT 타이머 상태 변수 및 외부 인터럽트 상태 변수 =====
int lpit0_ch0_flag_counter = 0; // LPIT0 채널 0 타임아웃 카운터 (delay_ms 등에서 사용)
int lpit0_ch1_flag_counter = 0; // 게임1에서 사용하는 LPIT0 채널 1 카운터
int lpit0_ch2_flag_counter = 0; // 타이머 기능에서 사용하는 LPIT0 채널 2 카운터
int lpit0_ch3_flag_counter = 0; // 게임2에서 사용하는 LPIT0 채널 3 카운터
int External_PIN = -1; // 버튼 인터럽트에서 눌린 키 코드 저장 변수

// ===== 기본 초기화 함수 정의 =====

void LPIT0_init(uint32_t delay); // LPIT 타이머 초기화 함수 선언

void WDOG_disable(void) {
    WDOG->CNT = 0xD928C520; // Watchdog 해제를 위한 Unlock 시퀀스
    WDOG->TOVAL = 0x0000FFFF; // 최대 타임아웃으로 설정
    WDOG->CS = 0x00002100; // 워치독 비활성화 설정
}

void delay_ms(uint32_t ms) {
    LPIT0_init(ms * 1000); // 1ms = 1000us 기준으로 변환하여 설정
    while (0 == (LPIT0->MSR & LPIT_MSR_TIF0_MASK)) {} // 완료될 때까지 대기
    lpit0_ch0_flag_counter++; // 카운터 증가
    LPIT0->MSR |= LPIT_MSR_TIF0_MASK; // 인터럽트 플래그 클리어
}

void delay_us(volatile int us) {
    LPIT0_init(us); // us 단위로 설정
    while (0 == (LPIT0->MSR & 0x01)) {} // 완료될 때까지 대기
    lpit0_ch0_flag_counter++; // 카운터 증가
    LPIT0->MSR |= 0x00;//오류!! LPIT0->MSR |= LPIT_MSR_TIF0_MASK;가 올바른 인터럽트 플래그 클리어(write 1 to clear)
}

void LPIT0_init(uint32_t delay) {
    uint32_t timeout;

    LPIT0->MCR |= LPIT_MCR_M_CEN_MASK; // LPIT 모듈 활성화

    PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6); // 클럭 소스 SPLL(40MHz)
    PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; // 클럭 게이트 활성화

    timeout = delay * 40; // 타이머 카운트 값 계산 (40MHz 기준, 1us로 주기설정. 1us당 40클럭)
    LPIT0->TMR[0].TVAL = timeout; // 채널 0 타이머 값 설정 (delay용)
    LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK; // 채널 0 타이머 시작

    // 게임1용 타이머 (0.5초 정도)
    LPIT0->TMR[1].TVAL = 20000000;
    LPIT0->TMR[1].TCTRL = 0x00000001; // 채널 1 타이머 시작

    // 타이머 기능용 (1초)
    LPIT0->TMR[2].TVAL = 40000000;
    LPIT0->TMR[2].TCTRL = 0x00000001;

    // 게임2용 타이머 (0.375초)
    LPIT0->TMR[3].TVAL = 15000000;
    LPIT0->TMR[3].TCTRL = 0x00000001;
}
... // 기존 내용 생략

void PORT_init(void) {
    PCC->PCCn[PCC_PORTC_INDEX] = PCC_PCCn_CGC_MASK; // PORTC 클럭 활성화

    // PTC0~4,7~9 핀을 출력으로 설정 (LED, BUZZER 등)
    PTC->PDDR |= 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<7 | 1<<8 | 1<<9;

    // 해당 핀들을 GPIO 모드로 설정
    PORTC->PCR[0] = PORT_PCR_MUX(1);
    PORTC->PCR[1] = PORT_PCR_MUX(1);
    PORTC->PCR[2] = PORT_PCR_MUX(1);
    PORTC->PCR[3] = PORT_PCR_MUX(1);
    PORTC->PCR[4] = PORT_PCR_MUX(1);
    PORTC->PCR[7] = PORT_PCR_MUX(1);
    PORTC->PCR[8] = PORT_PCR_MUX(1);
    PORTC->PCR[9] = PORT_PCR_MUX(1);

    // ===== PORTD 설정 =====
    PCC->PCCn[PCC_PORTD_INDEX] &= ~PCC_PCCn_CGC_MASK; // 일단 비활성화 후
    PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_PCS(0x001); // 클럭 소스 설정. SOSCDIV2_CLK 정확하지만 전력 많이 듦. Uart, ADC등에 사용
    PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK; // 다시 활성화

    // FTM2(PWM) 사용을 위한 클럭 설정
    PCC->PCCn[PCC_FTM2_INDEX] &= ~PCC_PCCn_CGC_MASK;
    PCC->PCCn[PCC_FTM2_INDEX] |= (PCC_PCCn_PCS(1) | PCC_PCCn_CGC_MASK);

    // PTD2,3,4,5,8은 입력 (스위치), 나머지는 출력으로 설정
    PTD->PDDR &= ~(1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<8);
    PTD->PDDR |= 1<<9 | 1<<10 | 1<<11 | 1<<12 | 1<<13 | 1<<14 | 1<<15;

    // PWM 출력 핀 설정 (PTD0, PTD1 -> FTM2)
    PORTD->PCR[0] = PORT_PCR_MUX(2); // FTM2_CH0
    PORTD->PCR[1] = PORT_PCR_MUX(2); // FTM2_CH1

    // 스위치 입력 핀 (PTD2~5,8) + pull-up 설정 (10 << 16)->오류!! pull up이면 11<<16이어야 함. 게다가 PE, PS위치는 1,0이므로 PORTD->PCR[2] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; 가 옳은 표현

    PORTD->PCR[2] = PORT_PCR_MUX(1) | (10 << 16);
    PORTD->PCR[3] = PORT_PCR_MUX(1) | (10 << 16);
    PORTD->PCR[4] = PORT_PCR_MUX(1) | (10 << 16);
    PORTD->PCR[5] = PORT_PCR_MUX(1) | (10 << 16);
    PORTD->PCR[8] = PORT_PCR_MUX(1) | (10 << 16);

    // LCD 제어 핀들 (PTD9~15)
    PORTD->PCR[9] = PORT_PCR_MUX(1);
    PORTD->PCR[10] = PORT_PCR_MUX(1);
    PORTD->PCR[11] = PORT_PCR_MUX(1);
    PORTD->PCR[12] = PORT_PCR_MUX(1);
    PORTD->PCR[13] = PORT_PCR_MUX(1);
    PORTD->PCR[14] = PORT_PCR_MUX(1);
    PORTD->PCR[15] = PORT_PCR_MUX(1);

    // ===== PORTE 설정 =====
    PCC->PCCn[PCC_PORTE_INDEX] = PCC_PCCn_CGC_MASK; // PORTE 클럭 활성화

    // PTE1~7,8,9,14,15,16을 출력으로 설정 (7-segment)
    PTE->PDDR |= 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<14 | 1<<15 | 1<<16;

    // 해당 핀들을 GPIO 모드로 설정
    PORTE->PCR[1] = PORT_PCR_MUX(1);
    PORTE->PCR[2] = PORT_PCR_MUX(1);
    PORTE->PCR[3] = PORT_PCR_MUX(1);
    PORTE->PCR[4] = PORT_PCR_MUX(1);
    PORTE->PCR[5] = PORT_PCR_MUX(1);
    PORTE->PCR[6] = PORT_PCR_MUX(1);
    PORTE->PCR[7] = PORT_PCR_MUX(1);
    PORTE->PCR[8] = PORT_PCR_MUX(1);
    PORTE->PCR[9] = PORT_PCR_MUX(1);
    PORTE->PCR[14] = PORT_PCR_MUX(1);
    PORTE->PCR[15] = PORT_PCR_MUX(1);
    PORTE->PCR[16] = PORT_PCR_MUX(1);

    // 초기 LED OFF (PTC1~4,7~9), active low 방식
    PTC->PSOR |= 0x39E;
}

void FTM_init(void) {
    PCC->PCCn[PCC_FTM0_INDEX] &= ~PCC_PCCn_CGC_MASK; // 클럭 설정을 위해 FTM0 클럭 비활성화
    PCC->PCCn[PCC_FTM0_INDEX] |= PCC_PCCn_PCS(0b010) | PCC_PCCn_CGC_MASK; // 클럭 소스: 8MHz (0b010 = SIRCDIV1_CLK = 8MHz, 정확성 떨어지고 저전력 사용)

    // 채널 2, 3을 PWM 출력 모드로 활성화 + 프리스케일 설정 PS(0): 프리스케일러 = 1 → 8MHz 클럭 그대로 사용
    FTM0->SC = FTM_SC_PWMEN2_MASK | FTM_SC_PWMEN3_MASK | FTM_SC_PS(0);

    FTM0->MOD = 8000 - 1; // PWM 주기 설정 (8000분주 -> 1kHz 주기)
    FTM0->CNTIN = FTM_CNTIN_INIT(0); // 카운터 초기값 0

    // 채널 2: 엣지 정렬, 고전력 PWM 설정
    FTM0->CONTROLS[2].CnSC |= FTM_CnSC_MSB_MASK;
    FTM0->CONTROLS[2].CnSC |= FTM_CnSC_ELSA_MASK;

    // 채널 3: 동일하게 설정
    FTM0->CONTROLS[3].CnSC |= FTM_CnSC_MSB_MASK;
    FTM0->CONTROLS[3].CnSC |= FTM_CnSC_ELSA_MASK;

    // 외부 클럭으로 PWM 모듈 실행 (CLKS=3 → 외부 클럭 소스 사용, SIRCDIV1_CLK)
    FTM0->SC |= FTM_SC_CLKS(3);
}
... // 기존 내용 생략

void NVIC_init_IRQs(void) {
    // PORTD 인터럽트 활성화 (포트D의 IRQ 번호는 62)
    S32_NVIC->ICPR[1] |= 1 << (62 % 32); // 이전에 발생한 인터럽트 클리어
    S32_NVIC->ISER[1] |= 1 << (62 % 32); // 인터럽트 활성화
    S32_NVIC->IP[61] = 0x0B; // 오류!! IRQ62번은 IP[62]가 맞음. 우선순위 설정 (11)

    // LPIT0 채널 1 인터럽트 (게임1용 타이머. LPIT0채널1의 IRQ번호는 49)
    S32_NVIC->ICPR[1] = 1 << (49 % 32);
    S32_NVIC->ISER[1] = 1 << (49 % 32);
    S32_NVIC->IP[49] = 0x0B;

    // LPIT0 채널 2 인터럽트 (타이머 기능. LPIT0채널2의 IRQ번호는 50)
    S32_NVIC->ICPR[1] = 1 << (50 % 32);
    S32_NVIC->ISER[1] = 1 << (50 % 32);
    S32_NVIC->IP[50] = 0x0B;

    // LPIT0 채널 3 인터럽트 (게임2 타이밍. LPIT0채널1의 IRQ번호는 51)
    S32_NVIC->ICPR[1] = 1 << (51 % 32);
    S32_NVIC->ISER[1] = 1 << (51 % 32);
    S32_NVIC->IP[51] = 0x0B;
}

unsigned int i = 0; // LCD 및 SEG 출력에 사용되는 인덱스 변수
unsigned int num_n[4]; // 각 자릿수 숫자를 저장하는 배열

void LCD_print(char* first, char* second) {
    i = 0;
    lcdinput(0x80); // 첫 번째 줄 시작 주소 명령 전송. 0x80은 LCD1602의 첫 번째 줄 DDRAM 시작 주소입니다. lcdinput() 함수는 LCD 명령어를 입력하는 함수이며, 커서를 첫 줄의 시작으로 이동시킵니다.
    delay_ms(1);
    while (first[i] != '\0') {
        lcdcharinput(first[i]); // 첫 번째 줄 문자 출력
        delay_ms(1);
        i++;
    }

    i = 0;
    lcdinput(0x80 + 0x40); // 두 번째 줄 시작 주소 명령 전송
    delay_ms(1);
    while (second[i] != '\0') {
        lcdcharinput(second[i]); // 두 번째 줄 문자 출력
        delay_ms(1);
        i++;
    }
}

void Seg_out_two(int number1, int number2) {
    // 숫자 분할: number1과 number2를 각각 2자리로 나눠서 배열에 저장
    num_n[0] = (number1 / 10) % 10;
    num_n[1] = number1 % 10;
    num_n[2] = (number2 / 10) % 10;
    num_n[3] = number2 % 10;

    for (int loop = 0; loop < 20; loop++) {
        // 천의 자리 출력
        PTE->PSOR = FND_SEL[0];
        PTE->PSOR = FND_DATA[num_n[0]];
        delay_ms(1);
        PTE->PCOR = 0x1C3FE; // 모든 출력 클리어

        // 백의 자리 출력
        PTE->PSOR = FND_SEL[1];
        PTE->PSOR = FND_DATA[num_n[1]];
        delay_ms(1);
        PTE->PCOR = 0x1C3FE;

        // 가운데 콜론(:) 출력
        PTE->PSOR = 0x10000; // PTE16
        delay_ms(1);
        PTE->PCOR = 0x1C3FE;

        // 십의 자리 출력
        PTE->PSOR = FND_SEL[2];
        PTE->PSOR = FND_DATA[num_n[2]];
        delay_ms(1);
        PTE->PCOR = 0x1C3FE;

        // 일의 자리 출력
        PTE->PSOR = FND_SEL[3];
        PTE->PSOR = FND_DATA[num_n[3]];
        delay_ms(1);
        PTE->PCOR = 0x1C3FE;
    }
}

void LCD_SEG(char* first, char* second, int number) {
    // 입력 숫자를 각 자리로 나눔
    num_n[0] = (number / 1000) % 10;
    num_n[1] = (number / 100) % 10;
    num_n[2] = (number / 10) % 10;
    num_n[3] = number % 10;

    i = 0;
    lcdinput(0x80); // 첫 번째 줄 시작 주소
    delay_ms(1);
    while (first[i] != '\0') {
        lcdcharinput(first[i]); // 문자 출력
        delay_us(500); // 약간의 지연
        PTE->PSOR = FND_SEL[i % 4]; // 현재 자리 선택
        PTE->PSOR = FND_DATA[num_n[i % 4]]; // 해당 자리 숫자 출력
        delay_ms(1);
        PTE->PCOR = 0xC3FE;
        i++;
    }

    i = 0;
    lcdinput(0x80 + 0x40); // 두 번째 줄 시작 주소
    delay_ms(1);
    while (second[i] != '\0') {
        lcdcharinput(second[i]);
        delay_us(500);
        PTE->PSOR = FND_SEL[i % 4];
        PTE->PSOR = FND_DATA[num_n[i % 4]];
        delay_ms(1);
        PTE->PCOR = 0xC3FE;
        i++;
    }
}
... // 기존 내용 생략

void PORTD_IRQHandler(void) {
    // SW1 눌림 처리 (PORTD2). PORTD->ISFR & (1 << 2)는 PORTD2가 눌렸는지(인터럽트 발생했는지) 확인하는 것
    if ((PORTD->ISFR & (1 << 2)) != 0) {
        if (mode == MODE_ENTER_PASSWORD_0 || mode == MODE_CHANGE_PASSWORD_1)
            External_PIN = PSW_1; // 비밀번호 입력 1
        if (mode >= 1 && mode <= MAX_MODE_NUM)
            External_PIN = MODE_LEFT; // 왼쪽 모드 이동
        if (mode == MODE_CHANGE_PASSWORD_2)
            External_PIN = NEW_PSW_1; // 새 비밀번호 1자리
        if (mode == MODE_CHANGE_PASSWORD_3)
            External_PIN = NEW_RE_PSW_1; // 비밀번호 재입력 1자리
        if (mode == MODE_SET_TIMER_2)
            External_PIN = TIMER_SECOND;
        if (mode == MODE_SET_TIMER_4)
            External_PIN = TIMER_START;
        if (mode == MODE_PLAY_GAME1_1)
            External_PIN = AVOID_POOP_TO_EXIT;
        if (mode == MODE_PLAY_GAME1_2)
            External_PIN = AVOID_POOP_TO_START;
        if (mode == MODE_PLAY_GAME1_3)
            External_PIN = AVOID_POOP_MOVE;
        if (mode == MODE_PLAY_GAME1_4)
            External_PIN = AVOID_POOP_EXIT;
        if (mode == MODE_SET_TIMER_6)
            External_PIN = TIMER_EXIT;
        if (mode >= MODE_REMOTE_1 && mode <= MODE_REMOTE_4)
            External_PIN = FAN_STOP;
        if (mode == MODE_PLAY_GAME2_1)
            External_PIN = TIMING_TO_EXIT;
        if (mode == MODE_PLAY_GAME2_2)
            External_PIN = TIMING_TO_START;
        if (mode == MODE_PLAY_GAME2_4)
            External_PIN = TIMING_BTN_1;
        if (mode == MODE_PLAY_GAME2_5)
            External_PIN = TIMING_EXIT;
    }

    // SW2 눌림 처리 (PORTD3)
    if ((PORTD->ISFR & (1 << 3)) != 0) {
        if (mode == MODE_ENTER_PASSWORD_0 || mode == MODE_CHANGE_PASSWORD_1)
            External_PIN = PSW_2;
        if (mode >= 1 && mode <= MAX_MODE_NUM)
            External_PIN = MODE_RIGHT; // 오른쪽 모드 이동
        if (mode == MODE_CHANGE_PASSWORD_2)
            External_PIN = NEW_PSW_2;
        if (mode == MODE_CHANGE_PASSWORD_3)
            External_PIN = NEW_RE_PSW_2;
        if (mode == MODE_PLAY_GAME1_1)
            External_PIN = AVOID_POOP_TO_EXIT;
        if (mode == MODE_PLAY_GAME1_2)
            External_PIN = AVOID_POOP_TO_START;
        if (mode == MODE_PLAY_GAME1_4)
            External_PIN = AVOID_POOP_EXIT;
        if (mode == MODE_SET_TIMER_6)
            External_PIN = TIMER_EXIT;
        if (mode >= MODE_REMOTE_1 && mode <= MODE_REMOTE_4)
            External_PIN = FAN_POWER_1;
        if (mode == MODE_PLAY_GAME2_1)
            External_PIN = TIMING_TO_EXIT;
        if (mode == MODE_PLAY_GAME2_2)
            External_PIN = TIMING_TO_START;
        if (mode == MODE_PLAY_GAME2_4)
            External_PIN = TIMING_BTN_2;
        if (mode == MODE_PLAY_GAME2_5)
            External_PIN = TIMING_EXIT;
    }

    // SW3 눌림 처리 (PORTD4)
    if ((PORTD->ISFR & (1 << 4)) != 0) {
        if (mode == MODE_ENTER_PASSWORD_0 || mode == MODE_CHANGE_PASSWORD_1)
            External_PIN = PSW_3;
        if (mode >= 1 && mode <= MAX_MODE_NUM)
            External_PIN = MODE_SELECT; // 모드 선택
        if (mode == MODE_CHANGE_PASSWORD_2)
            External_PIN = NEW_PSW_3;
        if (mode == MODE_CHANGE_PASSWORD_3)
            External_PIN = NEW_RE_PSW_3;
        if (mode == MODE_PLAY_GAME1_1)
            External_PIN = AVOID_POOP_START;
        if (mode == MODE_PLAY_GAME1_2 || mode == MODE_PLAY_GAME1_4)
            External_PIN = AVOID_POOP_EXIT;
        if (mode == MODE_SET_TIMER_6)
            External_PIN = TIMER_EXIT;
        if (mode >= MODE_REMOTE_1 && mode <= MODE_REMOTE_4)
            External_PIN = FAN_POWER_2;
        if (mode == MODE_PLAY_GAME2_1)
            External_PIN = TIMING_START;
        if (mode == MODE_PLAY_GAME2_2)
            External_PIN = TIMING_EXIT;
        if (mode == MODE_PLAY_GAME2_4)
            External_PIN = TIMING_BTN_3;
        if (mode == MODE_PLAY_GAME2_5)
            External_PIN = TIMING_EXIT;
    }

    // SW4 눌림 처리 (PORTD5)
    if ((PORTD->ISFR & (1 << 5)) != 0) {
        if (mode == MODE_ENTER_PASSWORD_0 || mode == MODE_CHANGE_PASSWORD_1)
            External_PIN = PSW_4;
        if (mode == MODE_CHANGE_PASSWORD_2)
            External_PIN = NEW_PSW_4;
        if (mode == MODE_CHANGE_PASSWORD_3)
            External_PIN = NEW_RE_PSW_4;
        if (mode == MODE_PLAY_GAME1_4)
            External_PIN = AVOID_POOP_EXIT;
        if (mode == MODE_SET_TIMER_6)
            External_PIN = TIMER_EXIT;
        if (mode >= MODE_REMOTE_1 && mode <= MODE_REMOTE_4)
            External_PIN = FAN_POWER_3;
        if (mode == MODE_PLAY_GAME2_4)
            External_PIN = TIMING_BTN_4;
        if (mode == MODE_PLAY_GAME2_5)
            External_PIN = TIMING_EXIT;
    }

    // SW5 눌림 처리 (PORTD8)
    if ((PORTD->ISFR & (1 << 8)) != 0) {
        if (mode == MODE_ENTER_PASSWORD_0 || mode == MODE_CHANGE_PASSWORD_1)
            External_PIN = PSW_CLEAR;
        if (mode == MODE_CHANGE_PASSWORD_2)
            External_PIN = NEW_PSW_CLEAR;
        if (mode == MODE_CHANGE_PASSWORD_3)
            External_PIN = NEW_RE_PSW_CLEAR;
        if (mode == MODE_PLAY_GAME1_3 || mode == MODE_PLAY_GAME1_4)
            External_PIN = AVOID_POOP_EXIT;
        if (mode >= MODE_SET_TIMER_1 && mode <= MODE_SET_TIMER_6)
            External_PIN = TIMER_EXIT;
        if (mode >= MODE_REMOTE_1 && mode <= MODE_REMOTE_4)
            External_PIN = FAN_EXIT;
        if (mode == MODE_PLAY_GAME2_4 || mode == MODE_PLAY_GAME2_5)
            External_PIN = TIMING_EXIT;
    }
}
... // 기존 내용 생략

switch (External_PIN) {
    // 인터럽트 상태 플래그 클리어 (SW1~SW5) 오류!!PORTD->ISFR |= (1 << 2); 또는 PORTD->PCR[2] |= (1 << 24);로 써줘야 함
    PORTD->PCR[2] &= ~(0x01000000); // ISF 비트 클리어 (SW1)
    PORTD->PCR[3] &= ~(0x01000000); // SW2
    PORTD->PCR[4] &= ~(0x01000000); // SW3
    PORTD->PCR[5] &= ~(0x01000000); // SW4
    PORTD->PCR[8] &= ~(0x01000000); // SW5

    // 비밀번호 입력 처리
    case PSW_0: case PSW_1: case PSW_2: case PSW_3: case PSW_4:
    case PSW_5: case PSW_6: case PSW_7: case PSW_8: case PSW_9:
        msg_mode0_psw[psw_num++] = External_PIN + '0';//strcmp()는 문자열 비교함수이기 때문에 msg_mode0_psw는 문자열로 만들어 줘야함->'0'는 문자이고 ASCII코드48이다 ex)3+'0'=51='3'
        if (psw_num >= 11) { // 6자리 입력 완료
            if (strcmp(msg_mode0_psw, msg_mode0_psw_answer) == 0) // 정답 비교
                mode++; // 잠금 해제
            strcpy(msg_mode0_psw, "     ******     "); // 초기화
            psw_num = 5;//비밀번호 입력하는 시작위치
        }
        break;

    case PSW_CLEAR:
        strcpy(msg_mode0_psw, "     ******     "); // 클리어
        psw_num = 5;
        break;

    // 새로운 비밀번호 설정 입력 처리
    case NEW_PSW_0: case NEW_PSW_1: case NEW_PSW_2: case NEW_PSW_3:
    case NEW_PSW_4: case NEW_PSW_5: case NEW_PSW_6: case NEW_PSW_7:
    case NEW_PSW_8: case NEW_PSW_9:
        msg_mode1_psw_new[psw_num++] = External_PIN - NEW_PSW_0 + '0';
        if (psw_num >= 11)
            mode++;
        break;

    case NEW_PSW_CLEAR:
        strcpy(msg_mode1_psw_new, "     ******     ");
        psw_num = 5;
        break;

    // 비밀번호 재입력 확인
    case NEW_RE_PSW_0: case NEW_RE_PSW_1: case NEW_RE_PSW_2:
    case NEW_RE_PSW_3: case NEW_RE_PSW_4: case NEW_RE_PSW_5:
    case NEW_RE_PSW_6: case NEW_RE_PSW_7: case NEW_RE_PSW_8:
    case NEW_RE_PSW_9:
        msg_mode1_psw_new_re[psw_num++] = External_PIN - NEW_RE_PSW_0 + '0';
        if (psw_num >= 11) {
            if (strcmp(msg_mode1_psw_new_re, msg_mode1_psw_new) == 0) {
                strcpy(msg_mode0_psw_answer, msg_mode1_psw_new);
                strcpy(msg_mode1_psw_new, "     ******     ");
                mode = MODE_CHANGE_PASSWORD_0;
            }
            strcpy(msg_mode1_psw_new_re, "     ******     ");
            psw_num = 5;
        }
        break;

    case NEW_RE_PSW_CLEAR:
        strcpy(msg_mode1_psw_new_re, "     ******     ");
        psw_num = 5;
        break;

    // 타이머 설정 처리
    case TIMER_SECOND:
        mode = MODE_SET_TIMER_3;
        timer_min = read_adc_chx(3);
        break;

    case TIMER_START:
        LCD_print("Timer is Running", "     (^3^)7     ");
        mode = MODE_SET_TIMER_5;
        timer_sec = read_adc_chx(3);
        LPIT0->MIER = 0x04; // 채널 2 인터럽트 활성화
        break;

    case TIMER_EXIT:
        timer_min = 0;
        timer_sec = 0;
        mode = MODE_SET_TIMER_0;
        LPIT0->MIER = 0x00;
        PTC->PCOR |= 1 << 0; // 부저 끔
        break;

    // 피하기 게임 상태 전환
    case AVOID_POOP_TO_START:
        mode = MODE_PLAY_GAME1_1;
        break;

    case AVOID_POOP_TO_EXIT:
        mode = MODE_PLAY_GAME1_2;
        break;

    case AVOID_POOP_START:
        mode = MODE_PLAY_GAME1_3;
        LPIT0->MIER = 0x02;//채널1의 인터럽트 enable
        break;

    case AVOID_POOP_EXIT:
        mode = 4;
        seg_num = 0;
        LPIT0->MIER = 0x00;
        strcpy(avoid_poop_row0, "                ");//LCD한 행을 비움(16열)
        strcpy(avoid_poop_row1, "                ");
        break;

    case AVOID_POOP_MOVE:
        avoid_poop_player ^= 1; // 줄 전환
        if (((avoid_poop_row0[15] == '*') && (avoid_poop_player == 0)) ||
            ((avoid_poop_row1[15] == '*') && (avoid_poop_player == 1))) {
            mode = MODE_PLAY_GAME1_4;
            LPIT0->MIER = 0x00;
            avoid_poop_player = 0;
            strcpy(avoid_poop_row0, "               O");
            strcpy(avoid_poop_row1, "                ");
        } else {
            if (avoid_poop_player == 0) {
                avoid_poop_row0[15] = 'O';
                avoid_poop_row1[15] = ' ';
            } else {
                avoid_poop_row0[15] = ' ';
                avoid_poop_row1[15] = 'O';
            }
        }
        break;
... // 기존 내용 생략

    // 타이밍 게임 상태 전환
    case TIMING_TO_START:
        mode = MODE_PLAY_GAME2_1; // Start 화면으로 이동
        break;

    case TIMING_TO_EXIT:
        mode = MODE_PLAY_GAME2_2; // Exit 화면으로 이동
        break;

    case TIMING_START:
        mode = MODE_PLAY_GAME2_3; // 게임 시작
        break;

    case TIMING_EXIT:
        LED_life = 3; // 생명 초기화
        LPIT0->MIER = 0x00; // 타이머 인터럽트 비활성화
        seg_num = 0; // 성공 횟수 초기화
        record[LED_num] = 0; // 해당 LED 기록 초기화
        LED_num = 0; // LED 인덱스 초기화
        PTC->PSOR |= 0x39E; // 모든 LED 끄기 (active low)  PTC1,2,3,4,7,8,9
        mode = MODE_PLAY_GAME2_0; // 타이밍 게임 메인 화면으로
        break;

    case TIMING_BTN_1:
        if (record[0] == 1) {
            PTC->PSOR = 1 << 1; // LED1 끄기. SW1눌리면 record[0]=1됨.
            record[0] = 0;
            seg_num++; // 성공 횟수 증가
        }
        break;

    case TIMING_BTN_2:
        if (record[1] == 1) {
            PTC->PSOR = 1 << 2; // LED2 끄기
            record[1] = 0;
            seg_num++;
        }
        break;

    case TIMING_BTN_3:
        if (record[2] == 1) {
            PTC->PSOR = 1 << 3; // LED3 끄기
            record[2] = 0;
            seg_num++;
        }
        break;

    case TIMING_BTN_4:
        if (record[3] == 1) {
            PTC->PSOR = 1 << 4; // LED4 끄기
            record[3] = 0;
            seg_num++;
        }
        break;

    // 선풍기 제어 모드 전환
    case FAN_STOP:
        mode = MODE_REMOTE_1; // 팬 정지 모드로
        break;

    case FAN_POWER_1:
        mode = MODE_REMOTE_2; // 팬 파워1
        break;

    case FAN_POWER_2:
        mode = MODE_REMOTE_3; // 팬 파워2
        break;

    case FAN_POWER_3:
        mode = MODE_REMOTE_4; // 팬 파워3
        break;

    case FAN_EXIT:
        // PWM 설정 해제하여 팬 정지
        FTM0->COMBINE &= ~(FTM_COMBINE_SYNCEN1_MASK | FTM_COMBINE_COMP1_MASK | FTM_COMBINE_DTEN1_MASK);
        FTM0->CONTROLS[2].CnV = FTM_CnV_VAL(0);
        FTM0->CONTROLS[3].CnV = FTM_CnV_VAL(0);
        mode = MODE_REMOTE_0; // 리모트 팬 메인 모드로 복귀
        break;
... // 기존 내용 생략

    ... // 이전 case들 생략

    // 모드 선택 이동 처리
    case MODE_LEFT:
        if (mode == 1)
            mode = MAX_MODE_NUM; // 처음이면 마지막 모드로 이동
        else
            mode--; // 이전 모드로 이동
        break;

    case MODE_RIGHT:
        if (mode == MAX_MODE_NUM)
            mode = 1; // 마지막이면 처음 모드로 이동
        else
            mode++; // 다음 모드로 이동
        break;

    case MODE_SELECT:
        mode *= 10; // 서브 모드 진입 (ex: 2 -> 20)
        break;

    case MODE_HOME:
        mode = MODE_LOCK_PHONE_0; // 홈 버튼 → 잠금 모드로 이동
        break;

    default:
        break; // 정의되지 않은 입력 무시
}

External_PIN = -1; // 입력 리셋

// 인터럽트 상태 플래그 다시 설정 (포트 D)->인터럽트 플래그 클리어(인터럽트 다시 사용 가능)
PORTD->PCR[2] |= 0x01000000;
PORTD->PCR[3] |= 0x01000000;
PORTD->PCR[4] |= 0x01000000;
PORTD->PCR[5] |= 0x01000000;
PORTD->PCR[8] |= 0x01000000;
}

// 피하기 게임 타이머 인터럽트 (CH1, 일정 시간마다 장애물 이동)
void LPIT0_Ch1_IRQHandler(void)
{
    LPIT0->MSR |= LPIT_MSR_TIF1_MASK; // 인터럽트 플래그 클리어
    lpit0_ch1_flag_counter++; // 인터럽트 발생 횟수 카운트

    // 오른쪽으로 한 칸씩 이동 (맨 앞 제거, 뒤로 밀기)
    for (int loop = 15; loop > 0; loop--) {
        avoid_poop_row0[loop] = avoid_poop_row0[loop - 1];
        avoid_poop_row1[loop] = avoid_poop_row1[loop - 1];
    }

    // 새 칸 초기화
    avoid_poop_row0[0] = ' ';
    avoid_poop_row1[0] = ' ';

    // 랜덤 위치에 장애물 생성 (같은 위치 중복 방지)
    int num = rand() % 5;
    if ((num == 0) && (avoid_poop_row0[1] != '*'))//0,1,2,3,4중 0,1나올 때 *나오고, row0[0]=*&&row1[1]=* or row0[1]=*&&row1[0]=*안나오게 설정
        avoid_poop_row1[0] = '*';
    if ((num == 1) && (avoid_poop_row1[1] != '*'))
        avoid_poop_row0[0] = '*';

    // 충돌 판정
    if (((avoid_poop_row0[15] == '*') && (avoid_poop_player == 0)) ||
        ((avoid_poop_row1[15] == '*') && (avoid_poop_player == 1))) {
        mode = MODE_PLAY_GAME1_4; // 게임 종료 모드
        LPIT0->MIER = 0x00; // 인터럽트 비활성화
        avoid_poop_player = 0; // 플레이어 위치 초기화
        strcpy(avoid_poop_row0, "               O");
        strcpy(avoid_poop_row1, "                ");
    } else {
        // 생존 → 점수 증가 및 위치 갱신
        seg_num++;
        if (avoid_poop_player == 0)
            avoid_poop_row0[15] = 'O';
        else
            avoid_poop_row1[15] = 'O';
    }
}

// 타이머 게임 인터럽트 (CH2, 1초 간격 감소)
void LPIT0_Ch2_IRQHandler(void)
{
    LPIT0->MSR |= LPIT_MSR_TIF2_MASK; // 인터럽트 플래그 클리어
    lpit0_ch2_flag_counter++;

    // 초 감소 처리
    if (timer_sec == 0) {
        timer_min--;
        timer_sec = 59;
    } else {
        timer_sec--;
    }

    // 시간이 다 되었을 경우
    if ((timer_min == 0) && (timer_sec == 0)) {
        timer_min = 0;
        timer_sec = 0;
        mode = MODE_SET_TIMER_6; // 타이머 종료 모드 진입
        LPIT0->MIER = 0x00; // 타이머 인터럽트 정지
        PTC->PSOR |= 1 << 0; // 부저 ON (active high) PTC0->buzzor
    }
}

// 타이밍 게임 인터럽트 (CH3, LED on/off + 실패 체크)
void LPIT0_Ch3_IRQHandler(void)
{
    LPIT0->MSR |= LPIT_MSR_TIF3_MASK; // 인터럽트 플래그 클리어
    lpit0_ch3_flag_counter++;

    if (record[LED_num] == 1) { // 실패 (제때 누르지 못함)
        record[LED_num] = 0;
        PTC->PSOR |= 1 << (LED_num + 1); // 해당 LED 끄기, PTC1~PTC4까지이므로 0~3 + 1
        LED_life--; // 생명 차감

        // 생명 2, 1일 때 표시용 LED 끄기
        if (LED_life == 2) PTC->PSOR |= 1 << 7;
        if (LED_life == 1) PTC->PSOR |= 1 << 8;

        if (LED_life == 0) {
            LED_life = 3; // 초기화
            PTC->PSOR |= 0x39E; // 모든 LED 끄기
            mode = MODE_PLAY_GAME2_5; // 실패로 탈출
            LPIT0->MIER = 0x00; // 인터럽트 정지
        }
    } else {
        // 새로운 랜덤 LED 점등 (성공)
        LED_num = rand() % 4;
        record[LED_num] = 1;
        PTC->PCOR |= 1 << (LED_num + 1); // 해당 LED ON (active low)
    }
}
// 메인 함수: 전체 시스템 초기화 및 모드 실행 루프
int main(void)
{
    WDOG_disable(); // 워치독 타이머 비활성화 (시스템 자동 리셋 방지)
    SOSC_init_8MHz(); // 외부 Slow OSC를 8MHz로 초기화
    SPLL_init_160MHz(); // SPLL(System PLL) 설정으로 160MHz 생성
    NormalRUNmode_80MHz(); // 시스템 클럭을 80MHz로 설정
    SystemCoreClockUpdate(); // CMSIS 변수 SystemCoreClock을 갱신
    delay_ms(20); // 초기화 후 안정화 대기

    PORT_init(); // 포트 초기화 (입출력 핀 설정)
    FTM_init(); // FTM(PWM 제어용 타이머) 초기화
    NVIC_init_IRQs(); // 인터럽트 설정
    ADC_init(); // ADC 초기화
    lcdinit(); // LCD 초기화
    delay_ms(200); // LCD 초기화 안정화 대기

    uint32_t adcResultInMv = 0; // ADC 결과 저장용 변수 (단위: mV)

    // 무한 루프 실행
    while (1) {
        switch (mode) {
        // ----------- 메인 모드 -----------
        case MODE_ENTER_PASSWORD_0:
            LCD_print(" Enter Password ", msg_mode0_psw);
            break;
        case MODE_LOCK_PHONE_0:
            LCD_print("<    MODE 1    >", "   Lock Phone   ");
            break;
        case MODE_CHANGE_PASSWORD_0:
            LCD_print("<    MODE 2    >", "Change  Password");
            break;
        case MODE_SET_TIMER_0:
            LCD_print("<    MODE 3    >", "   Set  Timer   ");
            break;
        case MODE_PLAY_GAME1_0:
            LCD_print("<    MODE 4    >", "  Play  GAME 1  ");
            break;
        case MODE_PLAY_GAME2_0:
            LCD_print("<    MODE 5    >", "  Play  GAME 2  ");
            break;
        case MODE_REMOTE_0:
            LCD_print("<    MODE 6    >", "   Remote Fan   ");
            break;

        // ----------- 비밀번호 변경 관련 서브 모드 -----------
        case MODE_LOCK_PHONE_1:
            mode = MODE_ENTER_PASSWORD_0; // 잠금모드 종료 → 다시 입력으로
            break;
        case MODE_CHANGE_PASSWORD_1:
            LCD_print("Current Password", msg_mode0_psw);
            break;
        case MODE_CHANGE_PASSWORD_2:
            LCD_print("  New Password  ", msg_mode1_psw_new);
            break;
        case MODE_CHANGE_PASSWORD_3:
            LCD_print("Confirm  Password", msg_mode1_psw_new_re);
            break;

        // ----------- 타이머 설정 모드 -----------
        case MODE_SET_TIMER_1:
            LCD_print("   Set Minute   ", "    Move ADC    ");
            timer_min = 0;
            mode = MODE_SET_TIMER_2;
            break;
        case MODE_SET_TIMER_2:
            convertAdcChan(13);
            while (adc_complete() == 0) {}
            adcResultInMv = read_adc_chx(3);//오류!!!timer_min = (read_adc_chx() * 60) / 5000;해야 끝까지 돌려야 60설정가능
            Seg_out_two(adcResultInMv, 0); //Seg_out_two(timer_min, 0); 
            break;
        case MODE_SET_TIMER_3:
            LCD_print("   Set Second   ", "    Move ADC    ");
            timer_sec = 0;
            mode = MODE_SET_TIMER_4;
            break;
        case MODE_SET_TIMER_4:
            convertAdcChan(13);
            while (adc_complete() == 0) {}
            adcResultInMv = read_adc_chx(3);//오류!!!timer_sec = (read_adc_chx() * 60) / 5000;
            Seg_out_two(timer_min, adcResultInMv); //Seg_out_two(timer_min, timer_sec); 
            break;
        case MODE_SET_TIMER_5:
            Seg_out_two(timer_min, timer_sec); // 타이머 진행 중
            break;
        case MODE_SET_TIMER_6:
            LCD_print("  TIME is OVER! ", "PRESS ANY KEY.."); // 타임업
            break;

        // ----------- 게임 1: 피하기 -----------
        case MODE_PLAY_GAME1_1:
            LCD_print("   Avoid Poop   ", ">Start<    Exit ");
            break;
        case MODE_PLAY_GAME1_2:
            LCD_print("   Avoid Poop   ", " Start    >Exit<");
            break;
        case MODE_PLAY_GAME1_3:
            LCD_SEG(avoid_poop_row0, avoid_poop_row1, seg_num);
            break;
        case MODE_PLAY_GAME1_4:
            LCD_SEG(" PRESS ANY KEY! ", "          SCORE:", seg_num);
            break;

        // ----------- 게임 2: 타이밍 LED -----------
        case MODE_PLAY_GAME2_1:
            LCD_print("  Timing  Game  ", ">Start<    Exit ");
            break;
        case MODE_PLAY_GAME2_2:
            LCD_print("  Timing  Game  ", " Start    >Exit<");
            break;
        case MODE_PLAY_GAME2_3:
            PTC->PCOR |= 0x380; //오류!!!LED 초기화 (LED5~8 OFF) PTC7, PTC8, PTC9가 0이 되므로 LED5,6,7은 켜짐 PTC->PSOR |= 0x380;라고 써야 LED5,6,7이 꺼지는 것(초기화)
            LCD_print("   Timng Game   ", "Turn off the LED");
            mode = MODE_PLAY_GAME2_4;
            LPIT0->MIER = 0x08; // CH3 인터럽트 허용
            break;
        case MODE_PLAY_GAME2_4:
            Seg_out_two(0, seg_num); // 성공 횟수 표시
            break;
        case MODE_PLAY_GAME2_5:
            LCD_SEG(" PRESS ANY KEY! ", "          SCORE:", seg_num);
            break;

        // ----------- 팬 리모컨 제어 -----------
        case MODE_REMOTE_1:
            LCD_print("   PRESS  KEY   ", " To Remote Fan! ");
            FTM0->COMBINE &= ~(FTM_COMBINE_SYNCEN1_MASK | FTM_COMBINE_COMP1_MASK | FTM_COMBINE_DTEN1_MASK);
            FTM0->CONTROLS[2].CnV = FTM_CnV_VAL(0);
            FTM0->CONTROLS[3].CnV = FTM_CnV_VAL(0);
            break;
        case MODE_REMOTE_2:
            FTM0->COMBINE |= FTM_COMBINE_SYNCEN1_MASK | FTM_COMBINE_COMP1_MASK | FTM_COMBINE_DTEN1_MASK;
            FTM0->CONTROLS[2].CnV = FTM_CnV_VAL(5300);
            FTM0->CONTROLS[3].CnV = FTM_CnV_VAL(5300);
            FTM0->SC |= FTM_SC_CLKS(3);
            break;
        case MODE_REMOTE_3:
            FTM0->COMBINE |= FTM_COMBINE_SYNCEN1_MASK | FTM_COMBINE_COMP1_MASK | FTM_COMBINE_DTEN1_MASK;
            FTM0->CONTROLS[2].CnV = FTM_CnV_VAL(6600);
            FTM0->CONTROLS[3].CnV = FTM_CnV_VAL(6600);
            FTM0->SC |= FTM_SC_CLKS(3);
            break;
        case MODE_REMOTE_4:
            FTM0->COMBINE |= FTM_COMBINE_SYNCEN1_MASK | FTM_COMBINE_COMP1_MASK | FTM_COMBINE_DTEN1_MASK;
            FTM0->CONTROLS[2].CnV = FTM_CnV_VAL(8000);
            FTM0->CONTROLS[3].CnV = FTM_CnV_VAL(8000);
            FTM0->SC |= FTM_SC_CLKS(3);
            break;

        default:
            LCD_SEG(" ERROR  OCCURED ", "           MODE:", mode);
            break;
        }
    }
    return 0; // 도달하지 않음
}


