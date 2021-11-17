

#include "object/jmethod.h"
#include "object/jfield.h"
#include "object/jobject.h"
#include "object/jclass.h"

#include "utils/number_utils.h"
#include "class/jclass_file.h"

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
    *(T *) (frame.operand_stack) = t;
    frame.operand_stack += slotof(T);
}


/**
 * 函数模板，用来从操作数栈中获取指定的参数并回调
 * 假如现在的操作数栈从栈底到栈顶为 [3, 2, 1]
 * 当传入 int (*func)(float a, int b) 时，执行的逻辑为 func(float(2), int(1));
 * 
 * 传入函数的最后一个参数一定是操作数栈的栈顶
 * 需要注意的是，这一点并没有完全实现，因为不同编译器的参数计算顺序是不一定的
 * 等待进一步实现
 */

#if ! __GLIBCXX__
#error only for gcc
#endif

template <typename ResultT, typename...ArgsT>
void invoke(ResultT(*func)(ArgsT...), jstack_frame &args)
{
    ResultT result = func(pop_param<ArgsT>(frame)...);
    push_param(frame, result);
}

template <typename...ArgsT>
void invoke(void (*func)(ArgsT...), jstack_frame &args)
{
    func(pop_param<ArgsT>(frame)...);
}


template <typename T>
T addT(const T a, const T b) { return a + b; }

template <typename T>
T subT(const T a, const T b) { return a - b; }

template <typename T>
T mulT(const T a, const T b) { return a * b; }

template <typename T>
T divT(const T a, const T b) { return a / b; }

template <typename T>
T remT(const T a, const T b) { return a % b; }

template <typename T>
T negT(const T val) { return -val; }

template <typename T>
T shlT(const T a, const jint b) { return a << b; }

template <typename T>
T shrT(const T a, const jint b) { return a >> b; }

template <typename SignedT, typename UnsignedT>
jint ushrT(const SignedT a, const jint b) { return ((UnsignedT) a) >> b; }



template <typename T>
T andT(const T a, const T b) { return a & b; }

template <typename T>
T orT(const T a, const T b) { return a | b; }

template <typename T>
T xorT(const T a, const T b) { return a ^ b; }

template <typename T>
T incT(const T a) { return a + 1; }

template <typename TypeIn, typename TypeOut>
TypeOut i2T(const TypeIn a) { return (TypeOut) a; }

jint lcmpT(const jlong a, const jlong b) { return a == b ? 0 : (a < b ? -1 : 1); }

template <typename Type, typename TypeW, jint ErrVal>
jint cmpT(const Type a, const Type b)
{
    if (a == TypeW::NaN || b == TypeW::NaN) {
        return ErrVal;
    }
    return a == b ? 0 : (a < b ? -1 : 1);
}


static constexpr jint (*fcmplT)(const float, const float ) = cmpT<jfloat, number_utils::Float, -1>;
static constexpr jint (*fcmpgT)(const float, const float ) = cmpT<jfloat, number_utils::Float, 1>;
static constexpr jint (*dcmplT)(const jdouble, const jdouble ) = cmpT<jdouble, number_utils::Double, -1>;
static constexpr jint (*dcmpgT)(const jdouble, const jdouble ) = cmpT<jdouble, number_utils::Double, 1>;


template <typename Type, typename Comparator>
void ifT(jstack_frame &frame, jclass_attr_code &code)
{
    Comparator<Type> cmp;
    Type val = *(Type *)(frame.operand_stack - 1);

    if (cmp.operator()(val, 0)) {
        short pc = code.code[frame.pc ++] << 8;
        pc |= code.code[frame.pc ++];
        frame.pc += pc;
    }
}

template <typename Type, typename Comparator>
void ifT(jstack_frame &frame, jclass_attr_code &code)
{
    Comparator<Type> cmp;
    Type val = pop_param(frame);

    if (cmp.operator()(val, 0)) {
        short pc = code.code[frame.pc ++] << 8;
        pc |= code.code[frame.pc ++];
        frame.pc += pc;
    }
}

template<>
void ifT<void, void>(jstack_frame &frame, jclass_attr_code &code)
{
    short pc = code.code[frame.pc ++] << 8;
    pc |= code.code[frame.pc ++];
    frame.pc += pc;
}


template <typename Type, typename Comparator>
void ifcmpT(jstack_frame &frame, jclass_attr_code &code)
{
    Comparator<Type> cmp;

    Type val1 = pop_param(frame);
    Type val2 = pop_param(frame);

    if (cmp.operator()(val, val2)) {
        short pc = code.code[frame.pc ++] << 8;
        pc |= code.code[frame.pc ++];
        frame.pc += pc;
    }
}

struct ref_equ
{
    bool operator()(jref &a, jref &b)
    {
        return jobject::cast(a) == jobject::cast(b);
    }
};

struct ref_not_equ
{
    bool operator()(jref &a, jref &b)
    {
        return jobject::cast(a) != jobject::cast(b);
    }
};


static inline void switch_table(jstack_frame &frame, jclass_attr_code &code)
{
    jint (*reverse)(jint) = number_utils::reverse_endian<jint>;

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
    constexpr jint (*reverse)(jint) = number_utils::reverse_endian<jint>;

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

static inline void do_ldc(jclass_const_pool &pool, int idx, int len, jstack_frame& frame)
{
    
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

        // 获取这个域的引用
        auto *name_and_type = pool.cast<jclass_const_name_and_type>(field_ref->name_and_type_index);
        auto *name = (char*) pool.cast<jclass_const_utf8>(name_and_type->name_index)->bytes;
        auto *type = (char*) pool.cast<jclass_const_utf8>(name_and_type->type_index)->bytes;

        field_ref->extra = clazz->get_field(name, type);

        if (field_ref->extra == nullptr) {
            LOGE("can't find field [%s][%s] in class [%s]\n", name, type, clazz->get_name());
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
            LOGE("can't find method [%s][%s] in class [%s]\n", name, type, clazz->get_name());
            exit(1);
        }
    }
    return (jmethod *)method_ref->extra;
}


static inline jvalue pop_jvalue(jstack_frame &frame, int slot_num)
{
    jvalue val = {0};
    switch (slot_num) {
        case 1: val.i = pop_param<jint>(frame); break;
        case 2: val.j = pop_param<jlong>(frame); break;
    }
    return val;
}

static inline void push_jvalue(jstack_frame &frame, jvalue &val, int slot_num)
{
    switch (slot_num) {
        case 1: push_param<jint>(frame, val.i); break;
        case 2: push_param<jlong>(frame, val.j); break;
    }
}
