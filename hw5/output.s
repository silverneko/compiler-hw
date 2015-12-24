.data
_g_n: .word 0
.text
.text
_start_fact:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
ldr x30, =_frameSize_fact
ldr x30, [x30, #0]
sub sp, sp, w30
str x9, [sp, #8]
str x10, [sp, #16]
str x11, [sp, #24]
str x12, [sp, #32]
str x13, [sp, #40]
str x14, [sp, #48]
str x15, [sp, #56]
str s16, [sp, #64]
str s17, [sp, #68]
str s18, [sp, #72]
str s19, [sp, #76]
str s20, [sp, #80]
str s21, [sp, #84]
str s22, [sp, #88]
str s23, [sp, #92]
#       }

ldr x14, =_g_n
ldr w9, [x14,#0]
.data
_CONSTANT_1: .word 1
.align 3
.text
ldr w10, _CONSTANT_1
cmp w9, w10
cset w9, eq
cmp w9, #0
beq _elseLabel_0
#       }

#               return n;

ldr x14, =_g_n
ldr w9, [x14,#0]
mov w0, w9
b _end_fact
b _ifExitLabel_0
_elseLabel_0:
#       }

#               n =n-1;

ldr x14, =_g_n
ldr w9, [x14,#0]
.data
_CONSTANT_2: .word 1
.align 3
.text
ldr w10, _CONSTANT_2
sub w9, w9, w10
ldr x10, =_g_n
str w9, [x10, #0]
#               return (n*fact());

ldr x14, =_g_n
ldr w9, [x14,#0]
bl _start_fact
mov w10, w0
mul w9, w9, w10
mov w0, w9
b _end_fact
_ifExitLabel_0:
_end_fact:
ldr x9, [sp, #8]
ldr x10, [sp, #16]
ldr x11, [sp, #24]
ldr x12, [sp, #32]
ldr x13, [sp, #40]
ldr x14, [sp, #48]
ldr x15, [sp, #56]
ldr s16, [sp, #64]
ldr s17, [sp, #68]
ldr s18, [sp, #72]
ldr s19, [sp, #76]
ldr s20, [sp, #80]
ldr s21, [sp, #84]
ldr s22, [sp, #88]
ldr s23, [sp, #92]
ldr x30, [x29, #8]
mov sp, x29
add sp, sp, #8
ldr x29, [x29,#0]
RET x30
.data
_frameSize_fact: .word 92
.text
_start_MAIN:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
ldr x30, =_frameSize_MAIN
ldr x30, [x30, #0]
sub sp, sp, w30
str x9, [sp, #8]
str x10, [sp, #16]
str x11, [sp, #24]
str x12, [sp, #32]
str x13, [sp, #40]
str x14, [sp, #48]
str x15, [sp, #56]
str s16, [sp, #64]
str s17, [sp, #68]
str s18, [sp, #72]
str s19, [sp, #76]
str s20, [sp, #80]
str s21, [sp, #84]
str s22, [sp, #88]
str s23, [sp, #92]
#       write("Enter a number:");

.data
_CONSTANT_3: .ascii "Enter a number:\000"
.align 3
.text
ldr x9, =_CONSTANT_3
mov x0, x9
bl _write_str
#       n = read();

bl _read_int
mov w9, w0
ldr x10, =_g_n
str w9, [x10, #0]
#       n = n+1;

ldr x14, =_g_n
ldr w9, [x14,#0]
.data
_CONSTANT_4: .word 1
.align 3
.text
ldr w10, _CONSTANT_4
add w9, w9, w10
ldr x10, =_g_n
str w9, [x10, #0]
#       }

ldr x14, =_g_n
ldr w9, [x14,#0]
.data
_CONSTANT_6: .word 1
.align 3
.text
ldr w10, _CONSTANT_6
cmp w9, w10
cset w9, gt
cmp w9, #0
beq _elseLabel_5
#       }

#               result = fact();

bl _start_fact
mov w9, w0
str w9, [x29, #-4]
b _ifExitLabel_5
_elseLabel_5:
#       }

#               result = 1;

.data
_CONSTANT_7: .word 1
.align 3
.text
ldr w9, _CONSTANT_7
str w9, [x29, #-4]
_ifExitLabel_5:
#       write("The factorial is ");

.data
_CONSTANT_8: .ascii "The factorial is \000"
.align 3
.text
ldr x9, =_CONSTANT_8
mov x0, x9
bl _write_str
#       write(result);

ldr w9, [x29, #-4]
mov w0, w9
bl _write_int
#       write("\n");

.data
_CONSTANT_9: .ascii "\n\000"
.align 3
.text
ldr x9, =_CONSTANT_9
mov x0, x9
bl _write_str
_end_MAIN:
ldr x9, [sp, #8]
ldr x10, [sp, #16]
ldr x11, [sp, #24]
ldr x12, [sp, #32]
ldr x13, [sp, #40]
ldr x14, [sp, #48]
ldr x15, [sp, #56]
ldr s16, [sp, #64]
ldr s17, [sp, #68]
ldr s18, [sp, #72]
ldr s19, [sp, #76]
ldr s20, [sp, #80]
ldr s21, [sp, #84]
ldr s22, [sp, #88]
ldr s23, [sp, #92]
ldr x30, [x29, #8]
mov sp, x29
add sp, sp, #8
ldr x29, [x29,#0]
RET x30
.data
_frameSize_MAIN: .word 92

