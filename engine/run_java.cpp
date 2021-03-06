


#include "engine.h"
#include "opsdef.h"
#include "opslib.h"


static void dump_frame_info(jmethod *me, java_stack_frame &frame, int op, jclass_attr_code &code) noexcept
{
    LOGD("\n------------------------dump stack frame------------------------\n");
    LOGD("method: %s->%s%s\n", me->clazz->name, me->name, me->sig);
    LOGD("pc: %d, op: %s(%d)\n", frame.pc, ops_str[op], op);
    LOGD("variable_table(total %d):\n", code.max_locals);
    for (int i = 0; i < code.max_locals; i ++) {
        char buff[512] = { '\0' };
        if (frame.variable_ref_table[i]) {
            if (frame.variable_table[i] == 0) {
                strcpy(buff, "(null)");
            }
            else {
                jobject *obj = jheap::cast((jref) frame.variable_table[i]);
                snprintf(buff, sizeof(buff), "(%s@%04x)", obj->klass->name, 0xFFFF & obj->hash_code());
            }
        }
        LOGD("\t\t[%d/%d]: %#llx %s\n", i, code.max_locals, frame.variable_table[i], buff);
    }
    int stack_depth = (int) (frame.operand_stack - frame.operand_stack_orig);
    LOGD("operand_stack(cur %d, max %d):\n", stack_depth, code.max_stack);

    for (int i = stack_depth - 1; i >= 0; --i) {
        char buff[512] = { '\0' };
        if (frame.operand_ref_stack_orig[i]) {
            if (frame.operand_stack_orig[i] == 0) {
                strcpy(buff, "(null)");
            }
            else {
                jobject *obj = jheap::cast((jref) frame.operand_stack_orig[i]);
                snprintf(buff, sizeof(buff), "(%s@%04x)", obj->klass->name, 0xFFFF & obj->hash_code());
            }
        }
        LOGD("\t\t[%d/%d]: %#llx %s\n", i, stack_depth, frame.operand_stack_orig[i], buff);
    }
    LOGD("------------------------------end-------------------------------\n");
}



/**
 * 真正的代码执行逻辑
 */ 
jvalue javsvm::run_java(jmethod *me, jref lock_object, jargs &args)
{
    jvalue result;

    // 创建栈帧并初始化
    jstack &stack = jvm::get().env().stack;
    auto &frame = (java_stack_frame&) stack.push(me);

    frame.lock_if(lock_object);
    
    jclass_attr_code &code = *(me->entrance.code_func);
    jclass_const_pool &constant_pool = me->clazz->class_file->constant_pool;
 
    // 将参数压进局部变量表
    memcpy(frame.variable_table, args.begin(), me->args_slot * sizeof(slot_t));
    memcpy(frame.variable_ref_table, me->args_ref_table, me->args_slot);

    check_safety_point();

    const auto code_length = code.code_length;

loop:
    // 解释执行
    while (frame.pc < code_length) {
        const int op = code.code[frame.pc];

        // 调试使用
#ifndef NDEBUG
        dump_frame_info(me, frame, op, code);
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
                int val = code.code[frame.pc + 1];
                frame.push_param((jbyte)(val & 0xFF));
                frame.pc += 2;
                break;
            }
            case sipush: {
                int val = code.code[frame.pc + 1] << 8;
                val |= code.code[frame.pc + 2];
                frame.push_param((jshort) (val & 0xFFFF));
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
            case fload: {
                int idx = code.code[frame.pc + 1];
                frame.load_param<jint>(idx);
                frame.pc += 2;
                break;
            }
            case aload: {
                int idx = code.code[frame.pc + 1];
                frame.load_param<jref>(idx);
                frame.pc += 2;
                break;
            }
            case lload:
            case dload: {
                int idx = code.code[frame.pc + 1];
                frame.load_param<jlong>(idx);
                frame.pc += 2;
                break;
            }
            case iload_0:
            case iload_1:
            case iload_2:
            case iload_3:
                frame.load_param<jint>(op - 26);
                frame.pc += 1;
                break;
            case lload_0:
            case lload_1:
            case lload_2:
            case lload_3:
                frame.load_param<jlong>(op - 30);
                frame.pc += 1;
                break;
            case fload_0:
            case fload_1:
            case fload_2:
            case fload_3:
                frame.load_param<jfloat>(op - 34);
                frame.pc += 1;
                break;
            case dload_0:
            case dload_1:
            case dload_2:
            case dload_3:
                frame.load_param<jdouble>(op - 38);
                frame.pc += 1;
                break;
            case aload_0:
            case aload_1:
            case aload_2:
            case aload_3:
                frame.load_param<jref>(op - 42);
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
            case fstore: {
                int idx = code.code[frame.pc + 1];
                frame.store_param<jint>(idx);
                frame.pc += 2;
                break;
            }
            case astore: {
                int idx = code.code[frame.pc + 1];
                frame.store_param<jref>(idx);
                frame.pc += 2;
                break;
            }
            case lstore: 
            case dstore: {
                int idx = code.code[frame.pc + 1];
                frame.store_param<jlong>(idx);
                frame.pc += 2;
                break;
            }
            case istore_0:
            case istore_1:
            case istore_2:
            case istore_3:
                frame.store_param<jint>(op - 59);
                frame.pc += 1;
                break;
            case lstore_0:
            case lstore_1:
            case lstore_2:
            case lstore_3:
                frame.store_param<jlong>(op - 63);
                frame.pc += 1;
                break;
            case fstore_0:
            case fstore_1:
            case fstore_2:
            case fstore_3:
                frame.store_param<jfloat>(op - 67);
                frame.pc += 1;
                break;
            case dstore_0:
            case dstore_1:
            case dstore_2:
            case dstore_3:
                frame.store_param<jdouble>(op - 71);
                frame.pc += 1;
                break;
            case astore_0:
            case astore_1:
            case astore_2:
            case astore_3:
                frame.store_param<jref>(op - 75);
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
                frame.operand_ref_stack -= op - 86;
                frame.pc += 1;
                break;
            case dup_x2:
                frame.operand_stack[2] = frame.operand_stack[-1];
                frame.operand_ref_stack[2] = frame.operand_ref_stack[-1];
                [[fallthrough]];
            case dup_x1:
                frame.operand_stack[1] = frame.operand_stack[-1];
                frame.operand_ref_stack[1] = frame.operand_ref_stack[-1];
                [[fallthrough]];
            case dup:
                frame.operand_stack[0] = frame.operand_stack[-1];
                frame.operand_ref_stack[0] = frame.operand_ref_stack[-1];
                frame.operand_stack += op - 88;
                frame.operand_ref_stack += op - 88;
                frame.pc += 1;
                break;
            case dup2_x2:
                frame.operand_stack[4] = frame.operand_stack[-2];
                frame.operand_stack[5] = frame.operand_stack[-1];
                frame.operand_ref_stack[4] = frame.operand_ref_stack[-2];
                frame.operand_ref_stack[5] = frame.operand_ref_stack[-1];
                [[fallthrough]];
            case dup2_x1:
                frame.operand_stack[2] = frame.operand_stack[-2];
                frame.operand_stack[3] = frame.operand_stack[-1];
                frame.operand_ref_stack[2] = frame.operand_ref_stack[-2];
                frame.operand_ref_stack[3] = frame.operand_ref_stack[-1];
                [[fallthrough]];
            case dup2:
                frame.operand_stack[0] = frame.operand_stack[-2];
                frame.operand_stack[1] = frame.operand_stack[-1];
                frame.operand_ref_stack[0] = frame.operand_ref_stack[-2];
                frame.operand_ref_stack[1] = frame.operand_ref_stack[-1];
                frame.operand_stack += (op - 91) << 1;
                frame.operand_ref_stack += (op - 91) << 1;
                frame.pc += 1;
                break;
            case swap: {
                slot_t _swap = frame.operand_stack[-1];
                frame.operand_stack[-1] = frame.operand_stack[-2];
                frame.operand_stack[-2] = _swap;
                u1 _swap2 = frame.operand_ref_stack[-1];
                frame.operand_ref_stack[-1] = frame.operand_ref_stack[-2];
                frame.operand_ref_stack[-2] = _swap2;
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
            case ifge: ifT<jint, std::greater_equal<jint>>(frame, code); break;
            case ifgt: ifT<jint, std::greater<jint>>(frame, code); break;
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

                // 非 count-loop 回跳处设置安全点
//                if (pc < 0)
                check_safety_point();
                break;
            }
            case goto_w: {
                int pc = 0;
                pc |= code.code[frame.pc + 1] << 24;
                pc |= code.code[frame.pc + 2] << 16;
                pc |= code.code[frame.pc + 3] << 8;
                pc |= code.code[frame.pc + 4] ;
                frame.pc += pc;

                // 非 count-loop 回跳处设置安全点
//                if (pc < 0)
                check_safety_point();
                break;
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
            case _return: result.j = 0; goto finally;

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
//            case invokedynamic:
//                // todo:
//                break;
            case _new: {
                int idx = code.code[frame.pc + 1] << 8;
                idx |= code.code[frame.pc + 2];

                // new 指令前添加安全点
                check_safety_point();

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
                check_safety_point();
                auto exp = frame.pop_param<jref>();
                throw_throwable(exp);
                frame.pc += 1;
                break;
            }

            case checkcast: check_cast(frame, constant_pool, code); break;
            case instanceof: instance_of(frame, constant_pool, code); break;

            case monitorenter: {
                jref ref = frame.pop_param<jref>();
                auto ptr = jheap::cast(ref);
                if (ptr == nullptr) {
                    throw_exp("java/lang/NullPointerException", "monitorenter");
                }
                else {
                    // 进入安全区之前需要先锁住对象，拿到 lock_event
                    // 因为进入安全区之后 jobject 指针位置可能会改变，我们不能在安全区内访问
                    // jobject. 但访问 lock_event 是可以的
                    auto event = ptr->lock_internal();
                    // 防止重新排序到下面
                    std::atomic_thread_fence(std::memory_order_seq_cst);
                    enter_safety_area();
                    event->lock();
                    leave_safety_area();
                }
                frame.pc += 1;
                break;
            }
            case monitorexit: {
                jref ref = frame.pop_param<jref>();
                auto ptr = jheap::cast(ref);
                if (ptr == nullptr) {
                    throw_exp("java/lang/NullPointerException", "monitorenter");
                }
                else {
                    auto ok = ptr->unlock();
                    assert(ok == 0);
                }
                frame.pc += 1;
                break;
            }
//            case wide:
//            case multianewarray:
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
    // 清空操作数栈
    frame.reset_operand_stack();

    frame.pc = frame.exp_handler_pc;
    frame.push_param<jref>(frame.exp);
    frame.exp_handler_pc = 0;
    frame.exp = nullptr;
    goto loop;

finally:
    frame.unlock();
    stack.pop();
    return result;
}
