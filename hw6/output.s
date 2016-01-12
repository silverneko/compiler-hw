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
.data
_const_1: .word -16
.align 3
.text
ldrsw x19, _const_1
sub x19, x29, x19
ldr w19, [x19, #0]
mov w0, w19
bl _write_int
.data
_string_const_2: .asciz "\n"
.align 3
.text
ldr x19, =_string_const_2
mov x0, x19
bl _write_str
.data
_const_3: .word -24
.align 3
.text
ldrsw x19, _const_3
sub x19, x29, x19
ldr s19, [x19, #0]
fmov s0, s19
bl _write_float
.data
_string_const_4: .asciz "\n"
.align 3
.text
ldr x19, =_string_const_4
mov x0, x19
bl _write_str
.data
_integer_const_5: .word 0
.align 3
.text
ldr w19, _integer_const_5
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
_AR_SIZE_6: .word 88
.align 3
.text
ldr w19, _AR_SIZE_6
sub sp, sp, w19
.data
_integer_const_7: .word 16
.align 3
.text
ldr w20, _integer_const_7
sub sp, sp, w20
.data
_integer_const_8: .word 20
.align 3
.text
ldr w20, _integer_const_8
.data
_const_9: .word 92
.align 3
.text
ldrsw x21, _const_9
sub x21, x29, x21
str w20, [x21, #0]
.data
_integer_const_10: .word 200
.align 3
.text
ldr w20, _integer_const_10
.data
_const_11: .word 96
.align 3
.text
ldrsw x21, _const_11
sub x21, x29, x21
str w20, [x21, #0]
.data
_float_const_12: .float 3.213000
.align 3
.text
ldr s20, _float_const_12
.data
_const_13: .word 100
.align 3
.text
ldrsw x21, _const_13
sub x21, x29, x21
fcvtzs w20, s20
str w20, [x21, #0]
.data
_float_const_14: .float 0.123400
.align 3
.text
ldr s20, _float_const_14
.data
_const_15: .word 104
.align 3
.text
ldrsw x21, _const_15
sub x21, x29, x21
str s20, [x21, #0]
.data
_const_16: .word 92
.align 3
.text
ldrsw x20, _const_16
sub x20, x29, x20
ldr w20, [x20, #0]
mov w0, w20
bl _write_int
.data
_string_const_17: .asciz "\n"
.align 3
.text
ldr x20, =_string_const_17
mov x0, x20
bl _write_str
.data
_const_18: .word 96
.align 3
.text
ldrsw x20, _const_18
sub x20, x29, x20
ldr w20, [x20, #0]
mov w0, w20
bl _write_int
.data
_string_const_19: .asciz "\n"
.align 3
.text
ldr x20, =_string_const_19
mov x0, x20
bl _write_str
.data
_const_20: .word 104
.align 3
.text
ldrsw x20, _const_20
sub x20, x29, x20
ldr s20, [x20, #0]
fmov s0, s20
bl _write_float
.data
_string_const_21: .asciz "\n"
.align 3
.text
ldr x20, =_string_const_21
mov x0, x20
bl _write_str
.data
_const_22: .word 100
.align 3
.text
ldrsw x20, _const_22
sub x20, x29, x20
ldr w20, [x20, #0]
mov w0, w20
bl _write_int
.data
_string_const_23: .asciz "\n"
.align 3
.text
ldr x20, =_string_const_23
mov x0, x20
bl _write_str
.data
_integer_const_24: .word 16
.align 3
.text
ldr w20, _integer_const_24
sub sp, sp, w20
.data
_const_25: .word 92
.align 3
.text
ldrsw x20, _const_25
sub x20, x29, x20
ldr w20, [x20, #0]
str w20, [sp, #0]
sub sp, sp, #8
.data
_const_26: .word 96
.align 3
.text
ldrsw x20, _const_26
sub x20, x29, x20
ldr w20, [x20, #0]
add sp, sp, #8
ldr w21, [sp, #0]
add w21, w21, w20
.data
_const_27: .word 120
.align 3
.text
ldrsw x20, _const_27
sub x20, x29, x20
str w21, [x20, #0]
.data
_const_28: .word 100
.align 3
.text
ldrsw x20, _const_28
sub x20, x29, x20
ldr w20, [x20, #0]
str w20, [sp, #0]
sub sp, sp, #8
.data
_const_29: .word 104
.align 3
.text
ldrsw x20, _const_29
sub x20, x29, x20
ldr s20, [x20, #0]
add sp, sp, #8
ldr w21, [sp, #0]
scvtf s21, w21
fadd s21, s21, s20
.data
_const_30: .word 112
.align 3
.text
ldrsw x20, _const_30
sub x20, x29, x20
str s21, [x20, #0]
bl _start_f
.text
ldr w20, _integer_const_24
add sp, sp, w20
.data
_integer_const_31: .word 16
.align 3
.text
ldr w20, _integer_const_31
sub sp, sp, w20
.data
_const_32: .word 92
.align 3
.text
ldrsw x20, _const_32
sub x20, x29, x20
ldr w20, [x20, #0]
str w20, [sp, #0]
sub sp, sp, #8
.data
_const_33: .word 96
.align 3
.text
ldrsw x20, _const_33
sub x20, x29, x20
ldr w20, [x20, #0]
add sp, sp, #8
ldr w21, [sp, #0]
add w21, w21, w20
.data
_const_34: .word 120
.align 3
.text
ldrsw x20, _const_34
sub x20, x29, x20
str w21, [x20, #0]
.data
_const_35: .word 100
.align 3
.text
ldrsw x20, _const_35
sub x20, x29, x20
ldr w20, [x20, #0]
str w20, [sp, #0]
sub sp, sp, #8
.data
_const_36: .word 104
.align 3
.text
ldrsw x20, _const_36
sub x20, x29, x20
ldr s20, [x20, #0]
add sp, sp, #8
ldr w21, [sp, #0]
scvtf s21, w21
fadd s21, s21, s20
.data
_const_37: .word 112
.align 3
.text
ldrsw x20, _const_37
sub x20, x29, x20
str s21, [x20, #0]
bl _start_f
.text
ldr w20, _integer_const_31
add sp, sp, w20
ldr w20, _glob
mov w0, w20
bl _write_int
.data
_string_const_38: .asciz "\n"
.align 3
.text
ldr x20, =_string_const_38
mov x0, x20
bl _write_str
ldr s20, _flob
fmov s0, s20
bl _write_float
.data
_string_const_39: .asciz "\n"
.align 3
.text
ldr x20, =_string_const_39
mov x0, x20
bl _write_str
.data
_integer_const_40: .word 16
.align 3
.text
ldr w20, _integer_const_40
sub sp, sp, w20
.data
_const_41: .word 92
.align 3
.text
ldrsw x20, _const_41
sub x20, x29, x20
ldr w20, [x20, #0]
str w20, [sp, #0]
sub sp, sp, #8
.data
_const_42: .word 96
.align 3
.text
ldrsw x20, _const_42
sub x20, x29, x20
ldr w20, [x20, #0]
add sp, sp, #8
ldr w21, [sp, #0]
add w21, w21, w20
.data
_const_43: .word 120
.align 3
.text
ldrsw x20, _const_43
sub x20, x29, x20
str w21, [x20, #0]
.data
_const_44: .word 100
.align 3
.text
ldrsw x20, _const_44
sub x20, x29, x20
ldr w20, [x20, #0]
str w20, [sp, #0]
sub sp, sp, #8
.data
_const_45: .word 104
.align 3
.text
ldrsw x20, _const_45
sub x20, x29, x20
ldr s20, [x20, #0]
add sp, sp, #8
ldr w21, [sp, #0]
scvtf s21, w21
fadd s21, s21, s20
.data
_const_46: .word 112
.align 3
.text
ldrsw x20, _const_46
sub x20, x29, x20
str s21, [x20, #0]
bl _start_f
.text
ldr w20, _integer_const_40
add sp, sp, w20
.data
_integer_const_47: .word 16
.align 3
.text
ldr w20, _integer_const_47
sub sp, sp, w20
.data
_const_48: .word 92
.align 3
.text
ldrsw x20, _const_48
sub x20, x29, x20
ldr w20, [x20, #0]
str w20, [sp, #0]
sub sp, sp, #8
.data
_const_49: .word 96
.align 3
.text
ldrsw x20, _const_49
sub x20, x29, x20
ldr w20, [x20, #0]
add sp, sp, #8
ldr w21, [sp, #0]
add w21, w21, w20
.data
_const_50: .word 120
.align 3
.text
ldrsw x20, _const_50
sub x20, x29, x20
str w21, [x20, #0]
.data
_const_51: .word 100
.align 3
.text
ldrsw x20, _const_51
sub x20, x29, x20
ldr w20, [x20, #0]
str w20, [sp, #0]
sub sp, sp, #8
.data
_const_52: .word 104
.align 3
.text
ldrsw x20, _const_52
sub x20, x29, x20
ldr s20, [x20, #0]
add sp, sp, #8
ldr w21, [sp, #0]
scvtf s21, w21
fadd s21, s21, s20
.data
_const_53: .word 112
.align 3
.text
ldrsw x20, _const_53
sub x20, x29, x20
str s21, [x20, #0]
bl _start_f
.text
ldr w20, _integer_const_47
add sp, sp, w20
.data
_integer_const_54: .word 0
.align 3
.text
ldr w20, _integer_const_54
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
_integer_const_55: .word 123
.align 3
.text
ldr w21, _integer_const_55
ldr x22, =_glob
str w21, [x22, #0]
.data
_float_const_56: .float 1241.121000
.align 3
.text
ldr s21, _float_const_56
ldr x22, =_flob
str s21, [x22, #0]
b _global_init_back
