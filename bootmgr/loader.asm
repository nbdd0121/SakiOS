[bits   16]
[global _start]
[extern main]

; Constants for debugging
%define TURN_OFF_GRAPHICS 1

; The stack will grow from 0x8000 to bottom
stackTop:

; Entrance point, will be detected by ld
_start:
    mov     sp, stackTop        ; setup stack, notice that segment registers are already set by bootsect
    
    ; Acquire all possible VBE modes
    mov     ax, 0x4F00
    mov     di, 0x600
    int     0x10

    movzx   edx, word[0x600+16]
    shl     dx, 4
    add     dx, [0x600+14]

    ; Loop to check each of them
    mov     di, 0x500
.loop:
    mov     cx, [edx]
    cmp     cx, -1
    je      .noAcceptableMode

    mov     ax, 0x4F01
    
    int     0x10

    ; Next mode
    inc     dx
    inc     dx

    ; Make sure that the frame buffer is aval
    test	byte[di], 0x80
	jz		.loop

	; Check if the condition was satisfied
	; Need to be direct color, not palette or packed
	cmp		byte[di+0x1B], 6
	jne		.loop

	; Filter out useless resolutions, we need bigget than 800
	cmp		word[di+0x12], 1024
	jb		.loop

	; The following checks make sure that every
	; pixel was 24 bit or 32 bit wide, and coded as RGB.
	cmp		byte[di+0x1F], 8
	jne		.loop
	cmp		byte[di+0x20], 16
	jne		.loop
	cmp		byte[di+0x21], 8
	jne		.loop
	cmp		byte[di+0x22], 8
	jne		.loop
	cmp		byte[di+0x23], 8
	jne		.loop
	cmp		byte[di+0x24], 0
	jne		.loop

.end:
    ; Set all needed video informations
    mov     eax, [di+0x28]
    mov     [Video_info.addr], eax

    mov     ax, [di+0x12]
    mov     [Video_info.xRes], ax
    
    mov     ax, [di+0x14]
    mov     [Video_info.yRes], ax

    mov     ax, [di+0x10]
    mov     [Video_info.bpl], ax

    mov     al, [di+0x19]
    mov     [Video_info.bpp], al

    mov     bx, [edx-2]
    bts     bx, 14
    mov     ax, 0x4F02
%if TURN_OFF_GRAPHICS==0
    int     0x10
%endif

.noAcceptableMode:

    ; Load GDT
    cli
    lgdt    [GDTPtr]

    ; Switched on A20
    in      al , 92h
    or      al , 010b
    out     92h, al

    ; Entering Protected Mode
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax

    jmp     dword Sel.code32:pmStart
    
;end main

GDT:
    .dummy      dq 0
    .code64     dq 00af9a000000ffffh
    .data64     dq 00af92000000ffffh
    .data32     dq 00cf92000000ffffh
    .code32     dq 00cf9a000000ffffh
    .end:

Sel.code32 equ GDT.code32-GDT
Sel.data32 equ GDT.data32-GDT

GDTPtr:
    dw GDT.end-GDT
    dd GDT

[bits 32]

pmStart:
    mov     ax, Sel.data32
    mov     gs, ax
    mov     fs, ax
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     esp, stackTop

    call    main

    cli
    hlt

[section .data]
[global Video_info]

Video_info:
.addr   dd 0
.xRes   dw 0
.yRes   dw 0
.bpl    dw 0
.bpp    db 0


    
















