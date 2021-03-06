

#include "../object/jmethod.h"
#include "../object/jfield.h"
#include "../object/jobject.h"
#include "../object/jclass.h"

#include "../utils/numbers.h"
#include "../class/jclass_file.h"
#include "../vm/jvm.h"
#include "../gc/safety_point.h"

#include <cmath> /* fmod */
#include <functional> /* std::equal_to, std::less */

using namespace javsvm;


template <typename T>
static void getT(java_stack_frame &frame)
{
    int idx = frame.pop_param<int>();
    jref ref = frame.pop_param<jref>();

    T buff;

    jvm::get().array.get_array_region(ref, idx, 1, &buff);
    frame.push_param<T>(buff);
    frame.pc += 1;
}

template <typename T>
static void setT(java_stack_frame &frame)
{
    T val = frame.pop_param<T>();
    int idx = frame.pop_param<int>();
    jref ref = frame.pop_param<jref>();

    jvm::get().array.set_array_region(ref, idx, 1, &val);
    frame.pc += 1;
}

template <typename T>
static void addT(java_stack_frame &frame)
{
    T q = frame.pop_param<T>();
    T p = frame.pop_param<T>();
    frame.push_param<T>(p + q);
    frame.pc += 1;
}

template <typename T>
static void subT(java_stack_frame &frame)
{
    T q = frame.pop_param<T>();
    T p = frame.pop_param<T>();
    frame.push_param<T>(p - q);
    frame.pc += 1;
}

template <typename T>
static void mulT(java_stack_frame &frame)
{
    T q = frame.pop_param<T>();
    T p = frame.pop_param<T>();
    frame.push_param<T>(p * q);
    frame.pc += 1;
}

template <typename T>
static void divT(java_stack_frame &frame)
{
    T q = frame.pop_param<T>();
    T p = frame.pop_param<T>();
    frame.push_param<T>(p / q);
    frame.pc += 1;
}

template <>
void divT<jint>(java_stack_frame &frame)
{
    auto q = frame.pop_param<jint>();
    auto p = frame.pop_param<jint>();

    if (q == 0) {
        // todo: 抛出除 0 异常
    }

    frame.push_param<jint>(p / q);
    frame.pc += 1;
}

template <>
void divT<jlong>(java_stack_frame &frame)
{
    auto q = frame.pop_param<jlong>();
    auto p = frame.pop_param<jlong>();

    if (q == 0) {
        // todo: 抛出除 0 异常
    }

    frame.push_param<jlong>(p / q);
    frame.pc += 1;
}

template <typename T>
static void remT(java_stack_frame &frame)
{
    T q = frame.pop_param<T>();
    T p = frame.pop_param<T>();
    frame.push_param<T>(p % q);
    frame.pc += 1;
}


template <>
void remT<jfloat>(java_stack_frame &frame)
{
    auto q = frame.pop_param<jfloat>();
    auto p = frame.pop_param<jfloat>();
    frame.push_param<jfloat>(fmod(p, q));
    frame.pc += 1;
}
template <>
void remT<jdouble>(java_stack_frame &frame)
{
    auto q = frame.pop_param<jdouble>();
    auto p = frame.pop_param<jdouble>();
    frame.push_param<jdouble>(fmod(p, q));
    frame.pc += 1;
}

template <typename T>
static void negT(java_stack_frame &frame)
{
    T val = frame.pop_param<T>();
    frame.push_param<T>(- val);
    frame.pc += 1;
}

template <typename T>
static void shlT(java_stack_frame &frame)
{
    T q = frame.pop_param<T>();
    T p = frame.pop_param<T>();
    frame.push_param<T>(p << q);
    frame.pc += 1;
}

template <typename T>
static void shrT(java_stack_frame &frame)
{
    T q = frame.pop_param<T>();
    T p = frame.pop_param<T>();
    frame.push_param<T>(p >> q);
    frame.pc += 1;
}

template <typename SignedT, typename UnsignedT>
static void ushrT(java_stack_frame &frame)
{
    SignedT q = frame.pop_param<SignedT>();
    SignedT p = frame.pop_param<SignedT>();
    frame.push_param<UnsignedT>(((UnsignedT) p) >> q);
    frame.pc += 1;
}



template <typename T>
static void andT(java_stack_frame &frame)
{
    T q = frame.pop_param<T>();
    T p = frame.pop_param<T>();
    frame.push_param<T>(p & q);
    frame.pc += 1;
}

template <typename T>
static void orT(java_stack_frame &frame)
{
    T q = frame.pop_param<T>();
    T p = frame.pop_param<T>();
    frame.push_param<T>(p | q);
    frame.pc += 1;
}

template <typename T>
static void xorT(java_stack_frame &frame)
{
    T q = frame.pop_param<T>();
    T p = frame.pop_param<T>();
    frame.push_param<T>(p ^ q);
    frame.pc += 1;
}



template <typename TypeIn, typename TypeOut>
static void i2T(java_stack_frame &frame)
{
    TypeIn val = frame.pop_param<TypeIn>();
    frame.push_param<TypeOut>((TypeOut) val);
    frame.pc += 1;
}


template <typename Type, typename TypeW, jint ErrVal>
static void cmpT(java_stack_frame &frame)
{
    Type b = frame.pop_param<Type>();
    Type a = frame.pop_param<Type>();

    if (a == TypeW::NaN || b == TypeW::NaN) {
        frame.push_param<int>(ErrVal);
    }
    frame.push_param<int>(a == b ? 0 : (a < b ? -1 : 1));
    frame.pc += 1;
}

template <typename T>
static void cmpT(java_stack_frame &frame)
{
    T b = frame.pop_param<T>();
    T a = frame.pop_param<T>();
    frame.push_param<int>(a == b ? 0 : (a < b ? -1 : 1));
    frame.pc += 1;
}


template <typename Type, typename Comparator>
static void ifT(java_stack_frame &frame, jclass_attr_code &code)
{
    Type val = frame.pop_param<Type>();

    if (Comparator().operator()(val, 0)) {
        int pc = code.code[frame.pc + 1] << 8;
        pc |= code.code[frame.pc + 2];
        frame.pc += (short) pc;
    }
    else {
        frame.pc += 3;
    }
}



template <typename Type, typename Comparator>
static void ifcmpT(java_stack_frame &frame, jclass_attr_code &code)
{
    Type val2 = frame.pop_param<Type>();
    Type val1 = frame.pop_param<Type>();

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
        return jheap::equals(a, b);
    }
};

struct ref_not_equ
{
    bool operator()(const jref &a, const jref &b)
    {
        return ! jheap::equals(a, b);
    }
};


static inline void switch_table(java_stack_frame &frame, jclass_attr_code &code)
{
    jint (*reverse)(jint) = numbers::reverse_endian<jint>;

    auto pc = ((frame.pc - 1) | 3) + 1;
    jint *buff = (jint *) (code.code + pc);

    jint default_byte = buff[0];
    jint low_byte = reverse(buff[1]);
    jint high_byte = reverse(buff[2]);

    jint val = frame.pop_param<jint>();

    if (val < low_byte || val > high_byte) {
        frame.pc += reverse(default_byte);
    } 
    else {
        frame.pc += reverse(buff[3 + val - low_byte]);
    }
}

static inline void lookup_table(java_stack_frame &frame, jclass_attr_code &code)
{
    constexpr jint (*reverse)(jint) = numbers::reverse_endian<jint>;

    auto pc = ((frame.pc - 1) | 3) + 1;
    jint *buff = (jint *) (code.code + pc);

    jint default_byte = buff[0];
    jint pairs_count = reverse(buff[1]);

    jint val = frame.pop_param<jint>();

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


static inline jclass *get_class(int index, jclass_const_pool &pool)
{
    auto *class_info = pool.cast<jclass_const_class>(index);
    if (class_info->extra == nullptr) {
        auto *class_name = pool.cast<jclass_const_utf8>(class_info->index);

        class_info->extra = jclass::load_class((char *) class_name->bytes);
        
        if (class_info->extra == nullptr) {
            LOGE("can't find class [%s]\n", class_name->bytes);
            exit(1);
        }
    }
    return (jclass*) class_info->extra;
}


static inline void do_ldc(jclass_const_pool &pool, int idx, java_stack_frame& frame)
{
    auto *const_value = pool.child_at(idx - 1);

    switch (const_value->tag) {
        case jclass_const_int::TAG: {
            u4 i = ((jclass_const_int *)const_value)->bytes;
            frame.push_param<jint>(*(jint *) &i);
            break;
        }
        case jclass_const_float::TAG: {
            u4 f = ((jclass_const_float *)const_value)->bytes;
            frame.push_param<jfloat>(*(jfloat *) &f);
            break;
        }
        case jclass_const_long::TAG: {
            u8 l = ((jclass_const_long *)const_value)->bytes;
            frame.push_param<jlong>(*(jlong *)&l);
            break;
        }
        case jclass_const_double::TAG: {
            u8 d = ((jclass_const_double *)const_value)->bytes;
            frame.push_param<jdouble>(*(jdouble *)&d);
            break;
        }
        case jclass_const_string::TAG: {
            // 这里比较复杂，因为要创建出一个新的 java.lang.String 对象
            auto *s = ((jclass_const_string *) const_value);
            auto *utf8 = pool.cast<jclass_const_utf8>(s->index);
            auto ref = jvm::get().string.find_or_new((char*) utf8->bytes);
            frame.push_param<jref>(ref);
            break;
        }
        case jclass_const_class::TAG: {
            frame.push_param<jref>(get_class(idx, pool)->object.get());
            break;
        }
        default:
            LOGE("unknown const_value_tag %d\n", const_value->tag);
            break;
    }
}


struct static_method
{
    jmethod* operator()(jclass *klass, const char *name, const char *type) const noexcept
    {
        return klass->get_static_method(name, type);
    }
    jvalue operator()(jmethod *m, jargs &args) const noexcept
    {
        return m->invoke_static(args);
    }
};

struct virtual_method
{
    jmethod* operator()(jclass *klass, const char *name, const char *type) const noexcept
    {
        return klass->get_virtual_method(name, type);
    }
    jvalue operator()(jmethod *m, jargs &args) const noexcept
    {
        return m->invoke_virtual(args.next<jref>(), args);
    }
};

struct interface_method
{
    jmethod* operator()(jclass *klass, const char *name, const char *type) const noexcept
    {
        return klass->get_interface_method(name, type);
    }
    jvalue operator()(jmethod *m, jargs &args) const noexcept
    {
        return m->invoke_interface(args.next<jref>(), args);
    }
};

struct special_method
{
    jmethod* operator()(jclass *klass, const char *name, const char *type) const noexcept
    {
        return klass->get_method(name, type);
    }
    jvalue operator()(jmethod *m, jargs &args) const noexcept
    {
        return m->invoke_special(args.next<jref>(), args);
    }
};

template <typename T>
static inline jmethod* get_method(int index, jclass_const_pool &pool)
{
    // 如果调用普通的函数，pool.child_at(index - 1) 是 jclass_const_method_ref 类型，
    // 对于接口，却是个 jclass_const_interface_ref 类型。鉴于二者内存上等价，
    // 我们一律强转为 jclass_const_method_ref
    auto *method_ref = reinterpret_cast<jclass_const_method_ref*>(pool.child_at(index - 1));
    // pool.cast<jclass_const_method_ref>(index);

    auto m = (jmethod*) method_ref->extra;

    if (UNLIKELY(m == nullptr)) {
        // 先获取到这个类的引用
        jclass *clazz = get_class(method_ref->class_index, pool);

        // 获取这个域的引用
        auto *name_and_type = pool.cast<jclass_const_name_and_type>(method_ref->name_and_type_index);
        auto *name = (char*) pool.cast<jclass_const_utf8>(name_and_type->name_index)->bytes;
        auto *type = (char*) pool.cast<jclass_const_utf8>(name_and_type->type_index)->bytes;

        method_ref->extra = m = T().operator()(clazz, name, type);
        if (m == nullptr) {
            LOGE("can't find method [%s][%s] in class [%s]\n", name, type, clazz->name);
            exit(1);
        }
    }
    return m;
}


template<typename Method, int OpCount>
static inline void invoke_method(java_stack_frame &frame,
                                 jclass_attr_code &code,
                                 jclass_const_pool &pool)
{
    int idx = code.code[frame.pc + 1] << 8;
    idx |= code.code[frame.pc + 2];

    jmethod *m = get_method<Method>(idx, pool);
    jargs args(frame.operand_stack -= m->args_slot);

    frame.operand_ref_stack -= m->args_slot;

    jvalue val = Method().operator()(m, args);


    switch (m->return_type) {
        case jmethod::none: break;
        case jmethod::integer32: frame.push_param<jint>(val.i); break;
        case jmethod::integer64: frame.push_param<jlong>(val.j); break;
        case jmethod::reference: frame.push_param<jref>(val.l); break;
        default: LOGE("push_jvalue: unknown slot_num: %d\n", m->return_type);
    }

    frame.pc += OpCount;
}


struct static_field
{
    static constexpr bool STATIC = true;

    jfield* operator()(jclass *klass, const char *name, const char *type) const noexcept
    {
        return klass->get_static_field(name, type);
    }
};

struct direct_field
{
    static constexpr bool STATIC = false;

    jfield* operator()(jclass *klass, const char *name, const char *type) const noexcept
    {
        return klass->get_field(name, type);
    }
};

template <typename T>
static inline jfield* get_field(int index, jclass_const_pool &pool)
{
    auto *field_ref = pool.cast<jclass_const_field_ref>(index);
    auto f = (jfield*) field_ref->extra;
    if (f == nullptr) {
        // 先获取到这个类的引用
        jclass *clazz = get_class(field_ref->class_index, pool);

        // 获取这个字段的引用
        auto *name_and_type = pool.cast<jclass_const_name_and_type>(field_ref->name_and_type_index);
        auto *name = (char*) pool.cast<jclass_const_utf8>(name_and_type->name_index)->bytes;
        auto *type = (char*) pool.cast<jclass_const_utf8>(name_and_type->type_index)->bytes;

        field_ref->extra = f = clazz->get_field(name, type);
        if (f == nullptr) {
            LOGE("can't find field [%s][%s] in class [%s]\n", name, type, clazz->name);
            exit(1);
        }
    }
    return f;
}

template <typename Field>
static inline void put_field(java_stack_frame &frame,
                             jclass_attr_code &code,
                             jclass_const_pool &pool)
{
    int idx = code.code[frame.pc + 1] << 8;
    idx |= code.code[frame.pc + 2];

    jfield *field = get_field<Field>(idx, pool);

    jvalue val;

    switch (field->type) {
        case jfield::flat_type::BOOLEAN:    val.z = frame.pop_param<jboolean>();    break;
        case jfield::flat_type::BYTE:       val.b = frame.pop_param<jbyte>();       break;
        case jfield::flat_type::CHAR:       val.c = frame.pop_param<jchar>();       break;
        case jfield::flat_type::SHORT:      val.s = frame.pop_param<jshort>();      break;
        case jfield::flat_type::INT:        val.i = frame.pop_param<jint>();        break;
        case jfield::flat_type::LONG:       val.j = frame.pop_param<jlong>();       break;
        case jfield::flat_type::FLOAT:      val.f = frame.pop_param<jfloat>();      break;
        case jfield::flat_type::DOUBLE:     val.d = frame.pop_param<jdouble>();     break;
        case jfield::flat_type::OBJECT:
        case jfield::flat_type::ARRAY:      val.l = frame.pop_param<jref>();        break;
    }

    if (Field::STATIC) {
        field->set_static(val);
    }
    else {
        field->set(frame.pop_param<jref>(), val);
    }
    frame.pc += 3;
}


template <typename Field>
static inline void get_field(java_stack_frame &frame,
                             jclass_attr_code &code,
                             jclass_const_pool &pool)
{
    int idx = code.code[frame.pc + 1] << 8;
    idx |= code.code[frame.pc + 2];

    jfield *field = get_field<Field>(idx, pool);

    jvalue val = Field::STATIC ? field->get_static() : field->get(frame.pop_param<jref>());

    switch (field->type) {
        case jfield::BOOLEAN:   frame.push_param<jboolean>(val.z);  break;
        case jfield::BYTE:      frame.push_param<jbyte>(val.b);     break;
        case jfield::CHAR:      frame.push_param<jchar>(val.c);     break;
        case jfield::SHORT:     frame.push_param<jshort>(val.s);    break;
        case jfield::INT:       frame.push_param<jint>(val.i);      break;
        case jfield::LONG:      frame.push_param<jlong>(val.j);     break;
        case jfield::FLOAT:     frame.push_param<jfloat>(val.f);    break;
        case jfield::DOUBLE:    frame.push_param<jdouble>(val.d);   break;
        case jfield::OBJECT:
        case jfield::ARRAY:     frame.push_param<jref>(val.l);      break;
    }
    frame.pc += 3;
}



static inline void new_array(java_stack_frame &frame, jclass_attr_code &code)
{
    int length = frame.pop_param<jint>();
    int type = code.code[frame.pc + 1];

    jref ref = nullptr;
    jarray &array = jvm::get().array;

    switch (type) {
        case 4: ref = array.new_boolean_array(length); break;
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
    frame.push_param<jref>(ref);
    frame.pc += 2;
}

static inline void a_new_array(java_stack_frame &frame,
                               jclass_const_pool &pool,
                               jclass_attr_code &code)
{
    int length = frame.pop_param<jint>();

    int index = code.code[frame.pc + 1] << 8;
    index |= code.code[frame.pc + 2];

    jclass *klass = get_class(index, pool);
    jref ref = jvm::get().array.new_object_array(klass, length);

    frame.push_param<jref>(ref);
    frame.pc += 3;
}


static inline void multi_array(java_stack_frame &frame,
                               jclass_const_pool &pool,
                               jclass_attr_code &code)
{
    int index = code.code[frame.pc] << 8;
    index |= code.code[frame.pc];
    jclass *klass = get_class(index, pool);

    u1 dimension = frame.pop_param<u1>();

    // todo: 未完全实现

    frame.pc += 3;
}

static inline void array_length(java_stack_frame &frame)
{
    jref ref = frame.pop_param<jref>();
    frame.push_param<jint>(jvm::get().array.get_array_length(ref));
    frame.pc += 1;
}


static inline void check_cast(java_stack_frame &frame,
                              jclass_const_pool &pool,
                              jclass_attr_code &code)
{
    int index = code.code[frame.pc + 1] << 8;
    index |= code.code[frame.pc + 2];
    jclass *klass = get_class(index, pool);

    jref obj = frame.top_param<jref>();

    if (jheap::cast(obj) == nullptr) {
        // 'If objectref is null, then the operand stack is unchanged.'
    }
    else if (! klass->is_instance(obj)) {
        throw_exp("java/lang/ClassCastException", klass->name);
        return;
    }
    frame.pc += 3;
}

static inline void instance_of(java_stack_frame &frame,
                              jclass_const_pool &pool,
                              jclass_attr_code &code)
{
    int index = code.code[frame.pc + 1] << 8;
    index |= code.code[frame.pc + 2];
    jclass *klass = get_class(index, pool);

    jref obj = frame.pop_param<jref>();
    frame.push_param<jint>(klass->is_instance(obj) ? 1 : 0);

    frame.pc += 3;
}






