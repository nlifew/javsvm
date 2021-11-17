


#include "runner.h"


#include "vm/jvm.h"
#include "vm/jstack.h"

#include <functional> /* std::equal_to, std::less */

#include "opsdef.h"
#include "opslib.h"

/**
 * 真正的代码执行逻辑
 */ 
jvalue runner::run_java(jmethod *m, jref _this, args_wrapper &args)
{
    jvalue result = {0};

    // 创建栈帧并初始化
    jstack &stack = jvm::get().env().stack();
    jstack_frame &frame = stack.push(m);
    
    
    jclass_attr_code &code = *(m->entrance.code_func);
    jclass_const_pool &constant_pool = m->clazz->class_file->constant_pool;
 
    // 将参数压进局部变量表
    memcpy(frame.variable_table, args.m_orig, m->args_slot * sizeof(jstack_frame::var_t));

    // 解释执行
    while (frame.pc < code.code_length) {
        const int op = code.code[frame.pc ++];
        switch (op) {
            case nop:
                break;
            case aconst_null:
                push_param<jref>(frame, 0);
                break;
            case iconst_m1:
            case iconst_0:
            case iconst_1:
            case iconst_2:
            case iconst_3:
            case iconst_4:
            case iconst_5:
                push_param<jint>(frame, op - 3);
                break;
            case lconst_0:
            case lconst_1:
                push_param<jlong>(frame, op - 9);
                break;
            case fconst_0:
            case fconst_1:
            case fconst_2:
                push_param<jfloat>(frame, op - 11);
                break;
            case dconst_0:
            case dconst_1:
                push_param<jdouble>(frame, op - 14);
                break;
            case bipush: {
                u1 val = code.code[frame.pc ++];
                push_param(frame, val);
                break;
            }
            case sipush: {
                u2 val = code.code[frame.pc ++] << 8;
                val |= code.code[frame.pc ++];
                push_param(frame, val);
                break;
            }
            case ldc: {
                int idx = code.code[frame.pc ++];
                do_ldc(constant_pool, idx, 1, frame);
                break;
            }
            case ldc_w: {
                int idx = code.code[frame.pc ++] << 8;
                idx |= code.code[frame.pc ++];
                do_ldc(constant_pool, idx, 1, frame);
                break;
            }
            case ldc2_w: {
                int idx = code.code[frame.pc ++] << 8;
                idx |= code.code[frame.pc ++];
                do_ldc(constant_pool, idx, 2, frame);
                break;
            }
            case iload: 
            case fload: 
            case aload: {
                int idx = code.code[frame.pc ++];
                frame.operand_stack[0] = frame.variable_table[idx];
                frame.operand_stack += 1;
                break;
            }
            case lload:
            case dload: {
                int idx = code.code[frame.pc ++];
                frame.operand_stack[0] = frame.variable_table[idx];
                frame.operand_stack[1] = frame.variable_table[idx + 1];
                frame.operand_stack += 2;
                break;
            }
            case iload_0:
            case iload_1:
            case iload_2:
            case iload_3:
                frame.operand_stack[0] = frame.variable_table[op - 26];
                frame.operand_stack += 1;
                break;
            case lload_0:
            case lload_1:
            case lload_2:
            case lload_3:
                frame.operand_stack[0] = frame.variable_table[op - 30];
                frame.operand_stack[1] = frame.variable_table[op - 29];
                frame.operand_stack += 2;
                break;
            case fload_0:
            case fload_1:
            case fload_2:
            case fload_3:
                frame.operand_stack[0] = frame.variable_table[op - 34];
                frame.operand_stack += 1;
                break;
            case dload_0:
            case dload_1:
            case dload_2:
            case dload_3:
                frame.operand_stack[0] = frame.variable_table[op - 38];
                frame.operand_stack[1] = frame.variable_table[op - 37];
                frame.operand_stack += 2;
                break;
            case aload_0:
            case aload_1:
            case aload_2:
            case aload_3:
                frame.operand_stack[0] = frame.variable_table[op - 42];
                frame.operand_stack += 1;
                break;
            case iaload:
            case laload:
            case faload:
            case daload:
            case aaload:
            case baload:
            case caload:
            case saload:
                // todo: 
                break;
            case istore: 
            case fstore: 
            case astore: {
                int idx = code.code[frame.pc ++];
                frame.variable_table[idx] = frame.operand_stack[-1];
                frame.operand_stack -= 1;
                break;
            }
            case lstore: 
            case dstore: {
                int idx = code.code[frame.pc ++];
                frame.variable_table[idx] = frame.operand_stack[-2];
                frame.variable_table[idx + 1] = frame.operand_stack[-1];
                frame.operand_stack -= 2;
                break;
            }
            case istore_0:
            case istore_1:
            case istore_2:
            case istore_3:
                frame.variable_table[op - 59] = frame.operand_stack[-1];
                frame.operand_stack -= 1;
                break;
            case lstore_0:
            case lstore_1:
            case lstore_2:
            case lstore_3:
                frame.variable_table[op - 63] = frame.operand_stack[-2];
                frame.variable_table[op - 62] = frame.operand_stack[-1];
                frame.operand_stack -= 2;
                break;
            case fstore_0:
            case fstore_1:
            case fstore_2:
            case fstore_3:
                frame.variable_table[op - 67] = frame.operand_stack[-1];
                frame.operand_stack -= 1;
                break;
            case dstore_0:
            case dstore_1:
            case dstore_2:
            case dstore_3:
                frame.variable_table[op - 71] = frame.operand_stack[-2];
                frame.variable_table[op - 70] = frame.operand_stack[-1];
                frame.operand_stack -= 2;
                break;
            case astore_0:
            case astore_1:
            case astore_2:
            case astore_3:
                frame.variable_table[op - 75] = frame.operand_stack[-1];
                frame.operand_stack -= 1;
                break;
            case iastore:
            case lastore:
            case fastore:
            case dastore:
            case aastore:
            case bastore:
            case castore:
            case sastore:
                // todo
                break;
            case pop:
            case pop2:
                frame.operand_stack -= op - 86;
                break;
            case dup_x2:
                frame.operand_stack[2] = frame.operand_stack[-1];
                [[fallthrough]];
            case dup_x1:
                frame.operand_stack[1] = frame.operand_stack[-1];
                [[fallthrough]];
            case dup:
                frame.operand_stack[0] = frame.operand_stack[-1];
                frame.operand_stack += op - 88;
                break;
            case dup2_x2:
                frame.operand_stack[4] = frame.operand_stack[-2];
                frame.operand_stack[5] = frame.operand_stack[-1];
                [[fallthrough]];
            case dup2_x1:
                frame.operand_stack[2] = frame.operand_stack[-2];
                frame.operand_stack[3] = frame.operand_stack[-1];
                [[fallthrough]];
            case dup2:
                frame.operand_stack[0] = frame.operand_stack[-2];
                frame.operand_stack[1] = frame.operand_stack[-1];
                frame.operand_stack += (op - 91) << 1;
                break;
            case swap: {
                jstack_frame::operand_t _swap = frame.operand_stack[-1];
                frame.operand_stack[-1] = frame.operand_stack[-2];
                frame.operand_stack[-2] = _swap;
                break;
            }
            case iadd: invoke(addT<jint>, frame); break;
            case ladd: invoke(addT<jlong>, frame); break;
            case fadd: invoke(addT<jfloat>, frame); break;
            case dadd: invoke(addT<jdouble>, frame); break;

            case isub: invoke(subT<jint>, frame); break;
            case lsub: invoke(subT<jlong>, frame); break;
            case fsub: invoke(subT<jfloat>, frame); break;
            case dsub: invoke(subT<jdouble>, frame); break;

            case imul: invoke(mulT<jint>, frame); break;
            case lmul: invoke(mulT<jlong>, frame); break;
            case fmul: invoke(mulT<jfloat>, frame); break;
            case dmul: invoke(mulT<jdouble>, frame); break;

            case idiv: invoke(divT<jint>, frame); break;
            case _ldiv: invoke(divT<jlong>, frame); break;
            case fdiv: invoke(divT<jfloat>, frame); break;
            case ddiv: invoke(divT<jdouble>, frame); break;

            case irem: invoke(remT<jint>, frame); break;
            case lrem: invoke(remT<jlong>, frame); break;
            case frem: invoke(remT<jfloat>, frame); break;
            case drem: invoke(remT<jdouble>, frame); break;

            case ineg: invoke(negT<jint>, frame); break;
            case lneg: invoke(negT<jlong>, frame); break;
            case fneg: invoke(negT<jfloat>, frame); break;
            case dneg: invoke(negT<jdouble>, frame); break;

            case ishl: invoke(shlT<jint>, frame); break;
            case lshl: invoke(shlT<jlong>, frame); break;
            case ishr: invoke(shrT<jint>, frame); break;
            case lshr: invoke(shrT<jlong>, frame); break;
            case iushr: invoke(ushrT<jint, uint32_t>, frame); break;
            case lushr: invoke(ushrT<jlong, uint64_t>, frame); break;

            case iand: invoke(andT<jint>, frame); break;
            case land: invoke(andT<jlong>, frame); break;
            case ior: invoke(orT<jint>, frame); break;
            case lor: invoke(orT<jlong>, frame); break;
            case ixor: invoke(xorT<jint>, frame); break;
            case lxor: invoke(xorT<jlong>, frame); break;
            case iinc: invoke(incT<jint>, frame); break;

            case i2l: invoke(i2T<jint, jlong>, frame); break;
            case i2f: invoke(i2T<jint, jfloat>, frame); break;
            case i2d: invoke(i2T<jint, jdouble>, frame); break;

            case l2i: invoke(i2T<jlong, jint>, frame); break;
            case l2f: invoke(i2T<jlong, jfloat>, frame); break;
            case l2d: invoke(i2T<jlong, jdouble>, frame); break;

            case f2i: invoke(i2T<jfloat, jint>, frame); break;
            case f2l: invoke(i2T<jfloat, jlong>, frame); break;
            case f2d: invoke(i2T<jfloat, jdouble>, frame); break;

            case d2i: invoke(i2T<jdouble, jint>, frame); break;
            case d2l: invoke(i2T<jdouble, jlong>, frame); break;
            case d2f: invoke(i2T<jdouble, jfloat>, frame); break;

            case i2b: invoke(i2T<jint, jbyte>, frame); break;
            case i2c: invoke(i2T<jint, jchar>, frame); break;
            case i2s: invoke(i2T<jint, jshort>, frame); break;

            case lcmp: invoke(lcmpT, frame); break;
            case fcmpl: invoke(fcmplT, frame); break;
            case fcmpg: invoke(fcmpgT, frame); break;
            case dcmpl: invoke(dcmplT, frame); break;
            case dcmpg: invoke(dcmpgT, frame); break;

            case ifeq: ifT<jint, std::equal_to<jint>>(frame, code); break;
            case ifne: ifT<jint, std::not_equal_to<jint>>(frame, code); break;
            case iflt: ifT<jint, std::less<jint>>(frame, code); break;
            case ifge: ifT<jint, std::greater<jint>>(frame, code); break;
            case ifgt: ifT<jint, std::greater_equal<jint>>(frame, code); break;
            case ifle: ifT<jint, std::less_equal<jint>>(frame, code); break;
            case ifnull: ifT<jref, ref_equ>(frame, code); break;
            case ifnonnull: ifT<jref, ref_not_equ>(frame, code); break;

            case if_icmpeq: ifcmpT<jint, std::equal_to<jint>>(frame, code); break;
            case if_icmpne: ifcmpT<jint, std::not_equal_to<jint>>(frame, code); break;
            case if_icmplt: ifcmpT<jint, std::less<jint>>(frame, code); break;
            case if_icmpge: ifcmpT<jint, std::greater_equal<jint>>(frame, code); break;
            case if_icmpgt: ifcmpT<jint, std::greater<jint>>(frame, code); break;
            case if_icmple: ifcmpT<jint, std::less_equal<jint>>(frame, code); break;
            case if_acmpeq: ifcmpT<jref, ref_equ>(frame, code); break;
            case if_acmpne: ifcmpT<jref, ref_not_equ>(frame, code); break;

            case _goto: ifT<void, void>(frame, code); break;
            case jsr:
            case ret:
                break;
            case tableswitch: switch_table(frame, code); break;
            case lookupswitch: lookup_table(frame, code); break;

            case ireturn:
                result.i = pop_param<jint>(frame);
                goto finally;
            case lreturn:
                result.j = pop_param<jlong>(frame);
                goto finally;
            case freturn:
                result.f = pop_param<jfloat>(frame);
                goto finally;
            case dreturn:
                result.d = pop_param<jdouble>(frame);
                goto finally;
            case areturn:
                result.l = pop_param<jref>(frame);
                goto finally;
            case _return:
                goto finally;

            case getstatic: {
                u1 idx = code.code[frame.pc ++];
                jfield *field = get_field(idx, constant_pool);
                jvalue val = field->get(nullptr);
                push_jvalue(frame, val, field->slot_num);
                break;
            }
            case putstatic: {
                u1 idx = code.code[frame.pc ++];
                jfield *field = get_field(idx, constant_pool);
                jvalue val = pop_jvalue(frame, field->slot_num);
                field->set(nullptr, val);
                break;
            }
            case getfield: {
                u1 idx = code.code[frame.pc ++];
                jfield *field = get_field(idx, constant_pool);

                jref obj = pop_param<jref>(frame);
                jvalue val = field->get(obj);
                push_jvalue(frame, val, field->slot_num);
                break;
            }
            case putfield: {
                u1 idx = code.code[frame.pc ++];
                jfield *field = get_field(idx, constant_pool);

                jvalue val = pop_jvalue(frame, field->slot_num);
                jref obj = pop_param<jref>(frame);
                field->set(obj, val);
                break;
            }

            case invokespecial: {
                int idx = code.code[frame.pc ++];
                jmethod *m = get_method(idx, constant_pool);
                frame.operand_stack -= m->args_slot;
                args_wrapper args(frame.operand_stack);
                jref ref = args.next<jref>();
                jvalue val = m->invoke_special(ref, args);
                push_jvalue(frame, val, m->return_slot);
                break;
            }
            case invokevirtual:
            case invokeinterface: {
                int idx = code.code[frame.pc ++];
                jmethod *m = get_method(idx, constant_pool);
                frame.operand_stack -= m->args_slot;
                args_wrapper args(frame.operand_stack);
                jref ref = args.next<jref>();
                jvalue val = m->invoke_virtual(ref, args);
                push_jvalue(frame, val, m->return_slot);
                break;
            }
            case invokestatic: {
                int idx = code.code[frame.pc ++];
                jmethod *m = get_method(idx, constant_pool);
                frame.operand_stack -= m->args_slot;
                args_wrapper args(frame.operand_stack);
                jvalue val = m->invoke_static(args);
                push_jvalue(frame, val, m->return_slot);
                break;
            }
            case invokedynamic:
                // todo:
                break;

            case _new: {
                int idx = code.code[frame.pc ++];
                jclass *cls = get_class(idx, constant_pool);
                jref ref = cls->new_instance();
                push_param<jref>(frame, ref);
                break;
            }
            case newarray:
            case anewarray:
            case arraylength:
            case athrow: 
            case checkcast:
            case instanceof:
                // todo
                break;

            case monitorenter: {
                jref ref = pop_param<jref>(frame);
                jobject::lock(ref);
                break;
            }
            case monitorexit: {
                jref ref = pop_param<jref>(frame);
                jobject::unlock(ref);
                break;
            }

            case wide:
            case multianewarray:
            case goto_w:
            case jsr_w:
                // todo:
                break;
            default:
                LOGE("unsuppoert byte code %#02x\n", op);
                exit(1);
                break;
        }
    }
    goto finally;

_catch:
    // 

finally:
    return result;
}

/**
 * native 函数执行逻辑
 */ 

jvalue runner::run_jni(jmethod *m, jref _this, args_wrapper &args)
{
    // todo:
    jvalue v = {0};
    return v;
}