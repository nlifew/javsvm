

#ifndef type
#error please define 'type'
#endif

#ifndef name
#error please define 'name'
#endif


#define _MACRO_ADD(x, y, z) x##y##z
#define _MACRO_ADD2(x, y) x##y
#define MACRO_ADD2(x, y) _MACRO_ADD2(x, y)
#define MACRO_ADD(x, y, z) _MACRO_ADD(x, y, z)

#define array_type MACRO_ADD(j, type, Array)
#define pointer_type MACRO_ADD2(j, type)

static array_type (JNICALL MACRO_ADD(New, name, Array))
        (JNIEnv *, jsize len)
{
    MACRO_ADD(auto array = javsvm::jvm::get().array.new_, type, _array(len));
    return (array_type) to_object(array);
}


static pointer_type * (JNICALL MACRO_ADD(Get, name, ArrayElements))
        (JNIEnv *env, array_type array, jboolean *isCopy)
{
    // 如果数组长度多于 1024 字节，返回直接引用；否则返回拷贝
    auto len = GetArrayLength(env, array);
    if (len > 1024) {
        return (pointer_type *) GetPrimitiveArrayCritical(env, array, isCopy);
    }

    if (isCopy) *isCopy = true;

    auto copy = new pointer_type[len];
    javsvm::jvm::get().array.get_array_region(to_object(array), 0, len, copy);
    return copy;
}


static void (JNICALL MACRO_ADD(Release, name, ArrayElements))
        (JNIEnv *env, array_type array, pointer_type *ptr, jint mode)
{
    // 如果数组长度多于 1024 字节，返回直接引用；否则返回拷贝
    auto len = GetArrayLength(env, array);
    if (len > 1024) {
        ReleasePrimitiveArrayCritical(env, array, ptr, mode);
        return;
    }
    switch (mode) {
        case 0:             /* 复制内容并释放elems缓冲区 */
            javsvm::jvm::get().array.set_array_region(to_object(array), 0, len, ptr);
            delete[] array;
            break;
        case JNI_COMMIT:    /* 复制内容但不释放elems缓冲区 */
            javsvm::jvm::get().array.set_array_region(to_object(array), 0, len, ptr);
            break;
        case JNI_ABORT:     /* 释放缓冲区而不复制可能的更改 */
            delete[] array;
            break;
        default:
#define _(x) #x
            LOGE("Release%sArrayElements: unknown mode: %d\n", _(type), mode);
#undef _
            break;
    }
}


static void (JNICALL MACRO_ADD(Get, name, ArrayRegion))
        (JNIEnv *, array_type array, jsize start, jsize l, pointer_type *buf)
{
    javsvm::jvm::get().array.get_array_region(to_object(array), start, l, buf);
}


static void (JNICALL MACRO_ADD(Set, name, ArrayRegion))
        (JNIEnv *, array_type array, jsize start, jsize l, const pointer_type *buf)
{
    javsvm::jvm::get().array.set_array_region(to_object(array), start, l, buf);
}


#undef name
#undef type
#undef MACRO_ADD
#undef _MACRO_ADD
#undef MACRO_ADD2
#undef _MACRO_ADD2
#undef array_type
#undef pointer_type

