.intel_syntax noprefix
.global k_scroll
.global k_clearscr
.global k_print

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