

#include "../object/jmethod.h"
#include "../object/jfield.h"
#include "../object/jobject.h"
#include "../object/jclass.h"

#include "../utils/numbers.h"
#include "../class/jclass_file.h"

#include <cmath>

using namespace javsvm;


template <typename T>
static T pop_param(jstack_frame &frame)
{
    frame.operand_stack -= slotof(T);
    T t = *(T *)(frame.operand_stack);
    return t;
}

template <typename T>
static void push_param(jstack_frame &frame, const T &t)
{
    *frame.operand_stack = 0;
    *(T *) (frame.operand_stack) = t;
    frame.operand_stack += slotof(T);
}



template <typename T>
static void getT(jstack_frame &frame)
{
    int idx = pop_param<int>(frame);
    jref ref = pop_param<jref>(frame);

    T buff;

    jvm::get().array.get_array_region(ref, idx, 1, &buff);
    push_param<T>(frame, buff);
}

template <typename T>
static void setT(jstack_frame &frame)
{
    T val = pop_param<T>(frame);
    int idx = pop_param<int>(frame);
    jref ref = pop_param<jref>(frame);

    jvm::get().array.set_array_region(ref, idx, 1, &val);
}

template <typename T>
static void addT(jstack_frame &frame)
{
    T p = pop_param<T>(frame);
    T q = pop_param<T>(frame);
    push_param(frame, p + q);
    frame.pc += 1;
}

template <typename T>
static void subT(jstack_frame &frame)
{
    T p = pop_param<T>(frame);
    T q = pop_param<T>(frame);
    push_param(frame, q - p); // NOT (p - q)
}

template <typename T>
static void mulT(jstack_frame &frame)
{
    T p = pop_param<T>(frame);
    T q = pop_param<T>(frame);
    push_param(frame, q * p);
}

template <typename T>
static void divT(jstack_frame &frame)
{
    T p = pop_param<T>(frame);
    T q = pop_param<T>(frame);
    push_param(frame, q / p); // NOT (p / q)
}

template <typename T>
static void remT(jstack_frame &frame)
{
    T p = pop_param<T>(frame);
    T q = pop_param<T>(frame);
    push_param(frame, q % p); // NOT (p % q)
}


template <>
void remT<jfloat>(jstack_frame &frame)
{
    auto q = pop_param<jfloat>(frame);
    auto p = pop_param<jfloat>(frame);
    push_param<jfloat>(frame, fmod(p, q));
}
template <>
void remT<jdouble>(jstack_frame &frame)
{
    auto q = pop_param<jdouble>(frame);
    auto p = pop_param<jdouble>(frame);
    push_param<jdouble>(frame, fmod(p, q));
}

template <typename T>
static void negT(jstack_frame &frame)
{
    T val = pop_param<T>(frame);
    push_param<T>(frame, - val);
}

template <typename T>
static void shlT(jstack_frame &frame)
{
    T p = pop_param<T>(frame);
    T q = pop_param<T>(frame);
    push_param(frame, q << p); // NOT (p << q)
}

template <typename T>
static void shrT(jstack_frame &frame)
{
    T p = pop_param<T>(frame);
    T q = pop_param<T>(frame);
    push_param(frame, q >> p); // NOT (p >> q)
}

template <typename SignedT, typename UnsignedT>
static void ushrT(jstack_frame &frame)
{
    SignedT p = pop_param<SignedT>(frame);
    SignedT q = pop_param<SignedT>(frame);
    push_param(frame, ((UnsignedT) q) >> p); // NOT (p >> q)
}



template <typename T>
static void andT(jstack_frame &frame)
{
    T p = pop_param<T>(frame);
    T q = pop_param<T>(frame);
    push_param(frame, q & p);
}

template <typename T>
static void orT(jstack_frame &frame)
{
    T p = pop_param<T>(frame);
    T q = pop_param<T>(frame);
    push_param(frame, q | p);
}

template <typename T>
static void xorT(jstack_frame &frame)
{
    T p = pop_param<T>(frame);
    T q = pop_param<T>(frame);
    push_param(frame, q ^ p);
}

static void inc(jstack_frame &frame, jclass_attr_code &code)
{
    u1 idx = code.code[frame.pc + 1];
    char inc = (char) code.code[frame.pc + 2];
    frame.variable_table[idx] += inc;
    frame.pc += 3;
}

template <typename TypeIn, typename TypeOut>
static void i2T(jstack_frame &frame)
{
    TypeIn val = pop_param<TypeIn>(frame);
    push_param<TypeOut>(frame, (TypeOut) val);
}


template <typename Type, typename TypeW, jint ErrVal>
static void cmpT(jstack_frame &frame)
{
    Type b = pop_param<Type>(frame);
    Type a = pop_param<Type>(frame);

    if (a == TypeW::NaN || b == TypeW::NaN) {
        push_param<int>(frame, ErrVal);
    }
    push_param<int>(frame, a == b ? 0 : (a < b ? -1 : 1));
}

template <typename T>
static void cmpT(jstack_frame &frame)
{
    T b = pop_param<T>(frame);
    T a = pop_param<T>(frame);
    push_param<int>(frame, a == b ? 0 : (a < b ? -1 : 1));
}


template <typename Type, typename Comparator>
static void ifT(jstack_frame &frame, jclass_attr_code &code)
{
    Type val = pop_param<Type>(frame);

    if (Comparator().operator()(val, 0)) {
        int pc = code.code[frame.pc + 1] << 8;
        pc |= code.code[frame.pc + 2];
        frame.pc += (short) pc;
    }
    else {
        frame.pc += 3;
    }
}

template<>
void ifT<void, void>(jstack_frame &frame, jclass_attr_code &code)
{
    int pc = code.code[frame.pc + 1] << 8;
    pc |= code.code[frame.pc + 2];
    frame.pc += (short) pc;
}


template <typename Type, typename Comparator>
static void ifcmpT(jstack_frame &frame, jclass_attr_code &code)
{
    Type val2 = pop_param<Type>(frame);
    Type val1 = pop_param<Type>(frame);

    if (Comparator().operator()(val1, val2)) {
        int pc = code.code[frame.pc + 1] << 8;
        pc |= code.code[frame.pc + 2];
        frame.pc += (short) pc;
    }
    else {
        frame.pc += 3;
    }
}

struct ref_equ
{
    bool operator()(const jref &a, const jref &b)
    {
        auto &heap = jvm::get().heap;
        return heap.lock(a).get() == heap.lock(b).get();
    }
};

struct ref_not_equ
{
    bool operator()(const jref &a, const jref &b)
    {
        auto &heap = jvm::get().heap;
        return heap.lock(a).get() != heap.lock(b).get();
    }
};


// todo: 下面的所有函数都需要校验 pc 的 ++ 逻辑

static inline void switch_table(jstack_frame &frame, jclass_attr_code &code)
{
    jint (*reverse)(jint) = numbers::reverse_endian<jint>;

    int pc = ((frame.pc - 1) | 3) + 1;
    jint *buff = (jint *) (code.code + pc);

    jint default_byte = buff[0];
    jint low_byte = reverse(buff[1]);
    jint high_byte = reverse(buff[2]);

    jint val = pop_param<jint>(frame);

    if (val < low_byte || val > high_byte) {
        frame.pc += reverse(default_byte);
    } 
    else {
        frame.pc += reverse(buff[3 + val - low_byte]);
    }
}

static inline void lookup_table(jstack_frame &frame, jclass_attr_code &code)
{
    constexpr jint (*reverse)(jint) = numbers::reverse_endian<jint>;

    int pc = ((frame.pc - 1) | 3) + 1;
    jint *buff = (jint *) (code.code + pc);

    jint default_byte = buff[0];
    jint pairs_count = reverse(buff[1]);

    jint val = pop_param<jint>(frame);

    jint *pair = (jint *)bsearch(&val, buff + 2, pairs_count, 8, [](const void *p1, const void *p2) -> int {
        jint i1 = *(jint *)p1;    // 此时 i1 == val
        jint i2 = reverse(*(jint *) p2);
        return i1 - i2;
    });
    if (pair == nullptr) {
        frame.pc += reverse(default_byte);
    }
    else {
        frame.pc += reverse(pair[1]);
    }
}

static inline void do_ldc(jclass_const_pool &pool, int idx, jstack_frame& frame)
{
    auto *const_value = pool.child_at(idx - 1);

    switch (const_value->tag) {
        case jclass_const_int::TAG: {
            u4 i = ((jclass_const_int *)const_value)->bytes;
            push_param(frame, *(jint *) &i);
            break;
        }
        case jclass_const_float::TAG: {
            u4 f = ((jclass_const_float *)const_value)->bytes;
            push_param(frame, *(jfloat *) &f);
            break;
        }
        case jclass_const_long::TAG: {
            u8 l = ((jclass_const_long *)const_value)->bytes;
            push_param(frame, *(jlong *)&l);
            break;
        }
        case jclass_const_double::TAG: {
            u8 d = ((jclass_const_double *)const_value)->bytes;
            push_param(frame, *(jdouble *)&d);
            break;
        }
        case jclass_const_string::TAG: {
            // 这里比较复杂，因为要创建出一个新的 java.lang.String 对象
            auto *s = ((jclass_const_string *) const_value);
            auto *utf8 = pool.cast<jclass_const_utf8>(s->index);
            auto ref = jvm::get().string_pool.find_or_new((char*) utf8->bytes);
            push_param(frame, ref);
            break;
        }
        default:
            LOGE("unknown const_value_tag %d\n", const_value->tag);
            break;
    }
}

static inline jclass *get_class(int index, jclass_const_pool &pool)
{
    auto *class_info = pool.cast<jclass_const_class>(index);
    if (class_info->extra == nullptr) {
        auto *class_name = pool.cast<jclass_const_utf8>(class_info->index);

        class_info->extra = jclass::find_class((char*) class_name->bytes);
        
        if (class_info->extra == nullptr) {
            LOGE("can't find class [%s]\n", class_name->bytes);
            exit(1);
        }
    }
    return (jclass*) class_info->extra;
}

static inline jfield* get_field(int index, jclass_const_pool &pool)
{
    auto *field_ref = pool.cast<jclass_const_field_ref>(index);
    if (field_ref->extra == nullptr) {
        // 先获取到这个类的引用
        jclass *clazz = get_class(field_ref->class_index, pool);

        // 获取这个字段的引用
        auto *name_and_type = pool.cast<jclass_const_name_and_type>(field_ref->name_and_type_index);
        auto *name = (char*) pool.cast<jclass_const_utf8>(name_and_type->name_index)->bytes;
        auto *type = (char*) pool.cast<jclass_const_utf8>(name_and_type->type_index)->bytes;

        field_ref->extra = clazz->get_field(name, type);

        if (field_ref->extra == nullptr) {
            LOGE("can't find field [%s][%s] in class [%s]\n", name, type, clazz->name);
            exit(1);
        }
    }
    return (jfield *)field_ref->extra;
}


static inline jmethod* get_method(int index, jclass_const_pool &pool)
{
    auto *method_ref = pool.cast<jclass_const_method_ref>(index);
    if (method_ref->extra == nullptr) {
        // 先获取到这个类的引用
        jclass *clazz = get_class(method_ref->class_index, pool);

        // 获取这个域的引用
        auto *name_and_type = pool.cast<jclass_const_name_and_type>(method_ref->name_and_type_index);
        auto *name = (char*) pool.cast<jclass_const_utf8>(name_and_type->name_index)->bytes;
        auto *type = (char*) pool.cast<jclass_const_utf8>(name_and_type->type_index)->bytes;

        method_ref->extra = clazz->get_method(name, type);

        if (method_ref->extra == nullptr) {
            LOGE("can't find method [%s][%s] in class [%s]\n", name, type, clazz->name);
            exit(1);
        }
    }
    return (jmethod *)method_ref->extra;
}


static inline jvalue pop_jvalue(jstack_frame &frame, int slot_num)
{
    jvalue val = {0};
    switch (slot_num) {
        case 0: break;
        case 1: val.i = pop_param<jint>(frame); break;
        case 2: val.j = pop_param<jlong>(frame); break;
        default:
            LOGE("pop_jvalue: unknown slot_num: %d\n", slot_num);
    }
    return val;
}

static inline void push_jvalue(jstack_frame &frame, jvalue &val, int slot_num)
{
    switch (slot_num) {
        case 0: break;
        case 1: push_param<jint>(frame, val.i); break;
        case 2: push_param<jlong>(frame, val.j); break;
        default:
            LOGE("push_jvalue: unknown slot_num: %d\n", slot_num);
    }
}

static inline void new_array(jstack_frame &frame, jclass_attr_code &code)
{
    int length = pop_param<jint>(frame);
    int type = code.code[frame.pc + 1];

    jref ref = nullptr;
    jarray &array = jvm::get().array;

    switch (type) {
        case 4: ref = array.new_bool_array(length); break;
        case 5: ref = array.new_char_array(length); break;
        case 6: ref = array.new_float_array(length); break;
        case 7: ref = array.new_double_array(length); break;
        case 8: ref = array.new_byte_array(length); break;
        case 9: ref = array.new_short_array(length); break;
        case 10: ref = array.new_int_array(length); break;
        case 11: ref = array.new_long_array(length); break;
        default:
            LOGI("new_array: unknown array type: %d\n", type);
    }
    push_param(frame, ref);
    frame.pc += 2;
}

static inline void a_new_array(jstack_frame &frame,
                               jclass_const_pool &pool,
                               jclass_attr_code &code)
{
    int length = pop_param<jint>(frame);

    int index = code.code[frame.pc + 1] << 8;
    index |= code.code[frame.pc + 2];

    jclass *klass = get_class(index, pool);
    jref ref = jvm::get().array.new_object_array(klass, length);

    push_param(frame, ref);
    frame.pc += 3;
}


static inline void multi_array(jstack_frame &frame,
                               jclass_const_pool &pool,
                               jclass_attr_code &code)
{
    int index = code.code[frame.pc] << 8;
    index |= code.code[frame.pc];
    jclass *klass = get_class(index, pool);

    u1 dimension = pop_param<u1>(frame);

    // todo: 未完全实现

    frame.pc += 3;
}

static inline void array_length(jstack_frame &frame)
{
    jref ref = pop_param<jref>(frame);
    push_param(frame, jvm::get().array.get_array_length(ref));
    frame.pc += 1;
}


static inline void check_case(jstack_frame &frame,
                              jclass_const_pool &pool,
                              jclass_attr_code &code)
{
    int index = code.code[frame.pc + 1] << 8;
    index |= code.code[frame.pc + 2];
    jclass *klass = get_class(index, pool);

    jref obj = pop_param<jref>(frame);

    if (! klass->is_instance(obj)) {
        // todo: 抛出 class cast exception
    }
    frame.pc += 3;
}

static inline void instance_of(jstack_frame &frame,
                              jclass_const_pool &pool,
                              jclass_attr_code &code)
{
    int index = code.code[frame.pc + 1] << 8;
    index |= code.code[frame.pc + 2];
    jclass *klass = get_class(index, pool);

    jref obj = pop_param<jref>(frame);
    push_param<jint>(frame, klass->is_instance(obj) ? 1 : 0);

    frame.pc += 3;
}






