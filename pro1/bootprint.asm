[BITS 16]
ORG 0x7c00

START:   
mov		ax, 0xb800	;텍스트 출력용 메모리
mov		es, ax
mov		ax, 0x00
mov		bx, 0
mov		cx, 80*25*2	;반복문

CLS:
mov		[es:bx], ax	;es
add		bx, 1
loop 	CLS			;화면 초기화

mov		ax, 0xb800
mov		es, ax		;es 초기화
mov		ah, 0x07	;픽셀 글씨 색 또는 배경 굵기 깜빡임 등
mov		bx, 0		;bx 초기화
lea		si, [HELLO_MSG]	;si 에 Hello_msg의 주소값 집어넣기

PRINT:
mov		al, [si]	
add		si , 0x01		;값을 1증가 시킴
mov		[es:bx], ax	;es + bx 에 ax넣기
add		bx, 2	
cmp		al, 0		; al-0 = 0  : sf(부호):0 cf(올림수):0 zf(0, 두개의 비교값이 같을 때):1 
				;       < 0 : sf:1 cf:0 zf:0
				;	> 0 : sf:0, zf:0, cf:0
jnz		PRINT		;jump if not zero (!=)

HELLO_MSG:
db "Hello, Jihye's World", 0x00


