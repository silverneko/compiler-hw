.text
_start_main:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
str x19, [x29, #-8]
str x20, [x29, #-16]
str x21, [x29, #-24]
str x22, [x29, #-32]
str x23, [x29, #-40]
str x24, [x29, #-48]
str x25, [x29, #-56]
str x26, [x29, #-64]
str x27, [x29, #-72]
str x28, [x29, #-80]
str x29, [x29, #-88]
.data
_AR_SIZE_0: .word 88
.align 3
.text
ldr w19, _AR_SIZE_0
sub sp, sp, w19
.data
_integer_const_1: .word 4
.align 3
.text
ldr w19, _integer_const_1
sub sp, sp, w19
_IF_2:
.data
_const_3: .word 92
.align 3
.text
ldr w19, _const_3
sub x19, x29, x19
ldr w19, [x19, #0]
str w19, [sp, #0]
sub sp, sp, #8
.data
_integer_const_4: .word 10
.align 3
.text
ldr w19, _integer_const_4
add sp, sp, #8
ldr w20, [sp, #0]
cmp w20, w19
beq _ELSE_5
mov w20, #0
b _END_5
_ELSE_5:
mov w20, #1
_END_5:
str w20, [sp, #0]
sub sp, sp, #8
.data
_integer_const_6: .word 23
.align 3
.text
ldr w19, _integer_const_6
add sp, sp, #8
ldr w20, [sp, #0]
cmp w20, #0
beq _ELSE_7
cmp w19, #0
beq _ELSE_7
mov w20, #1
b _END_7
_ELSE_7:
mov w20, #0
_END_7:
cmp w20, #0
beq _ELSE_2
.data
_OLD_SP_8: .skip 8
.text
ldr x19, =_OLD_SP_8
mov x20, sp
str x20, [x19, #0]
_IF_9:
.data
_const_10: .word 92
.align 3
.text
ldr w19, _const_10
sub x19, x29, x19
ldr w19, [x19, #0]
str w19, [sp, #0]
sub sp, sp, #8
.data
_integer_const_11: .word 20
.align 3
.text
ldr w19, _integer_const_11
add sp, sp, #8
ldr w20, [sp, #0]
cmp w20, w19
beq _ELSE_12
mov w20, #0
b _END_12
_ELSE_12:
mov w20, #1
_END_12:
cmp w20, #0
beq _ELSE_9
.data
_OLD_SP_13: .skip 8
.text
ldr x19, =_OLD_SP_13
mov x20, sp
str x20, [x19, #0]
.data
_integer_const_14: .word 4
.align 3
.text
ldr w19, _integer_const_14
sub sp, sp, w19
.data
_const_15: .word 96
.align 3
.text
ldr w19, _const_15
sub x19, x29, x19
ldr w19, [x19, #0]
mov w0, w19
b _end_main
ldr x20, =_OLD_SP_13
ldr x21, [x20, #0]
mov sp, x21
b _END_IF_9
_ELSE_9:
_END_IF_9:
ldr x20, =_OLD_SP_8
ldr x21, [x20, #0]
mov sp, x21
b _END_IF_2
_ELSE_2:
_END_IF_2:
.data
_const_16: .word 92
.align 3
.text
ldr w20, _const_16
sub x20, x29, x20
ldr w20, [x20, #0]
mov w0, w20
b _end_main
_end_main:
ldr x19, [x29, #-8]
ldr x20, [x29, #-16]
ldr x21, [x29, #-24]
ldr x22, [x29, #-32]
ldr x23, [x29, #-40]
ldr x24, [x29, #-48]
ldr x25, [x29, #-56]
ldr x26, [x29, #-64]
ldr x27, [x29, #-72]
ldr x28, [x29, #-80]
ldr x29, [x29, #-88]
ldr x30, [x29, #8]
add sp, x29, #8
ldr x29, [x29, #0]
ret x30
