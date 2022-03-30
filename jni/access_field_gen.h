
#ifndef type
#error please define 'type'
#endif

#ifndef name
#error please define 'name'
#endif


#define _MACRO_ADD(x, y, z) x##y##z
#define MACRO_ADD(x, y, z) _MACRO_ADD(x, y, z)

/********************************* direct field begin *********************************/

type (JNICALL MACRO_ADD(Get, name, Field))
        (JNIEnv *, jobject obj, jfieldID field)
{
    safety_area_guard guard;
    return take_from<type>(get_field(obj, field));
}


void (JNICALL MACRO_ADD(Set, name, Field))
        (JNIEnv *, jobject obj, jfieldID field, type val)
{
    safety_area_guard guard;
    set_field(obj, field, pack_to(val));
}

/********************************* direct field end *********************************/

/********************************* field begin *********************************/

type (JNICALL MACRO_ADD(GetStatic, name, Field))
        (JNIEnv *, jclass, jfieldID field)
{
    safety_area_guard guard;
    return take_from<type>(get_static_field(field));
}


void (JNICALL MACRO_ADD(SetStatic, name, Field))
        (JNIEnv *, jclass, jfieldID field, type val)
{
    safety_area_guard guard;
    set_static_field(field, pack_to(val));
}


/********************************* field end *********************************/

#undef name
#undef type
#undef MACRO_ADD
#undef _MACRO_ADD

