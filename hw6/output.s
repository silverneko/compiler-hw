.data
_glob: .word 0
_flob: .word 0
.align 3
.text
_start_f:
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
mov x19, #0
.data
_const_1: .word 2
.align 3
.text
ldr x20, _const_1
mul x19, x19, x20
.data
_integer_const_2: .word 0
.align 3
.text
ldr w20, _integer_const_2
lsl x20, x20, #2
add x19, x19, x20
.data
_const_3: .word -16
.align 3
.text
ldrsw x20, _const_3
sub x20, x29, x20
ldr x20, [x20, #0]
add x19, x19, x20
ldr w19, [x19, #0]
mov w0, w19
bl _write_int
.data
_string_const_4: .asciz "\n"
.align 3
.text
ldr x19, =_string_const_4
mov x0, x19
bl _write_str
.data
_integer_const_5: .word 20
.align 3
.text
ldr w19, _integer_const_5
mov x20, #0
.data
_integer_const_6: .word 1
.align 3
.text
ldr w21, _integer_const_6
.data
_const_7: .word 2
.align 3
.text
ldr x22, _const_7
mul x20, x20, x22
lsl x21, x21, #2
add x20, x20, x21
.data
_const_8: .word -16
.align 3
.text
ldrsw x21, _const_8
sub x21, x29, x21
ldr x21, [x21, #0]
add x20, x20, x21
str w19, [x20, #0]
.data
_integer_const_9: .word 0
.align 3
.text
ldr w19, _integer_const_9
mov w0, w19
b _end_f
_end_f:
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
.text
_start_MAIN:
b _global_init
_global_init_back:
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
_AR_SIZE_10: .word 88
.align 3
.text
ldr w19, _AR_SIZE_10
sub sp, sp, w19
.data
_integer_const_11: .word 16
.align 3
.text
ldr w20, _integer_const_11
sub sp, sp, w20
.data
_integer_const_12: .word 10
.align 3
.text
ldr w20, _integer_const_12
mov x21, #0
.data
_integer_const_13: .word 0
.align 3
.text
ldr w22, _integer_const_13
.data
_const_14: .word 2
.align 3
.text
ldr x23, _const_14
mul x21, x21, x23
lsl x22, x22, #2
add x21, x21, x22
.data
_integer_const_15: .word 0
.align 3
.text
ldr w22, _integer_const_15
.data
_const_16: .word 2
.align 3
.text
ldr x23, _const_16
mul x21, x21, x23
lsl x22, x22, #2
add x21, x21, x22
add x21, x21, x29
.data
_const_17: .word 104
.align 3
.text
ldrsw x22, _const_17
neg x22, x22
add x21, x21, x22
str w20, [x21, #0]
.data
_integer_const_18: .word 8
.align 3
.text
ldr w20, _integer_const_18
sub sp, sp, w20
mov x20, #0
.data
_const_19: .word 2
.align 3
.text
ldr x21, _const_19
mul x20, x20, x21
.data
_integer_const_20: .word 0
.align 3
.text
ldr w21, _integer_const_20
lsl x21, x21, #2
add x20, x20, x21
add x20, x20, x29
.data
_const_21: .word 104
.align 3
.text
ldrsw x21, _const_21
neg x21, x21
add x20, x20, x21
.data
_const_22: .word 112
.align 3
.text
ldrsw x21, _const_22
sub x21, x29, x21
str x20, [x21, #0]
bl _start_f
.text
ldr w20, _integer_const_18
add sp, sp, w20
mov x20, #0
.data
_const_23: .word 2
.align 3
.text
ldr x21, _const_23
mul x20, x20, x21
.data
_integer_const_24: .word 0
.align 3
.text
ldr w21, _integer_const_24
lsl x21, x21, #2
add x20, x20, x21
.data
_const_25: .word 2
.align 3
.text
ldr x21, _const_25
mul x20, x20, x21
.data
_integer_const_26: .word 1
.align 3
.text
ldr w21, _integer_const_26
lsl x21, x21, #2
add x20, x20, x21
add x20, x20, x29
.data
_const_27: .word 104
.align 3
.text
ldrsw x21, _const_27
neg x21, x21
add x20, x20, x21
ldr w20, [x20, #0]
mov w0, w20
bl _write_int
.data
_string_const_28: .asciz "\n"
.align 3
.text
ldr x20, =_string_const_28
mov x0, x20
bl _write_str
.data
_integer_const_29: .word 0
.align 3
.text
ldr w20, _integer_const_29
mov w0, w20
b _end_MAIN
_end_MAIN:
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
.text
_global_init:
.data
_integer_const_30: .word 123
.align 3
.text
ldr w21, _integer_const_30
ldr x22, =_glob
str w21, [x22, #0]
.data
_float_const_31: .float 1241.121000
.align 3
.text
ldr s21, _float_const_31
ldr x22, =_flob
str s21, [x22, #0]
b _global_init_back
