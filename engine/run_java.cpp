


#include "engine.h"



#include <functional> /* std::equal_to, std::less */

#include "opsdef.h"
#include "opslib.h"

/**
 * 真正的代码执行逻辑
 */ 
jvalue javsvm::run_java(jmethod *me, jref _this, jargs &args)
{
    jvalue result = {0};

    // 创建栈帧并初始化
    jstack &stack = jvm::get().env().stack;
    jstack_frame &frame = stack.push(me);
    
    
    jclass_attr_code &code = *(me->entrance.code_func);
    jclass_const_pool &constant_pool = me->clazz->class_file->constant_pool;
 
    // 将参数压进局部变量表
    memcpy(frame.variable_table, args.begin(), me->args_slot * sizeof(slot_t));

    const auto code_length = code.code_length;

loop:
    // 解释执行
    while (frame.pc < code_length) {
        const int op = code.code[frame.pc];

        // 调试使用
#ifndef NDEBUG
    LOGD("------------------------dump stack frame------------------------\n");
    LOGD("pc: %d, op: %s(%d)\n", frame.pc, ops_str[op], op);
    LOGD("variable_table(total %d):\n", code.max_locals);
    for (int i = 0; i < code.max_locals; i ++) {
        LOGD("\t\t[%d/%d]: %#llx\n", i, code.max_locals, frame.variable_table[i]);
    }
    int stack_depth = (int) (frame.operand_stack - frame.operand_stack_orig);
    LOGD("operand_stack(cur %d, max %d):\n", stack_depth, code.max_stack);
    for (int i = 0; i < stack_depth; i ++) {
        LOGD("\t\t[%d/%d]: %#llx\n", i, stack_depth, *(frame.operand_stack - i - 1));
    }
    LOGD("------------------------------end-------------------------------\n");
#endif

    /**
     * 注意事项:
     * 1. 对于每条指令，先执行操作，再更新 pc
     */

        switch (op) {
            case nop:
                frame.pc += 1;
                break;
            case aconst_null:
                frame.push_param<jref>(nullptr);
                frame.pc += 1;
                break;
            case iconst_m1:
            case iconst_0:
            case iconst_1:
            case iconst_2:
            case iconst_3:
            case iconst_4:
            case iconst_5:
                frame.push_param<jint>(op - 3);
                frame.pc += 1;
                break;
            case lconst_0:
            case lconst_1:
                frame.push_param<jlong>(op - 9);
                frame.pc += 1;
                break;
            case fconst_0:
            case fconst_1:
            case fconst_2:
                frame.push_param<jfloat>((float) op - 11);
                frame.pc += 1;
                break;
            case dconst_0:
            case dconst_1:
                frame.push_param<jdouble>(op - 14);
                frame.pc += 1;
                break;
            case bipush: {
                u1 val = code.code[frame.pc + 1];
                frame.push_param(val);
                frame.pc += 2;
                break;
            }
            case sipush: {
                u2 val = code.code[frame.pc + 1] << 8;
                val |= code.code[frame.pc + 2];
                frame.push_param(val);
                frame.pc += 3;
                break;
            }
            case ldc: {
                int idx = code.code[frame.pc + 1];
                do_ldc(constant_pool, idx, frame);
                frame.pc += 2;
                break;
            }
            case ldc_w:
            case ldc2_w: {
                int idx = code.code[frame.pc + 1] << 8;
                idx |= code.code[frame.pc + 2];
                do_ldc(constant_pool, idx, frame);
                frame.pc += 3;
                break;
            }
            case iload: 
            case fload: 
            case aload: {
                int idx = code.code[frame.pc + 1];
                frame.operand_stack[0] = frame.variable_table[idx];
                frame.operand_stack += 1;
                frame.pc += 2;
                break;
            }
            case lload:
            case dload: {
                int idx = code.code[frame.pc + 1];
                frame.operand_stack[0] = frame.variable_table[idx];
                frame.operand_stack[1] = frame.variable_table[idx + 1];
                frame.operand_stack += 2;
                frame.pc += 2;
                break;
            }
            case iload_0:
            case iload_1:
            case iload_2:
            case iload_3:
                frame.operand_stack[0] = frame.variable_table[op - 26];
                frame.operand_stack += 1;
                frame.pc += 1;
                break;
            case lload_0:
            case lload_1:
            case lload_2:
            case lload_3:
                frame.operand_stack[0] = frame.variable_table[op - 30];
                frame.operand_stack[1] = frame.variable_table[op - 29];
                frame.operand_stack += 2;
                frame.pc += 1;
                break;
            case fload_0:
            case fload_1:
            case fload_2:
            case fload_3:
                frame.operand_stack[0] = frame.variable_table[op - 34];
                frame.operand_stack += 1;
                frame.pc += 1;
                break;
            case dload_0:
            case dload_1:
            case dload_2:
            case dload_3:
                frame.operand_stack[0] = frame.variable_table[op - 38];
                frame.operand_stack[1] = frame.variable_table[op - 37];
                frame.operand_stack += 2;
                frame.pc += 1;
                break;
            case aload_0:
            case aload_1:
            case aload_2:
            case aload_3:
                frame.operand_stack[0] = frame.variable_table[op - 42];
                frame.operand_stack += 1;
                frame.pc += 1;
                break;

            case iaload: getT<jint>(frame); break;
            case laload: getT<jlong>(frame); break;
            case faload: getT<jfloat>(frame); break;
            case daload: getT<jdouble>(frame); break;
            case aaload: getT<jref>(frame); break;
            case baload: getT<jbyte>(frame); break;
            case caload: getT<jchar>(frame); break;
            case saload: getT<jshort>(frame); break;

            case istore: 
            case fstore: 
            case astore: {
                int idx = code.code[frame.pc + 1];
                frame.variable_table[idx] = frame.operand_stack[-1];
                frame.operand_stack -= 1;
                frame.pc += 2;
                break;
            }
            case lstore: 
            case dstore: {
                int idx = code.code[frame.pc + 1];
                frame.variable_table[idx] = frame.operand_stack[-2];
                frame.variable_table[idx + 1] = frame.operand_stack[-1];
                frame.operand_stack -= 2;
                frame.pc += 2;
                break;
            }
            case istore_0:
            case istore_1:
            case istore_2:
            case istore_3:
                frame.variable_table[op - 59] = frame.operand_stack[-1];
                frame.operand_stack -= 1;
                frame.pc += 1;
                break;
            case lstore_0:
            case lstore_1:
            case lstore_2:
            case lstore_3:
                frame.variable_table[op - 63] = frame.operand_stack[-2];
                frame.variable_table[op - 62] = frame.operand_stack[-1];
                frame.operand_stack -= 2;
                frame.pc += 1;
                break;
            case fstore_0:
            case fstore_1:
            case fstore_2:
            case fstore_3:
                frame.variable_table[op - 67] = frame.operand_stack[-1];
                frame.operand_stack -= 1;
                frame.pc += 1;
                break;
            case dstore_0:
            case dstore_1:
            case dstore_2:
            case dstore_3:
                frame.variable_table[op - 71] = frame.operand_stack[-2];
                frame.variable_table[op - 70] = frame.operand_stack[-1];
                frame.operand_stack -= 2;
                frame.pc += 1;
                break;
            case astore_0:
            case astore_1:
            case astore_2:
            case astore_3:
                frame.variable_table[op - 75] = frame.operand_stack[-1];
                frame.operand_stack -= 1;
                frame.pc += 1;
                break;

            case iastore: setT<jint>(frame);  break;
            case lastore: setT<jlong>(frame); break;
            case fastore: setT<jfloat>(frame); break;
            case dastore: setT<jdouble>(frame); break;
            case aastore: setT<jref>(frame); break;
            case bastore: setT<jbyte>(frame); break;
            case castore: setT<jchar>(frame); break;
            case sastore: setT<jshort>(frame); break;
            case pop:
            case pop2:
                frame.operand_stack -= op - 86;
                frame.pc += 1;
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
                frame.pc += 1;
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
                frame.pc += 1;
                break;
            case swap: {
                slot_t _swap = frame.operand_stack[-1];
                frame.operand_stack[-1] = frame.operand_stack[-2];
                frame.operand_stack[-2] = _swap;
                frame.pc += 1;
                break;
            }
            case iadd: addT<jint>(frame); break;
            case ladd: addT<jlong>(frame); break;
            case fadd: addT<jfloat>(frame); break;
            case dadd: addT<jdouble>(frame); break;

            case isub: subT<jint>(frame); break;
            case lsub: subT<jlong>(frame); break;
            case fsub: subT<jfloat>(frame); break;
            case dsub: subT<jdouble>(frame); break;

            case imul: mulT<jint>(frame); break;
            case lmul: mulT<jlong>(frame); break;
            case fmul: mulT<jfloat>(frame); break;
            case dmul: mulT<jdouble>(frame); break;

            case idiv: divT<jint>(frame); break;
            case _ldiv: divT<jlong>(frame); break;
            case fdiv: divT<jfloat>(frame); break;
            case ddiv: divT<jdouble>(frame); break;

            case irem: remT<jint>(frame); break;
            case lrem: remT<jlong>(frame); break;
            case frem: remT<jfloat>(frame); break;
            case drem: remT<jdouble>(frame); break;

            case ineg: negT<jint>(frame); break;
            case lneg: negT<jlong>(frame); break;
            case fneg: negT<jfloat>(frame); break;
            case dneg: negT<jdouble>(frame); break;

            case ishl: shlT<jint>(frame); break;
            case lshl: shlT<jlong>(frame); break;
            case ishr: shrT<jint>(frame); break;
            case lshr: shrT<jlong>(frame); break;
            case iushr: ushrT<jint, uint32_t>(frame); break;
            case lushr: ushrT<jlong, uint64_t>(frame); break;

            case iand: andT<jint>(frame); break;
            case land: andT<jlong>(frame); break;
            case ior: orT<jint>(frame); break;
            case lor: orT<jlong>(frame); break;
            case ixor: xorT<jint>(frame); break;
            case lxor: xorT<jlong>(frame); break;

            case iinc: {
                u1 idx = code.code[frame.pc + 1];
                auto inc = (int8_t) code.code[frame.pc + 2];
                frame.variable_table[idx] += inc;
                frame.pc += 3;
                break;
            }

            case i2l: i2T<jint, jlong>(frame); break;
            case i2f: i2T<jint, jfloat>(frame); break;
            case i2d: i2T<jint, jdouble>(frame); break;

            case l2i: i2T<jlong, jint>(frame); break;
            case l2f: i2T<jlong, jfloat>(frame); break;
            case l2d: i2T<jlong, jdouble>(frame); break;

            case f2i: i2T<jfloat, jint>(frame); break;
            case f2l: i2T<jfloat, jlong>(frame); break;
            case f2d: i2T<jfloat, jdouble>(frame); break;

            case d2i: i2T<jdouble, jint>(frame); break;
            case d2l: i2T<jdouble, jlong>(frame); break;
            case d2f: i2T<jdouble, jfloat>(frame); break;

            case i2b: i2T<jint, jbyte>(frame); break;
            case i2c: i2T<jint, jchar>(frame); break;
            case i2s: i2T<jint, jshort>(frame); break;

            case lcmp: cmpT<jlong>(frame); break;
            case fcmpl: cmpT<jfloat, numbers::Float, -1>(frame); break;
            case fcmpg: cmpT<jfloat, numbers::Float, 1>(frame); break;
            case dcmpl: cmpT<jdouble, numbers::Double, -1>(frame); break;
            case dcmpg: cmpT<jdouble, numbers::Double, 1>(frame); break;

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

            case _goto: {
                int pc = code.code[frame.pc + 1] << 8;
                pc |= code.code[frame.pc + 2];
                frame.pc += (short) pc;
                break;
            }
            case goto_w: {
                // todo
            }

            // jsr 和 ret 指令已经被 jvm 规范禁止使用
            case jsr: LOGE("invalid op code: jsr\n"); exit(1); break;
            case ret: LOGE("invalid op code: ret\n"); exit(1); break;

            case tableswitch: switch_table(frame, code); break;
            case lookupswitch: lookup_table(frame, code); break;

            case ireturn: result.i = frame.pop_param<jint>(); goto finally;
            case lreturn: result.j = frame.pop_param<jlong>(); goto finally;
            case freturn: result.f = frame.pop_param<jfloat>(); goto finally;
            case dreturn: result.d = frame.pop_param<jdouble>(); goto finally;
            case areturn: result.l = frame.pop_param<jref>(); goto finally;
            case _return: goto finally;

            // 下面的指令不需要加入空检查和类初始化检查的逻辑，jfield 和 jmethod 中会有真实的逻辑
            case getstatic:
                get_field<static_field>(frame, code, constant_pool);
                break;
            case putstatic:
                put_field<static_field>(frame, code, constant_pool);
                break;
            case getfield:
                get_field<direct_field>(frame, code, constant_pool);
                break;
            case putfield:
                put_field<direct_field>(frame, code, constant_pool);
                break;
            case invokevirtual:
                invoke_method<virtual_method, 3>(frame, code, constant_pool);
                break;
            case invokespecial:
                invoke_method<special_method, 3>(frame, code, constant_pool);
                break;
            case invokestatic:
                invoke_method<static_method, 3>(frame, code, constant_pool);
                break;
            case invokeinterface:
                invoke_method<interface_method, 5>(frame, code, constant_pool);
                break;
            case invokedynamic:
                // todo:
                break;
            case _new: {
                int idx = code.code[frame.pc + 1] << 8;
                idx |= code.code[frame.pc + 2];

                jclass *cls = get_class(idx, constant_pool);
                jref ref = cls->new_instance();

                frame.push_param<jref>(ref);
                frame.pc += 3;
                break;
            }
            case newarray:
                new_array(frame, code);
                break;
            case anewarray:
                a_new_array(frame, constant_pool, code);
                break;
            case arraylength:
                array_length(frame);
                break;

            case athrow: {
                auto exp = frame.pop_param<jref>();
                throw_throwable(exp);
                frame.pc += 1;
                break;
            }

            case checkcast: check_cast(frame, constant_pool, code); break;
            case instanceof: instance_of(frame, constant_pool, code); break;

            case monitorenter: {
                jref ref = frame.pop_param<jref>();
                auto ptr = jvm::get().heap.lock(ref);
                if (ptr == nullptr) {
                    throw_exp("java/lang/NullPointerException", "monitorenter");
                }
                else {
                    ptr->lock();
                }
                frame.pc += 1;
                break;
            }
            case monitorexit: {
                jref ref = frame.pop_param<jref>();
                auto ptr = jvm::get().heap.lock(ref);
                if (ptr == nullptr) {
                    throw_exp("java/lang/NullPointerException", "monitorenter");
                }
                else {
                    ptr->unlock();
                }
                frame.pc += 1;
                break;
            }
            case wide:
            case multianewarray:
                // todo
            case jsr_w:
                LOGE("invalid op code: jsr\n"); exit(1); break;
            default:
                LOGE("unsupported byte code %#02x\n", op);
                exit(1);
        }
    }
_catch:
    // 正常情况下通过 return 语句返回，是不会走到这里的。
    // 执行到这里说明要么字节码出现问题，要么 pc 被异常处理机制调整过
    if (frame.exp == nullptr) {
        // 没有异常发生，说明当前函数并不支持处理该异常，直接返回
        goto finally;
    }
    frame.operand_stack = frame.operand_stack_orig;
    frame.pc = frame.exp_handler_pc;
    frame.push_param<jref>(frame.exp);
    frame.exp_handler_pc = 0;
    frame.exp = nullptr;
    goto loop;

finally:
    return result;
}
