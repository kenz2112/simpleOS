.intel_syntax noprefix
.global k_scroll
.global k_clearscr
.global k_print

//Assignment 2
.global kbd_enter
.global _got_key
.global _scan_buffer
.global lidtr
.global outportb
.global sti_command
.extern kbd_handler


k_scroll:
    pushad
    pushf
    mov esi, 80* 2 + 0xb8000
    mov edi, 0xb8000
    mov ecx, 80 * 24 * 2
    rep movsb
    mov ecx, 80
    mov al, ' '
    mov ah, 31
    rep stosw
    popf
    popad
    ret

k_clearscr:
    pushad
    pushf
    mov esi, 0xb8000
    mov edi, 0xb8000
    mov ecx, 80 * 25
    mov al, ' '
    mov ah, 31
    rep stosw
    popf
    popad
    ret

k_print:
    push ebp
    mov ebp, esp
    pushad
    mov ecx, [ebp + 16]
    imul ecx, 80
    mov edx, [ebp + 20]
    add ecx, edx
    imul ecx, 2
    add ecx, 0xb8000
    mov esi, [ebp + 8]
    mov edi, ecx
    mov ebx, [ebp + 12]

loop: 
    cmp ebx, 0
    je done
    cmp edi, (0xB8000 + (80 * 25 * 2))
    jg done
    movsb
    movb [edi], 31
    inc edi
    dec ebx
    cmp ebx, 0
    jg loop

done: 
    popad
    pop ebp
    ret

//Assignment 2
kbd_enter:
    cli
    pushf
    pushad
    mov eax, 0
    in al,0x64
    and al, 0x01
    jz _kbd_skip
    in al, 0x60
    mov edx, eax
    push edx
    call kbd_handler
    add esp, 4

_kbd_skip:
    mov al, 0x20
    out 0x20, al
    popad
    popf
    iret

_got_key:
    .asciz "got key:"

_scan_buffer:
    .asciz "          "

lidtr:
    push ebp
    mov ebp, esp
    pushad
    pushf
    mov eax, [ebp + 8]
    lidt [eax]
    popf
    popa
    pop ebp 
    ret

outportb:
    push ebp
    mov ebp, esp
    pusha
    pushf
    mov edx, [ebp+8]
    mov eax, [ebp +12]
    out dx, al
    popf
    popa
    pop ebp
    ret
sti_command:
    sti
    ret