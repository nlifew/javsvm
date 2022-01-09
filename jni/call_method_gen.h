




#ifndef return_type
#error please define 'return_type'
#endif

#ifndef name
#error please define 'name'
#endif


#define _MICRO_ADD(x, y, z) x##y##z
#define MACRO_ADD(x, y, z) _MICRO_ADD(x, y, z)

/********************************* direct method begin *********************************/

static return_type (JNICALL MACRO_ADD(Call, name, MethodA))
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    auto _args = to_args(methodID, args);
    jvalue value = call_direct_method(env, obj, methodID, _args);
    delete[] _args;
    auto ret = take_from<return_type>(value);
    return ret;
}


static return_type (JNICALL MACRO_ADD(Call, name, MethodV))
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto _args = to_args(methodID, args);
    jvalue value = call_direct_method(env, obj, methodID, _args);
    delete[] _args;
    auto ret = take_from<return_type>(value);
    return ret;
}

static return_type (JNICALL MACRO_ADD(Call, name, Method))
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = MACRO_ADD(Call, name, MethodV)(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}

/********************************* direct method end *********************************/

/********************************* static method begin *********************************/

static return_type (JNICALL MACRO_ADD(CallStatic, name, MethodA))
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
    auto _args = to_args(methodID, args);
    jvalue value = call_static_method(env, clazz, methodID, _args);
    auto ret = take_from<return_type>(value);
    delete[] _args;
    return ret;
}


static return_type (JNICALL MACRO_ADD(CallStatic, name, MethodV))
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
    auto _args = to_args(methodID, args);
    jvalue value = call_static_method(env, clazz, methodID, _args);
    auto ret = take_from<return_type>(value);
    delete[] _args;
    return ret;
}

static return_type (JNICALL MACRO_ADD(CallStatic, name, Method))
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = MACRO_ADD(CallStatic, name, MethodV)(env, clazz, methodID, ap);
    va_end(ap);
    return ret;
}

/********************************* static method end *********************************/

/***************************** non virtual method begin *****************************/


static return_type (JNICALL MACRO_ADD(CallNonvirtual, name, MethodA))
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
    auto _args = to_args(methodID, args);
    jvalue value = call_nonvirtual_method(env, obj, clazz, methodID, _args);
    auto ret = take_from<return_type>(value);
    delete[] _args;
    return ret;
}


static return_type (JNICALL MACRO_ADD(CallNonvirtual, name, MethodV))
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
    auto _args = to_args(methodID, args);
    jvalue value = call_nonvirtual_method(env, obj, clazz, methodID, _args);
    auto ret = take_from<return_type>(value);
    delete[] _args;
    return ret;
}

static return_type (JNICALL MACRO_ADD(CallNonvirtual, name, Method))
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = MACRO_ADD(CallNonvirtual, name, MethodV)(env, obj, clazz, methodID, ap);
    va_end(ap);
    return ret;
}

/***************************** non virtual method end *****************************/


#undef return_type
#undef name
#undef _MICRO_ADD
#undef MICRO_ADD
