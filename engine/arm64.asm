.section	__TEXT,__text,regular,pure_instructions

; 函数原型 extern "C" void calljni64(
; void *addr,               /* 函数地址，由 load_library() 获取得到，不能是 nullptr */
; int ret_type,             /* 返回类型，0 表示无返回值，1 为 int64，2 是 float32, 3 是 double64 */
; void *ret,                /* 返回值的写入地址，不能为 nullptr */
; const int64_t *x,         /* 固定长度为 8 的数组，数组内的值将被写入到 x0-x7 通用寄存器 */
; int stack_len,            /* 表示要压进栈的参数大小，单位是字节，为非负的整数。需要调用者自己对齐 */
; const void* stack,        /* 要压进栈的参数的起始地址。高地址会被先压进栈。 */
; int fargc,                /* 传递参数时使用的浮点数寄存器数量。和标准 arm64 不同，最多使用 16 个 */
; const int *fargctl,       /* 控制 fargv[i] 中的数据怎样写入浮点数寄存器。为 0 时使用低 32 位(s0-s15)，1 时使用低 64 位(d0-d15)。长度为 fargc */
; const uint64_t *fargv,    /* 长度为 fargc 的数组，fargv[i] 中的数据将写入 fargc_ctl[i] 寄存器中 */
;)
.globl	_calljni64                       ; -- Begin function calljni64
.p2align	2
_calljni64:                             ; @calljni64
	sub	sp, sp, #96                     ; =96
	stp	x29, x30, [sp, #80]             ; 16-byte Folded Spill
	add	x29, sp, #80                    ; =80
	ldr	x8, [x29, #16]
	stur	x0, [x29, #-8]
	stur	w1, [x29, #-12]
	stur	x2, [x29, #-24]
	stur	x3, [x29, #-32]
	stur	w4, [x29, #-36]
	stur    x5, [x29, #-48] ; str	x5, [sp, #32]
	stur    w6, [x29, #-52] ; str	w6, [sp, #28]
	stur    x7, [x29, #-64] ; str	x7, [sp, #16]
	stur    x8, [x29, #-72] ; str	x8, [sp, #8]

	; 到这里为止，所有 9 个参数已经保存在 x0-x8 寄存器中
	; 我们使用 x19-x26 共 8 个寄存器作为临时变量，先保存到栈中
	; 其中 x26 寄存器用来保存 sp 的地址，在函数调用结束后恢复栈
	sub sp, sp, #64
	stp x19, x20, [sp, #48]
	stp x21, x22, [sp, #32]
	stp x23, x24, [sp, #16]
	stp x25, x26, [sp]

	; 判断是否使用了浮点数(fargc)。如果使用，将 d0-d15 共 16 个寄存器压栈（避免分支跳转）
	cmp w6, #0
	b.le LPushFloatRegisterDone
	sub sp, sp, #128
	stp d0, d1, [sp, #112]
	stp d2, d3, [sp, #96]
	stp d4, d5, [sp, #80]
	stp d6, d7, [sp, #64]
	stp d8, d9, [sp, #48]
	stp d10, d11, [sp, #32]
	stp d12, d13, [sp, #16]
	stp d14, d15, [sp]
LPushFloatRegisterDone:
	; 当前栈指针保存到 x26
	mov x26, sp

	; 如果 stack_len 大于 0，需要拓展栈
	cmp w4, #0
	b.le LPushStackDone
	sub sp, sp, x4
	; 循环，把参数压进栈
	mov w19, #0
	mov x22, sp
LPushStackLoop:
	cmp w19, w4
	b.ge LPushStackDone
	; 判断剩余需要复制的大小是否大于 8。如果大于等于 8，复制 8 个字节，否则复制 1 个字节
	; 复制过程中使用的临时变量为 x21
	; 按理说 sp 指针是 16 位对齐的，因此不可能走到 LPushOneByteToStack，但还是留着吧
	subs w20, w4, w19
	cmp w20, #8
	b.lt LPushOneByteToStack
	ldr x21, [x5, x19]
	str x21, [x22]
	add w19, w19, #8
	add x22, x22, #8
	b LPushStackLoop
LPushOneByteToStack:
	ldrb w21, [x5, x19]
	strb w21, [x22]
	add w19, w19, #1
	add x22, x22, #1
	b LPushStackLoop
LPushStackDone:

	; 处理浮点数参数
	mov w19, #0
LCheckFloatRegister0:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d0
	fmov s0, w21
	add w19, w19, #1
	b LCheckFloatRegister1
LStoreFloat64_d0:
	fmov d0, x21
	add w19, w19, #1
LCheckFloatRegister1:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d1
	fmov s1, w21
	add w19, w19, #1
	b LCheckFloatRegister2
LStoreFloat64_d1:
	fmov d1, x21
	add w19, w19, #1
LCheckFloatRegister2:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d2
	fmov s2, w21
	add w19, w19, #1
	b LCheckFloatRegister3
LStoreFloat64_d2:
	fmov d2, x21
	add w19, w19, #1
LCheckFloatRegister3:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d3
	fmov s3, w21
	add w19, w19, #1
	b LCheckFloatRegister4
LStoreFloat64_d3:
	fmov d3, x21
	add w19, w19, #1
LCheckFloatRegister4:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d4
	fmov s4, w21
	add w19, w19, #1
	b LCheckFloatRegister5
LStoreFloat64_d4:
	fmov d4, x21
	add w19, w19, #1
LCheckFloatRegister5:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d5
	fmov s5, w21
	add w19, w19, #1
	b LCheckFloatRegister6
LStoreFloat64_d5:
	fmov d5, x21
	add w19, w19, #1
LCheckFloatRegister6:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d6
	fmov s6, w21
	add w19, w19, #1
	b LCheckFloatRegister7
LStoreFloat64_d6:
	fmov d6, x21
	add w19, w19, #1
LCheckFloatRegister7:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d7
	fmov s7, w21
	add w19, w19, #1
	b LCheckFloatRegister8
LStoreFloat64_d7:
	fmov d7, x21
	add w19, w19, #1
LCheckFloatRegister8:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d8
	fmov s8, w21
	add w19, w19, #1
	b LCheckFloatRegister9
LStoreFloat64_d8:
	fmov d8, x21
	add w19, w19, #1
LCheckFloatRegister9:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d9
	fmov s9, w21
	add w19, w19, #1
	b LCheckFloatRegister10
LStoreFloat64_d9:
	fmov d9, x21
	add w19, w19, #1
LCheckFloatRegister10:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d10
	fmov s10, w21
	add w19, w19, #1
	b LCheckFloatRegister11
LStoreFloat64_d10:
	fmov d10, x21
	add w19, w19, #1
LCheckFloatRegister11:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d11
	fmov s11, w21
	add w19, w19, #1
	b LCheckFloatRegister12
LStoreFloat64_d11:
	fmov d11, x21
	add w19, w19, #1
LCheckFloatRegister12:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d12
	fmov s12, w21
	add w19, w19, #1
	b LCheckFloatRegister13
LStoreFloat64_d12:
	fmov d12, x21
	add w19, w19, #1
LCheckFloatRegister13:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d13
	fmov s13, w21
	add w19, w19, #1
	b LCheckFloatRegister14
LStoreFloat64_d13:
	fmov d13, x21
	add w19, w19, #1
LCheckFloatRegister14:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d14
	fmov s14, w21
	add w19, w19, #1
	b LCheckFloatRegister15
LStoreFloat64_d14:
	fmov d14, x21
	add w19, w19, #1
LCheckFloatRegister15:
	cmp w19, w6
	b.ge LStoreFloatRegisterDone
	lsl x22, x19, #2
	ldr w20, [x7, x22] ; fargctl[i]
	lsl x22, x19, #3
	ldr x21, [x8, x22] ; fargv[i]
	cmp w20, #0
	b.ne LStoreFloat64_d15
	fmov s15, w21
	add w19, w19, #1
	b LStoreFloatRegisterDone
LStoreFloat64_d15:
	fmov d15, x21
	add w19, w19, #1
LStoreFloatRegisterDone:

	; 处理 x0-x7 寄存器参数
	mov x19, x3
	ldr x0, [x19]
	ldr x1, [x19, #8]
	ldr x2, [x19, #16]
	ldr x3, [x19, #24]
	ldr x4, [x19, #32]
	ldr x5, [x19, #40]
	ldr x6, [x19, #48]
	ldr x7, [x19, #56]

	; 此时所有的参数都已经准备好，跳转到指定地址
	ldr	x19, [x29, #-8]
	blr x19

	; 子函数调用完恢复栈指针
	mov sp, x26

	; 准备写入返回值。返回值类型 ret_type 保存在 [x29, #-12] 中
	ldur w19, [x29, #-12]
	ldur x20, [x29, #-24]
	;b LCheckReturnInt64
LCheckReturnInt64:
	;ldur w19, [x29, #-12]
	cmp w19, #1
	b.ne LCheckReturnFloat32
	;ldur x20, [x29, #-24]
	str x0, [x20]
	;b LReturnDone
LCheckReturnFloat32:
	;ldur w19, [x29, #-12]
	cmp w19, #2
	b.ne LCheckReturnDouble64
	;ldur x20, [x29, #-24]
	str s0, [x20]
	;b LReturnDone
LCheckReturnDouble64:
	;ldur w19, [x29, #-12]
	cmp w19, #3
	b.ne LReturnDone
	;ldur x20, [x29, #-24]
	str d0, [x20]
	; b LReturnDone
LReturnDone:
	; 恢复浮点数寄存器
	ldr w19, [x29, #-52]
	cmp w19, #0
	b.le LPopFloatRegisterDone
	ldp d0, d1, [sp, #112]
	ldp d2, d3, [sp, #96]
	ldp d4, d5, [sp, #80]
	ldp d6, d7, [sp, #64]
	ldp d8, d9, [sp, #48]
	ldp d10, d11, [sp, #32]
	ldp d12, d13, [sp, #16]
	ldp d14, d15, [sp]
	add sp, sp, #128
LPopFloatRegisterDone:
	; 恢复临时寄存器 x19-x26
	ldp x19, x20, [sp, #48]
	ldp x21, x22, [sp, #32]
	ldp x23, x24, [sp, #16]
	ldp x25, x26, [sp]
	add sp, sp, #64

	; 恢复栈指针和 x29, x30，返回
	ldp	x29, x30, [sp, #80]             ; 16-byte Folded Reload
	add	sp, sp, #96                     ; =96
	ret
										; -- End function