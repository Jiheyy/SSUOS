org	0x7c00  	;메모리의 몇 번지에서 실행해야 하는지 알려주는 선언문 

[BITS 16]		;이 프로그램이 16비튿 단위로 데이터를 처리하는 프로그램임으로 알린다.

START:  

	mov	ax, 0xb800
	mov	es, ax
	mov	ax, 0x00
	mov	bx, 0
	mov 	cx, 80*25*2
;	jmp		SSU1 ;BOOT1_LOAD로 점프

CLS:
	mov	[es:bx], ax
	add	bx, 1
	loop	CLS	;화면 초기화

	mov     ax, 0xb800
    mov     es, ax
    mov     bx, 0
    mov     ah, 0x07
	mov		si, 0
	mov		dx, 1	;dx는 현재 위치를 알기 위해 사용, 1은 ssuos_1 2는 ssuos_2, 3은 ssuos_3

PRINT:
	mov ah, 0x0f
	mov	bx, 0
	mov	si, 0

;ssuos_1 출력
SSU1:
    mov     al, [ssuos_1+si]
    mov     [es:bx], ax
    add     bx, 2
	add     si, 1
	cmp	al, 0
	jne SSU1		
		
	mov	si, 0
	add	bx, 10

;ssuos_2 출력	
SSU2:
      mov     al, [ssuos_2+si]
      mov     [es:bx], ax
      add     bx, 2
      add     si, 1
	  
	  cmp al, 0
	  jne SSU2
		  
	  mov si, 0
	  mov	bx, 160 
		        
;ssuos_3 출력
SSU3:
        mov     al, [ssuos_3+si]
        add     si, 1
        mov     [es:bx], ax
        add     bx, 2
	
		cmp	al, 0
		jne	SSU3
	
		mov	si, 0
		mov bx, 0

;select에 따른 위치변경
SELECT:
	mov 	si, 0

	cmp		dx, 1	;1번 위치
	je		CHANGE1

	cmp		dx, 2	;2번위치
	je		CHANGE2

	cmp		dx, 3	;3번위치
	je		CHANGE3

;select 출력	
P_SEL:
	mov		al, [select+si]
	add		si, 1
	mov		[es:bx], ax
	add		bx, 2

	cmp		al, 0
	jne		P_SEL
	
;키보드 인터럽트
INPUT:
	mov		ah, 10h
	int		16h


	cmp		ah, $48	;up
	je		UP

	cmp		ah, $4D	;right
	je		RIGHT
	
	cmp		ah, $4B	;left
	je		LEFT

	cmp		ah, $50	;down
	je		DOWN

	cmp		ah, $1C	;enter
	je		KERNEL_SELECT	

UP:

	cmp	dx, 2	;2번에 위치할 경우 올라갈 수 없으므로 그냥 출력
	je	PRINT
	
	cmp	dx, 1	;1번도 마찬가지
	je	PRINT

	mov	dx, 1	;3번일 경우 올라갈 수 있으니까 dx(현재위치)를 1로 바꿔줌
	jmp	PRINT

RIGHT:

	cmp	dx, 3
	je	PRINT

	cmp	dx, 2
	je	PRINT
	
	mov	dx, 2	;1번일 경우 현재위치를 2로 바꿔줌
	jmp	PRINT

LEFT:

	cmp	dx, 1
	je	PRINT

	cmp	dx, 3
	je	PRINT

	mov	dx, 1	;2번일 경우 1로 바꿔줌
	jmp	PRINT

DOWN:

	cmp	dx, 2
	je	PRINT

	cmp	dx, 3
	je	PRINT

	mov	dx, 3 	;1번일 경우 위치를 3으로 바꿔줌
	jmp	PRINT

;현재 위치를 bx 0으로 첫 위치로 초기화	
CHANGE1:	
	mov	bx, 0
	jmp	P_SEL

;현재 위치를 2번 위치로 바꿔줌	
CHANGE2:
	mov	bx, 24+10
	jmp	P_SEL

;현재 위치를 3번 위치로 바꿔줌
CHANGE3:
	mov	bx, 160
	jmp	P_SEL

;커널 몇번을 선택했는지 dx를 통해 알아냄	
KERNEL_SELECT:

	cmp	dx, 1
	je	KERNEL_1

	cmp	dx, 2
	je	KERNEL_2

	cmp	dx,3
	je	KERNEL_3

;커널1의 CHS 값으로 설정	
KERNEL_1:
	mov		ch, 0
	mov		cl, 0x6
	mov		dh, 0

	jmp		KERNEL_LOAD

;커널2의 CHS 값으로 설정	
KERNEL_2:
	mov		ch, 9
	mov		cl, 47
	mov		dh, 14

	jmp		KERNEL_LOAD

;커널3의 CHS 값으로 설정
KERNEL_3:
	mov		ch, 14
	mov		cl, 7
	mov		dh, 14

	jmp		KERNEL_LOAD

;커널 로드
KERNEL_LOAD:
		mov     ax, 0x1000	
        mov     es, ax		
        mov     bx, 0x0		

        mov     ah, 2		
        mov     al, 0x3f	
        ;mov     ch, 0		
        ;mov     cl, 0x6	
        ;mov     dh, 0     
        mov     dl, 0x80 ;; 

        int     0x13
        jc      KERNEL_LOAD


BOOT1_LOAD:
	mov     ax, 0x0900 
        mov     es, ax		;ax를 통해 es를 0x0900으로 초기화한다.
        mov     bx, 0x0
        mov     ah, 2		;0x13 인터럽트 호출시 ah에 저장된 값에 따라 수행되는 결과가 다름. 2는 색터읽기
        mov     al, 0x4		;al 읽을 섹터 수를 지정 1~128사이의 값을 지정 가능
        mov     ch, 0	
        mov     cl, 2	
        mov     dh, 0		
        mov     dl, 0x80


        int     0x13		;0x13 인터럽트 호출
        jc      BOOT1_LOAD	;Carry 플래그 발생시(=Error) 다시 시도



END:
	jmp		0x0900:0x0000

select db "[O]",0
ssuos_1 db "[ ] SSUOS_1",0 
ssuos_2 db "[ ] SSUOS_2",0
ssuos_3 db "[ ] SSUOS_3",0
ssuos_4 db "[ ] SSUOS_4",0
partition_num : resw 1

times   446-($-$$) db 0x00

PTE:
partition1 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition2 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition3 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x98, 0x3a, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition4 db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
times 	510-($-$$) db 0x00
dw	0xaa55
