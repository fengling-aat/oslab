/* Real Mode Hello World */
/*.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	movw $0x7d00, %ax
	movw %ax, %sp # setting stack pointer to 0x7d00
    pushw $13 # pushing the size to print into stack
    pushw $message # pushing the address of message into stack
    callw displayStr # calling the display function
loop:
    jmp loop

message:
    .string "Hello, World!\n\0"

displayStr:
    pushw %bp
    movw 4(%esp), %ax
    movw %ax, %bp
    movw 6(%esp), %cx
    movw $0x1301, %ax
    movw $0x000c, %bx
    movw $0x0000, %dx
    int $0x10
    popw %bp
    ret
*/


/* Protected Mode Hello World */
/*.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
    cli
    inb $0x92,%al
    //into nan qiao xin pian
 
    orb $0x02,%al
    //open xin pian di er wei

    outb %al,$0x92
    //pei zhi fan hui gei xin pian
    data32 addr32 lgdt gdtDesc
    //orl $0x01,%cr0

    movl %cr0, %eax
    orb $0x01, %al
    movl %eax, %cr0
    //set cr0 first 1
    data32 ljmp $0x08, $start32

.code32
start32:
	movw $0x10, %ax # setting data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
    //movl $0x48656c6c,0xb8000
    movl $0x0f650f48,0xb8000
    movl $0x0f6c0f6c,0xb8004
    //movl $0x6f2c776f,0xb8004
    movl $0x0f2c0f6f,0xb8008
    movl $0x0f6f0f77,0xb800c
    //movl $0x726c6421,0xb8008
    movl $0x0f6c0f72,0xb8010
    movl $0x0f210f64,0xb8014
loop32:
	jmp loop32


.p2align 2
gdt:
    .word 0,0
    .byte 0,0,0,0
 
    .word 0xffff,0
    .byte 0,0x9a,0xcf,0
 
    .word 0xffff,0
    .byte 0,0x92,0xcf,0

    .word 0xffff,0x8000
    .byte 0x0b,0x92,0xcf,0
    #GDT definition here
 
gdtDesc: 
    #gdtDesc definition here
    .word (gdtDesc-gdt-1)
    .long gdt
*/
/* Protected Mode Loading Hello World APP */
.code16

.global start
start:
    movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
    #TODO: Protected Mode Here
    cli
    inb $0x92,%al 
    //into nan qiao xin pian

    orb $0x02,%al 
    //open xin pian di er wei

    outb %al,$0x92 
    //pei zhi fan hui gei xin pian
    data32 addr32 lgdt gdtDesc
    //orl $0x01,%cr0

    movl %cr0, %eax
    orb $0x01, %al
    movl %eax, %cr0
    //set cr0 first 1
    data32 ljmp $0x08, $start32

.code32
start32:
	movw $0x10, %ax # setting data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x18, %ax # setting graphics data segment selector
	movw %ax, %gs
	
	movl $0x8000, %eax # setting esp
	movl %eax, %esp
	jmp bootMain # jump to bootMain in boot.c

.p2align 2
gdt:
    .word 0,0
    .byte 0,0,0,0

    .word 0xffff,0
    .byte 0,0x9a,0xcf,0

    .word 0xffff,0
    .byte 0,0x92,0xcf,0

    .word 0xffff,0x8000
    .byte 0x0b,0x92,0xcf,0
	#GDT definition here

gdtDesc: 
	#gdtDesc definition here
    .word (gdtDesc-gdt-1)
    .long gdt

