org	0x9000  ;메모리 어디에서 프로그램을 시작할 것인지 지정. 기계어코드로 번역한 boot1.bin이 적재되어 이곳에서 변수나 함수를 접근

[BITS 16]  	;어셈블러가 16bit모드(real mode)에서 동작하도록 실행 코드를 생성해야 함을 명시

		cli		; Clear Interrupt Flag

		mov     ax, 0xb800 	;비디오 메모리의 시작주소는 0xb800
        mov     es, ax			;es레지스터에 ax의 세그먼트를 넣는다
        mov     ax, 0x00
        mov     bx, 0
        mov     cx, 80*25*2 
CLS:				;loop 는 cx의 값을 감소시키고 cx가 0보다 크면 레이블로 이동한다. cx의 값을 감소시키면서 CLS로 이동하며 반복수행을 하게 된다.
				;반복하며 비디오 메모리(화면)을 0으로 채움(화면 지우기)
        mov     [es:bx], ax
        add     bx, 1
        loop    CLS 
 
Initialize_PIC:
		;ICW1 - 두 개의 PIC를 초기화 
		mov		al, 0x11
		out		0x20, al
		out		0xa0, al

		;ICW2 - 발생된 인터럽트 번호에 얼마를 더할지 결정
		mov		al, 0x20
		out		0x21, al
		mov		al, 0x28
		out		0xa1, al

		;ICW3 - 마스터/슬레이브 연결 핀 정보 전달
		mov		al, 0x04
		out		0x21, al
		mov		al, 0x02
		out		0xa1, al

		;ICW4 - 기타 옵션 
		mov		al, 0x01
		out		0x21, al
		out		0xa1, al

		mov		al, 0xFF
		;out		0x21, al
		out		0xa1, al

Initialize_Serial_port:
		xor		ax, ax	;ax, dx를 0으로 초기화 (xor가 mov보다 빠름)
		xor		dx, dx
		mov		al, 0xe3
		int		0x14	;int 14h 로 시리얼 포트 초기화

READY_TO_PRINT:
		xor		si, si	;초기화
		xor		bh, bh
PRINT_TO_SERIAL:
		mov		al, [msgRMode+si]	;int 14h, 01h,는 문자를 전송한다. al에는 전송할 바이트 단위가 들어간다. al에 msgRMode+si가 가지는 값을 복사한다. 바이트 단위로 전송을 하게 되며 al에 들어있는 값이 없을 때까지 함수를 반복한다.
		mov		ah, 0x01
		int		0x14
		add		si, 1
		cmp		al, 0
		jne		PRINT_TO_SERIAL
PRINT_NEW_LINE:					;int 14h 인터럽트는 시리얼포트로 문자를 전송한다.
		mov		al, 0x0a		;0x0a는 line feed(현재 라인에서 다음 줄로 이동)를 하고
		mov		ah, 0x01
		int		0x14
		mov		al, 0x0d	;0x0d는 carriage return(현재위치를 나타내는 커서를 맨 앞으로 이동)을 하게 된다. 두 개가 같이 이루어져 다음 줄로 이동하고 맨 앞으로 이동하는 개행의 역할을 수행한다.
		mov		ah, 0x01
		int		0x14

; OS assignment 2
; add your code here
; print current date to boch display




Activate_A20Gate:
		mov		ax,	0x2401
		int		0x15

;Detecting_Memory:
;		mov		ax, 0xe801
;		int		0x15

PROTECTED:
        xor		ax, ax		;ax를 초기화하고 SET_GDT 함수를 호출한다. 보호모드 진입 준비를 하는 단계이다.
        mov		ds, ax              

		call	SETUP_GDT

        mov		eax, cr0  	;cr0 레지스터 : 프로세서의 상태와 동작모드를 제어하는 여러가지 제어 flag를 가지고 있다. (PE 가 1이면 보호모드 진입, 0이면 리얼모드)
        or		eax, 1	  	;eax 와 1을 or연산
        mov		cr0, eax  	;그 값을 cr0에 넣으면 cr0레지스터의 PE는 1을 갖게 되어 보호모드 진입을 설정한다.

		jmp		$+2	;32비트 모드로 바뀌면서 16비트 코드를 실행하면 CPU가 에러를 발생시키는데 아직 파이프라인에 16비트 명령어가 남아았을 수 있으므로 nop(아무작업안함)명령어로 이를 방지한다.(nop는 16, 32비트나 차이가 없다)
		nop
		nop
		jmp		CODEDESCRIPTOR:ENTRY32		;그리고 32비트 커널부분으로 점프한다.

SETUP_GDT:				;GDT_DESC 자료구조를 프로세서에 설정하여 GDT 테이블을 로드한다.
		lgdt	[GDT_DESC]
		ret

[BITS 32]  		;32bit에서 동작하는 코드 생성

ENTRY32:		;보호 모드 커널용 데이터 세그먼트 디스크립터를  ax레지스터에 저장
		mov		ax, 0x10	;세그먼트 셀렉터들이 모두 0x10을 가리키도록 설정(초기화)
		mov		ds, ax
		mov		es, ax
		mov		fs, ax
		mov		gs, ax

		mov		ss, ax
  		mov		esp, 0xFFFE	;esp, ebp 레지스터의 어드레스를 0xFFFE로 설정
		mov		ebp, 0xFFFE	

		mov		edi, 80*2	;edi에 80*2의 값을 대입
		lea		esi, [msgPMode]		;msgPMode 의 주소를 esi에 대입
		call	PRINT			;PRINT함수 호출

		;IDT TABLE
	    cld
		mov		ax,	IDTDESCRIPTOR
		mov		es, ax
		xor		eax, eax
		xor		ecx, ecx
		mov		ax, 256
		mov		edi, 0
 
IDT_LOOP:
		lea		esi, [IDT_IGNORE]
		mov		cx, 8
		rep		movsb
		dec		ax
		jnz		IDT_LOOP

		lidt	[IDTR]

		sti
		jmp	CODEDESCRIPTOR:0x10000

PRINT:
		push	eax		;eax, ebx, edx, es의 값을 스택에 저장
		push	ebx
		push	edx
		push	es
		mov		ax, VIDEODESCRIPTOR	;es 레지스터에 비디오 디스크립터를 저장한다.
		mov		es, ax
PRINT_LOOP:
		or		al, al		;al이 비었으면 PRINT_END로 jump를 하고 아니면 al에 문자 하나를 가져오고 문자를 하나 출력한다.
		jz		PRINT_END
		mov		al, byte[esi]
		mov		byte [es:edi], al
		inc		edi		;edi를 증가시켜 글자색을 위한 준비를 하고
		mov		byte [es:edi], 0x07	;0x07 글자속성을 준다. 비디오메모리는 (문자, 속성)의 상으로 구성되어 있어서 문자를 넣고 속성을 집어넣는다.

OUT_TO_SERIAL:
		mov		bl, al		;bl에 문자를 복사한다.
		mov		dx, 0x3fd	;dx에 0x3fd라는 포트주소를 넣는다. (0x3fd는 COMI 포트의 LSR을 가리킨다.)
CHECK_LINE_STATUS:
		in		al, dx		;dx(0x3fd 라인 상태 레지스터)를 읽은 뒤
		and		al, 0x20	;0x20(5번째 비트)은 송신상태가 준비가 되었는지 확인한다.
		cmp		al, 0		;0이라면 송신이 준비되지 않았으므로
		jz		CHECK_LINE_STATUS	;반복한다.
		mov		dx, 0x3f8	;dx에 0x3f8이라는 주소를 넣는다. 0x3f8은 COM1 포트이다.
		mov		al, bl
		out		dx, al		;out 으로 dx가 가리키는 포트로 문자를 출력한다.

		inc		esi		;esi를 증가시켜 다음 문자를 가리킨다.
		inc		edi		;edi를 증가시켜 다음 문자로 이동한다.
		jmp		PRINT_LOOP	;PRINT_LOOP로 jump를 한다.
PRINT_END:
LINE_FEED:
		mov		dx, 0x3fd
		in		al, dx
		and		al, 0x20
		cmp		al, 0		;dx(0x3fd, 라인 상태 레지스터)를 읽은 뒤 송신 상태가 준비가 되었는지 확인을 반복한다.
		jz		LINE_FEED
		mov		dx, 0x3f8
		mov		al, 0x0a
		out		dx, al		;0x3f8포트로 0x0a(Line Feed)를 출력한다.
CARRIAGE_RETURN:
		mov		dx, 0x3fd
		in		al, dx
		and		al, 0x20
		cmp		al, 0		;dx(0x3fd, 라인 상태 레지스터)를 읽은 뒤 송신 상태가 준비가 되었는 지 확인을 반복한다.
		jz		CARRIAGE_RETURN
		mov		dx, 0x3f8	;0x3f8포트로 0x0d(Carriage Return)를 출력한다.
		mov		al, 0x0d
		out		dx, al

		pop		es		;es, edx, ebx, eax의 값을 스택에서 꺼내서 복원 후 함수 종료 호출한 곳으로 되돌아가기
		pop		edx
		pop		ebx
		pop		eax
		ret

GDT_DESC:
        dw GDT_END - GDT - 1    
        dd GDT                 
GDT:
		NULLDESCRIPTOR equ 0x00		;GDT 테이블을 정의한다. 세그먼트 영역에 대한 데이터를 일정한 디스크립터 형식으로 기술하고 이를 하나의 테이블에 모아두는 것인데, 이는 각 디스크립터 필드에 대해 정의한다. NULL 세그먼트 디스크립터는 모든 비트가 0이다.
			dw 0 
			dw 0 
			db 0 
			db 0 
			db 0 
			db 0
		CODEDESCRIPTOR  equ 0x08
			dw 0xffff             
			dw 0x0000              
			db 0x00                
			db 0x9a                    
			db 0xcf                
			db 0x00                
		DATADESCRIPTOR  equ 0x10
			dw 0xffff              
			dw 0x0000              
			db 0x00                
			db 0x92                
			db 0xcf                
			db 0x00                
		VIDEODESCRIPTOR equ 0x18
			dw 0xffff              ;세그먼트의 최대값 0xffff
			dw 0x8000              ;세그먼트의 시작주소 0 ~ 15 : 0x8000
			db 0x0b                ;세그먼트의 시작 주소 16 ~ 23 : 0x0B
			db 0x92                ;0x92 = 1001 0010 : P(1), DPL(0), S(1), data, readable, writable
			db 0x40                ;0x40 = 0100 1111 : G(0), D/B(1), L(0), AVL(0), limit 0xF
			;db 0xcf                    
			db 0x00                 ;세그먼트의 시작주소 24~31 : 0x0000
		IDTDESCRIPTOR	equ 0x20
			dw 0xffff
			dw 0x0000
			db 0x02
			db 0x92
			db 0xcf
			db 0x00
GDT_END:
IDTR:
		dw 256*8-1
		dd 0x00020000
IDT_IGNORE:
		dw ISR_IGNORE
		dw CODEDESCRIPTOR
		db 0
		db 0x8E
		dw 0x0000
ISR_IGNORE:
		push	gs
		push	fs
		push	es
		push	ds
		pushad
		pushfd
		cli
		nop
		sti
		popfd
		popad
		pop		ds
		pop		es
		pop		fs
		pop		gs
		iret



msgRMode db "Real Mode", 0
msgPMode db "Protected Mode", 0

 
times 	2048-($-$$) db 0x00
