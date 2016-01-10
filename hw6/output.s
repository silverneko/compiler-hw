.data
_glob: .word 0
_flob: .word 0
.align 3
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
_AR_SIZE_0: .word 88
.align 3
.text
ldr w19, _AR_SIZE_0
sub sp, sp, w19
.data
_integer_const_1: .word 16
.align 3
.text
ldr w19, _integer_const_1
sub sp, sp, w19
.data
_integer_const_2: .word 20
.align 3
.text
ldr w19, _integer_const_2
.data
_const_3: .word 92
.align 3
.text
ldr w20, _const_3
sub x20, x29, x20
str w19, [x20, #0]
.data
_integer_const_4: .word 200
.align 3
.text
ldr w19, _integer_const_4
.data
_const_5: .word 96
.align 3
.text
ldr w20, _const_5
sub x20, x29, x20
str w19, [x20, #0]
.data
_float_const_6: .float 3.213000
.align 3
.text
ldr s19, _float_const_6
.data
_const_7: .word 100
.align 3
.text
ldr w20, _const_7
sub x20, x29, x20
fcvtzs w19, s19
str w19, [x20, #0]
.data
_float_const_8: .float 0.123400
.align 3
.text
ldr s19, _float_const_8
.data
_const_9: .word 104
.align 3
.text
ldr w20, _const_9
sub x20, x29, x20
str s19, [x20, #0]
.data
_const_10: .word 92
.align 3
.text
ldr w19, _const_10
sub x19, x29, x19
ldr w19, [x19, #0]
mov w0, w19
bl _write_int
.data
_string_const_11: .asciz "\n"
.align 3
.text
ldr x19, =_string_const_11
mov x0, x19
bl _write_str
.data
_const_12: .word 96
.align 3
.text
ldr w19, _const_12
sub x19, x29, x19
ldr w19, [x19, #0]
mov w0, w19
bl _write_int
.data
_string_const_13: .asciz "\n"
.align 3
.text
ldr x19, =_string_const_13
mov x0, x19
bl _write_str
.data
_const_14: .word 104
.align 3
.text
ldr w19, _const_14
sub x19, x29, x19
ldr s19, [x19, #0]
fmov s0, s19
bl _write_float
.data
_string_const_15: .asciz "\n"
.align 3
.text
ldr x19, =_string_const_15
mov x0, x19
bl _write_str
.data
_const_16: .word 100
.align 3
.text
ldr w19, _const_16
sub x19, x29, x19
ldr w19, [x19, #0]
mov w0, w19
bl _write_int
.data
_string_const_17: .asciz "\n"
.align 3
.text
ldr x19, =_string_const_17
mov x0, x19
bl _write_str
ldr w19, _glob
mov w0, w19
bl _write_int
.data
_string_const_18: .asciz "\n"
.align 3
.text
ldr x19, =_string_const_18
mov x0, x19
bl _write_str
ldr s19, _flob
fmov s0, s19
bl _write_float
.data
_string_const_19: .asciz "\n"
.align 3
.text
ldr x19, =_string_const_19
mov x0, x19
bl _write_str
.data
_integer_const_20: .word 0
.align 3
.text
ldr w19, _integer_const_20
mov w0, w19
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
_integer_const_21: .word 123
.align 3
.text
ldr w20, _integer_const_21
ldr x21, =_glob
str w20, [x21, #0]
.data
_float_const_22: .float 1241.121000
.align 3
.text
ldr s20, _float_const_22
ldr x21, =_flob
str s20, [x21, #0]
b _global_init_back
