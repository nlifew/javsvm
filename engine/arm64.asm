.section	__TEXT,__text,regular,pure_instructions

;函数原型 extern "C" int64_t calljni64(
;	const void *addr,			/* 函数地址，由 load_library() 获取得到，不能是 nullptr */
;   int return_type,			/* 返回类型，0 表示返回 int64，非 0 返回 double64.  */
;   const int64_t* integers,	/* 固定长度为 8 的数组，数组内的值将被写入到 x0-x7 通用寄存器 */
;   const int64_t* floats,		/* 固定长度为 16 的数组，数组内的值将被写入到 d0-d15 浮点数寄存器 */
;   int stack_len,				/* 表示要压进栈的参数大小，单位是字节，为非负的整数。需要调用者自己对齐到 16 字节 */
;   const void *stack			/* 要压进栈的参数的起始地址。高地址会被先压进栈。 */
;);
;
.globl	_calljni64                         ; -- Begin function call64
.p2align	2
; 6 个参数，每个 8 字节，再加上 x29 和 x30，共 64 字节，刚好 16 字节对齐
_calljni64:
	sub	sp, sp, #64                     ; =64
    stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48                    ; =48

	; 参数压栈，共 6 个参数，48 个字节
	stur	x0,	[x29, #-8]
	stur	w1,	[x29, #-12]
	stur	x2,	[x29, #-24]
	stur	x3,	[x29, #-32]
	stur	w4,	[x29, #-36]
	stur	x5,	[x29, #-48]

	; 保存浮点数寄存器。我们最多支持 16 个 double64 参数
	; 即 128 个字节
	sub sp, sp, #128
    stp	d0, d1, [sp, #112]
    stp	d2, d3, [sp, #96]
    stp	d4, d5, [sp, #80]
    stp	d6, d7, [sp, #64]
    stp	d8, d9, [sp, #48]
    stp	d10, d11, [sp, #32]
    stp	d12, d13, [sp, #16]
    stp	d14, d15, [sp]

	; 我们使用 x19-x26 共 8 个寄存器作为临时变量。
	; 这几个寄存器是 callee 保存状态的，所以要备份到栈中
	; 其中 x26 寄存器用来保存 sp 的地址，在函数调用结束后恢复栈
	sub sp, sp, #64
	stp x19, x20, [sp, #48]
	stp x21, x22, [sp, #32]
	stp x23, x24, [sp, #16]
	stp x25, x26, [sp]

	; 保存栈指针
	mov	x26, sp

	; 如果参数 stack_len (w4) 不为 0，需要拓展栈
	sub sp, sp, x4
	; 循环，把参数压进栈
	mov w19, #0
	mov x22, sp
LPushStackLoop:
	cmp w19, w4
	b.ge LPushStackDone
	; 每次复制 8 个字节
	ldr x21, [x5, x19]
	str x21, [x22]
	add w19, w19, #8
	add x22, x22, #8
	b LPushStackLoop
LPushStackDone:
	; 栈拓展完之后，设置浮点数寄存器
	ldp d0, d1, [x3]
	ldp d2, d3, [x3, #16]
	ldp d4, d5, [x3, #32]
	ldp d6, d7, [x3, #48]
	ldp d8, d9, [x3, #64]
	ldp d10, d11, [x3, #80]
	ldp d12, d13, [x3, #96]
	ldp d14, d15, [x3, #112]

	; 处理 x0-x7 寄存器参数
	mov x19, x2
	ldp x0, x1, [x19]
	ldp x2, x3, [x19, #16]
	ldp x4, x5, [x19, #32]
	ldp x6, x7, [x19, #48]

	; 此时所有的参数都已经准备好，跳转到指定地址
	ldur x19, [x29, #-8]
	blr x19

	; 子函数调用完恢复栈指针
	mov	sp,	x26

	; 准备写入返回值。返回值类型 ret_type 保存在 [x29, #-12] 上
	ldur w19, [x29, #-12]
	cmp w19, #0
	b.eq LReturnDone
	fmov x0, d0
LReturnDone:
	; 恢复临时寄存器 x19-x26
	ldp x19, x20, [sp, #48]
	ldp x21, x22, [sp, #32]
	ldp x23, x24, [sp, #16]
	ldp x25, x26, [sp]
	add sp, sp, #64

	; 恢复浮点数寄存器
	ldp d0, d1, [sp, #112]
	ldp d2, d3, [sp, #96]
	ldp d4, d5, [sp, #80]
	ldp d6, d7, [sp, #64]
	ldp d8, d9, [sp, #48]
	ldp d10, d11, [sp, #32]
	ldp d12, d13, [sp, #16]
	ldp d14, d15, [sp]
	add sp, sp, #128

	; 恢复栈指针和 x29, x30，返回
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	add	sp, sp, #64                     ; =64
	ret
										; -- End function
