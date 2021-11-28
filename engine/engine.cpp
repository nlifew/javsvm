


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
    jstack &stack = jvm::get().env().stack();
    jstack_frame &frame = stack.push(me);
    
    
    jclass_attr_code &code = *(me->entrance.code_func);
    jclass_const_pool &constant_pool = me->clazz->class_file->constant_pool;
 
    // 将参数压进局部变量表
    memcpy(frame.variable_table, args.begin(), me->args_slot * sizeof(slot_t));

    // 解释执行
    while (frame.pc < code.code_length) {
        const int op = code.code[frame.pc];

        // 调试使用
#if 1
    LOGI("------------------------dump stack frame------------------------\n");
    LOGI("pc: %d, op: %s(%d)\n", frame.pc, ops_str[op], op);
    LOGI("variable_table(total %d):\n", code.max_locals);
    for (int i = 0; i < code.max_locals; i ++) {
        LOGI("\t\t[%d/%d]: %#llx\n", i, code.max_locals, frame.variable_table[i]);
    }
    int stack_depth = (int) (frame.operand_stack - frame.operand_stack_orig);
    LOGI("operand_stack(cur %d, max %d):\n", stack_depth, code.max_stack);
    for (int i = 0; i < stack_depth; i ++) {
        LOGI("\t\t[%d/%d]: %#llx\n", i, stack_depth, *(frame.operand_stack - i - 1));
    }
    LOGI("------------------------------end-------------------------------\n");
#endif
        switch (op) {
            case nop:
                frame.pc += 1;
                break;
            case aconst_null:
                push_param<jref>(frame, nullptr);
                frame.pc += 1;
                break;
            case iconst_m1:
            case iconst_0:
            case iconst_1:
            case iconst_2:
            case iconst_3:
            case iconst_4:
            case iconst_5:
                push_param<jint>(frame, op - 3);
                frame.pc += 1;
                break;
            case lconst_0:
            case lconst_1:
                push_param<jlong>(frame, op - 9);
                frame.pc += 1;
                break;
            case fconst_0:
            case fconst_1:
            case fconst_2:
                push_param<jfloat>(frame, ((float) op) - 11);
                frame.pc += 1;
                break;
            case dconst_0:
            case dconst_1:
                push_param<jdouble>(frame, op - 14);
                frame.pc += 1;
                break;
            case bipush: {
                u1 val = code.code[frame.pc + 1];
                push_param(frame, val);
                frame.pc += 2;
                break;
            }
            case sipush: {
                u2 val = code.code[frame.pc + 1] << 8;
                val |= code.code[frame.pc + 2];
                push_param(frame, val);
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

            case iaload: getT<jint>(frame); frame.pc += 1; break;
            case laload: getT<jlong>(frame); frame.pc += 1; break;
            case faload: getT<jfloat>(frame); frame.pc += 1; break;
            case daload: getT<jdouble>(frame); frame.pc += 1; break;
            case aaload: getT<jref>(frame); frame.pc += 1; break;
            case baload: getT<jbyte>(frame); frame.pc += 1; break;
            case caload: getT<jchar>(frame); frame.pc += 1; break;
            case saload: getT<jshort>(frame); frame.pc += 1; break;

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

            case iastore: setT<jint>(frame); frame.pc += 1; break;
            case lastore: setT<jlong>(frame); frame.pc += 1; break;
            case fastore: setT<jfloat>(frame); frame.pc += 1; break;
            case dastore: setT<jdouble>(frame); frame.pc += 1; break;
            case aastore: setT<jref>(frame); frame.pc += 1; break;
            case bastore: setT<jbyte>(frame); frame.pc += 1; break;
            case castore: setT<jchar>(frame); frame.pc += 1; break;
            case sastore: setT<jshort>(frame); frame.pc += 1; break;
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

            case isub: subT<jint>(frame); frame.pc += 1; break;
            case lsub: subT<jlong>(frame); frame.pc += 1; break;
            case fsub: subT<jfloat>(frame); frame.pc += 1; break;
            case dsub: subT<jdouble>(frame); frame.pc += 1; break;

            case imul: mulT<jint>(frame); frame.pc += 1; break;
            case lmul: mulT<jlong>(frame); frame.pc += 1; break;
            case fmul: mulT<jfloat>(frame); frame.pc += 1; break;
            case dmul: mulT<jdouble>(frame); frame.pc += 1; break;

            case idiv: divT<jint>(frame); frame.pc += 1; break;
            case _ldiv: divT<jlong>(frame); frame.pc += 1; break;
            case fdiv: divT<jfloat>(frame); frame.pc += 1; break;
            case ddiv: divT<jdouble>(frame); frame.pc += 1; break;

            case irem: remT<jint>(frame); frame.pc += 1; break;
            case lrem: remT<jlong>(frame); frame.pc += 1; break;
            case frem: remT<jfloat>(frame); frame.pc += 1; break;
            case drem: remT<jdouble>(frame); frame.pc += 1; break;

            case ineg: negT<jint>(frame); frame.pc += 1; break;
            case lneg: negT<jlong>(frame); frame.pc += 1; break;
            case fneg: negT<jfloat>(frame); frame.pc += 1; break;
            case dneg: negT<jdouble>(frame); frame.pc += 1; break;

            case ishl: shlT<jint>(frame); frame.pc += 1; break;
            case lshl: shlT<jlong>(frame); frame.pc += 1; break;
            case ishr: shrT<jint>(frame); frame.pc += 1; break;
            case lshr: shrT<jlong>(frame); frame.pc += 1; break;
            case iushr: ushrT<jint, uint32_t>(frame); frame.pc += 1; break;
            case lushr: ushrT<jlong, uint64_t>(frame); frame.pc += 1; break;

            case iand: andT<jint>(frame); frame.pc += 1; break;
            case land: andT<jlong>(frame); frame.pc += 1; break;
            case ior: orT<jint>(frame); frame.pc += 1; break;
            case lor: orT<jlong>(frame); frame.pc += 1; break;
            case ixor: xorT<jint>(frame); frame.pc += 1; break;
            case lxor: xorT<jlong>(frame); frame.pc += 1; break;

            case iinc: inc(frame, code); break;

            case i2l: i2T<jint, jlong>(frame); frame.pc += 1; break;
            case i2f: i2T<jint, jfloat>(frame); frame.pc += 1; break;
            case i2d: i2T<jint, jdouble>(frame); frame.pc += 1; break;

            case l2i: i2T<jlong, jint>(frame); frame.pc += 1; break;
            case l2f: i2T<jlong, jfloat>(frame); frame.pc += 1; break;
            case l2d: i2T<jlong, jdouble>(frame); frame.pc += 1; break;

            case f2i: i2T<jfloat, jint>(frame); frame.pc += 1; break;
            case f2l: i2T<jfloat, jlong>(frame); frame.pc += 1; break;
            case f2d: i2T<jfloat, jdouble>(frame); frame.pc += 1; break;

            case d2i: i2T<jdouble, jint>(frame); frame.pc += 1; break;
            case d2l: i2T<jdouble, jlong>(frame); frame.pc += 1; break;
            case d2f: i2T<jdouble, jfloat>(frame); frame.pc += 1; break;

            case i2b: i2T<jint, jbyte>(frame); frame.pc += 1; break;
            case i2c: i2T<jint, jchar>(frame); frame.pc += 1; break;
            case i2s: i2T<jint, jshort>(frame); frame.pc += 1; break;

            case lcmp: cmpT<jlong>(frame); frame.pc += 1; break;
            case fcmpl: cmpT<jfloat, numbers::Float, -1>(frame); frame.pc += 1; break;
            case fcmpg: cmpT<jfloat, numbers::Float, 1>(frame); frame.pc += 1; break;
            case dcmpl: cmpT<jdouble, numbers::Double, -1>(frame); frame.pc += 1; break;
            case dcmpg: cmpT<jdouble, numbers::Double, 1>(frame); frame.pc += 1; break;

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
                // todo
                break;
            case tableswitch: switch_table(frame, code); break;
            case lookupswitch: lookup_table(frame, code); break;

            case ireturn: result.i = pop_param<jint>(frame); goto finally;
            case lreturn: result.j = pop_param<jlong>(frame); goto finally;
            case freturn: result.f = pop_param<jfloat>(frame); goto finally;
            case dreturn: result.d = pop_param<jdouble>(frame); goto finally;
            case areturn: result.l = pop_param<jref>(frame); goto finally;
            case _return: goto finally;

            // todo: 下面的指令需要加入空检查和类初始化检查的逻辑
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
                push_param<jref>(frame, ref);

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

            case athrow:
                // todo:
                break;

            case checkcast: check_case(frame, constant_pool, code); break;
            case instanceof: instance_of(frame, constant_pool, code); break;

            case monitorenter: {
                jref ref = pop_param<jref>(frame);
                auto ptr = jvm::get().heap.lock(ref);
                ptr->lock();
                break;
            }
            case monitorexit: {
                jref ref = pop_param<jref>(frame);
                auto ptr = jvm::get().heap.lock(ref);
                ptr->unlock();
                break;
            }
            case wide:
            case multianewarray:
            case goto_w:
            case jsr_w:
                // todo:
                break;
            default:
                LOGE("unsupported byte code %#02x\n", op);
                exit(1);
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

jvalue javsvm::run_jni(jmethod *m, jref _this, jargs &args)
{
    // todo: 实现 jni 函数调用
    jvalue v = {0};
    return v;
}