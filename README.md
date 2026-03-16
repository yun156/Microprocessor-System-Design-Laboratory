# :art: Microprocessor-System-Design-Laboratory
마이크로프로세서 설계 실험 과목 프로젝트
# :rocket: 설계 목표

사용자에게 여러 기능을 지원하는 모바일 스마트 기기를 구현한다.
- 비밀번호를 이용한 잠금 및 비밀번호 변경 기능
- 타이머 및 버저를 이용한 알람 기능
- 오락을 제공하기 위한 일부 게임들
- 모바일 기기를 통한 간편한 선풍기 제어

# :memo: 개발일정 
- 1주차 : 주제 선정 및 기능 고안
- 2, 3주차 : 알고리즘 구현 및 보드 작동 점검
- 4주차 : 최종 점검 및 보고서 작성

# :tada: 구성원 및 역할 분담
| 이름   | 수행 역할                                    |
| ---------- | ---------------------------------------------- |
| 윤승환 | 설계 기획, 잠금화면, 모드 선택 메뉴, 비밀번호 변경, 게임1 구현, 보고서 제작 |
| 이동인 | 타이머, 게임2, 선풍기 제어 구현, 보고서 제작|

# :memo: 프로젝트 세부기능설명
<img width="242" height="147" alt="Image" src="https://github.com/user-attachments/assets/9d82c4c3-3676-44cc-bdf8-9971c05466fb" />

- 잠금화면

인증: 사용자의 모바일 기기 보안을 위해 비밀번호를 통한 인증 단계가 필요하다.

잠금: 사용자가 원할 때 모바일 기기를 잠글 수 있다.

<img width="242" height="147" alt="Image" src="https://github.com/user-attachments/assets/1410bd6e-e04f-4c64-9b22-0497eedf3eb7" />

- PW변경

맞춤: 사용자의 편의를 위해서 비밀번호 변경이 가능하다. 

재인증: 비밀번호 변경 전에는 재인증을 통해 사용자라는 것을 확인한다. 변경한 비밀번호 또한 재확인이 필요하다.

<img width="667" height="218" alt="Image" src="https://github.com/user-attachments/assets/f62ba5d0-e44f-44fa-b8ff-ec444ac6da14" />

- 타이머

설정: 타이머 동작을 위한 시간과 시작 시점은 사용자가 직접 설정할 수 있다.

알람: 타이머의 시간이 완료되면 소리를 울려 사용자에게 알릴 수 있다.

<img width="667" height="218" alt="Image" src="https://github.com/user-attachments/assets/cf0dbff0-5dfa-4a01-a77b-1d56eccafab0" />

- 피하기 게임

플레이: 무작위로 장애물이 등장하고 플레이어 방향으로 다가온다. 사용자는 플레이어의 위치를 변경할 수 있다.
점수: 게임 진행의 진척도를 확인할 수 있다.

<img width="667" height="218" alt="Image" src="https://github.com/user-attachments/assets/38cd836f-dde2-419e-8e96-e50637f2deb3" />

- 타이밍 게임

플레이: 무작위로 LED가 켜지게 되고 짧은 제한 시간 내에 일치하는 버튼을 눌러 LED를 꺼야 한다.
점수: 게임 진행의 진척도를 확인할 수 있다.

<img width="317" height="318" alt="Image" src="https://github.com/user-attachments/assets/3896a4b5-e8f0-42e5-ba57-50487c9476c7" />

- 선풍기 제어

제어: 모터를 선풍기로 가정하고, 모바일 기기를 통해 작동시키는 것을 구현한다.
기호: 선풍기의 세기는 사용자가 원하는 대로 설정할 수 있다.

# :zap: 하드웨어 구성 및 회로도
<img width="1023" height="398" alt="Image" src="https://github.com/user-attachments/assets/0f7a70eb-90f2-435c-8d44-a67745b28040" />

<img width="927" height="307" alt="Image" src="https://github.com/user-attachments/assets/fd796cdb-abce-46c0-a912-02fd278d7622" />

<img width="1065" height="392" alt="Image" src="https://github.com/user-attachments/assets/2f431fb8-7638-46aa-ade5-0bf67b49ff9e" />

# :white_check_mark: 소프트웨어 구성 및 Flow chart

- 기본 구성

main 코드
```
#define MODE_CHANGE_PASSWORD 20
#define MODE_SET_TIMER 30
...

int mode;
...

While (1) {
	switch (mode) {
	      case MODE_CHANGE_PASSWORD : …
	      case MODE_SET_TIMER : …
 	      //LCD 출력 및 7-Segment 출력
	}
}
```

interrupt 코드
```
#define FAN_STOP 70
#define FAN_POWER_1 71
#define FAN_POWER_2 72
#define FAN_POWER_3 73
…

if ((PORTD->ISFR & (1<<2)) != 0) {
	if (mode == …)
		 External_PIN = FAN_STOP;
	if (mode == …)
		 External_PIN = FAN_POWER_1;
}
…

switch (External_PIN) {
	case FAN_STOP : …
 	case FAN_POWER_1 : …
 	//mode 변경, LPIT 채널 활성화 등
}

```
<img width="1007" height="470" alt="Image" src="https://github.com/user-attachments/assets/379ee151-11ac-44a3-9e23-38f312d8c5e0" />

<img width="1008" height="420" alt="Image" src="https://github.com/user-attachments/assets/2274988d-4f11-4e03-a03f-1fbc96919bc9" />

<img width="987" height="387" alt="Image" src="https://github.com/user-attachments/assets/33c85581-f2f5-48a6-b3b7-80f8d80d7241" />

<img width="1027" height="515" alt="Image" src="https://github.com/user-attachments/assets/b35295f9-fb93-4ffd-b2df-9eba931856e1" />

# :recycle: 설계결과 및 논의

설계결과
- 잠금을 통해 모바일 기기의 보안성을 높일 수 있다.
- 타이머를 통해 원하는 시간을 잴 수 있다.
- 게임을 통해 사용자에게 오락을 제공한다.
- 선풍기 제어를 통해 사용자에게 편의성을 제공한다.
- 한정된 버튼을 활용하여 여러 기능들을 구현하였다.
- main에서는 LCD 출력, 7-Segment 출력, 모터 동작 등의 반복전인 수행만을 하도록 간편화.  
- interrupt에서는 그 외 모든 기능들을 수행하도록 설계.
- 또한, integer 변수(mode)에 따른 다른 동작을 수행하도록 설계.

## macro(#define)와 switch-case문을 이용한 체계적인 구조로 코드의 가독성이 높아지고, 이 후 기능 추가가 용이하다.

논의	
- main에서는 LCD 화면을 반복적으로 갱신. 이는 전력소모로 이어질 수 있다.
→ 변경사항이 존재할 때만 화면을 갱신하도록 재설계한다

- 타이머, 게임1, 게임2는 모두 다른 LPIT 채널을 사용. 여러 기능이 추가되면 채널 부족으로 이어질 수 있다.
→ interrupt와 마찬가지로 하나의 채널이 mode에 따라 다르게 동작하도록 재설계한다.

# :bug: 시연 동영상

YOUTUBE 영상 주소: https://youtu.be/iO8tHOxSTE0

