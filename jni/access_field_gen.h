
#ifndef type
#error please define 'type'
#endif

#ifndef name
#error please define 'name'
#endif


#define _MACRO_ADD(x, y, z) x##y##z
#define MACRO_ADD(x, y, z) _MACRO_ADD(x, y, z)

/********************************* direct field begin *********************************/

static type (JNICALL MACRO_ADD(Get, name, Field))
        (JNIEnv *env, jobject obj, jfieldID field)
{
    return take_from<type>(get_field(env, obj, field));
}


static void (JNICALL MACRO_ADD(Set, name, Field))
        (JNIEnv *env, jobject obj, jfieldID field, type val)
{
    set_field(env, obj, field, pack_to(val));
}

/********************************* direct field end *********************************/

/********************************* static field begin *********************************/

static type (JNICALL MACRO_ADD(GetStatic, name, Field))
        (JNIEnv *env, jclass clazz, jfieldID field)
{
    return take_from<type>(get_static_field(env, clazz, field));
}


static void (JNICALL MACRO_ADD(SetStatic, name, Field))
        (JNIEnv *env, jclass clazz, jfieldID field, type val)
{
    set_static_field(env, clazz, field, pack_to(val));
}


/********************************* static field end *********************************/

#undef name
#undef type
#undef MACRO_ADD
#undef _MACRO_ADD

