[org 7c00h]
[bits   16]

jmp     0:main   ; nullify cs if the bootsect is loaded at 7c0h:0

PRIM_DESC_LBA   equ 16

BUFFER          equ 0x8000
DESC_TYPE_CODE  equ BUFFER
DESC_ROOT_DIR   equ BUFFER+156

; entrance point
main:
    mov     ax,cs
    mov     ds,ax           ; Fix ds and es, set them equal to cs
    mov     es,ax

    xor     ax, ax
    mov     ss, ax
    mov     sp, 0x7c00      ; setup stack

    mov     [driveNum], dl  ; The bios pass us the drive number in dl, we need to save it first

    mov     eax, PRIM_DESC_LBA
    mov     cx, 1
    mov     di, BUFFER
    call    readSector

    cmp     byte [DESC_TYPE_CODE], 1
    jnz     assertionFailed

    ; Load the "/"
    mov     edi, DESC_ROOT_DIR
    call    loadDirTable

    ; Search for "SAKI"
    mov     esi, dirNameSaki
    call    searchForFile
    
    ; Load "/SAKI"
    call    loadDirTable

    ; Search for "BOOTMGR"
    mov     esi, dirNameBootmgr
    call    searchForFile

    ; Load "/SAKI/BOOTMGR"
    call    loadDirTable
    
    ; Search for "BOOTMGR.BIN;1"
    mov     esi, dirNameBooter
    call    searchForFile   

    ; Load "/SAKI/BOOTMGR/BOOTMGR.BIN;1"
    mov     eax, [edi+2]    ; LBA Address
    mov     ecx, [edi+10]   ; Size of file
    add     ecx, 2048       
    shr     ecx, 11         ; Change unit from byte to sector
    mov     di, BUFFER
    call    readSector

    mov     dl, [driveNum]
    jmp     0:BUFFER
    ; Print information
    mov     esi, BUFFER
    call    print
    cli
    hlt
;end main


; Load directory table into memory
; @param edi        directory entry
; @destory eax, ecx, edi
loadDirTable:
    mov     eax, [edi+2]
    mov     cx, 1
    mov     di, BUFFER
    call    readSector
    ret
;end loadDirTable


; Search for file in directory at BUFFER
; @param esi        filename
; @destory eax, ecx, edx, edi
searchForFile:
    call    strlen          ; Get length of filename
    mov     edi, BUFFER
.loop:
    movzx   eax, byte[edi]  ; Get size of record
    or      eax, eax        ; If zero, no more record is available
    jz      .err

    movzx   ecx, byte[edi+32]   ; Get length of actual filename
    cmp     ecx, edx            ; If not equal, next record
    jnz     .cont

    push    esi
    push    edi
    add     edi, 33         ; Set edi to the actual filename
    
    repe    cmpsb
    pop     edi
    pop     esi
    jz      .ret
.cont:
    add     edi, eax        ; Point to the next record
    jmp     .loop
.ret:
    ret
.err:
    call    print
    mov     si, fileNotFound
    call    print
    cli
    hlt
;end searchForFile


; print given string
; @param si     offset of the string
; @destroy ax, bx, si
print:
    mov     bx, 0000Fh
    mov     ah, 0Eh
.loop:
    mov     al, [si]
    inc     si
    or      al, al
    jz      .end
    int     10h
    jmp     .loop
.end:
    ret
;end print


; read a sector from the boot device
; @param eax    lba address of the first sector to read
; @param cx     number of sectors
; @param edi    buffer to store read sectors
readSector:
    mov     [DAP.lbaLow], eax
    mov     [DAP.sectCount], cx
    mov     eax, edi    
    shr     eax, 4                  ; Calculate the segment from absolute address
    mov     [DAP.bufferSeg] ,ax
    and     di, 0xf                 ; Calculate the offset from absolute address
    mov     [DAP.bufferOff], di
    mov     ah, 0x42
    mov     dl, [driveNum]
    mov     si, DAP;
    int     13h
    ret
;end readSector

; Find length of given string
; @param si     target string
; @return edx   length of string
; @destory eax, edx, ecx, edi
strlen:
    mov     di, si
    or      cx, -1      ; -1 means that the function will never stop
    xor     ax, ax      ; Clear eax to 0
    repne   scasb       ; Compare until [esi] to 0
    neg     cx          ; Since cx=-1-(len+1), we change the cx to 1+(len+1)
    dec     cx
    dec     cx
    movzx   edx, cx
    ret
;end strlen


assertionFailed:
    mov     si, typeNotPVD
    call    print
    cli
    hlt

typeNotPVD db "PVD is not first desc",0
fileNotFound db " does not exist in cdrom.",0

dirNameSaki db "SAKI",0
dirNameBootmgr db "BOOTMGR",0
dirNameBooter db "BOOTMGR.BIN;1",0

; driveNum is the initial dl value passed to us by bios
driveNum    db  0

; Disk Address Packet(DAP), argument for extended 13h
DAP:
    .packetSize  db 16, 0
    .sectCount   dw 0
    .bufferOff   dw 0
    .bufferSeg   dw 0
    .lbaLow      dd 0
    .lbaHigh     dd 0

times 510-($-$$) db 0
dw   0xAA55  ; End mark















