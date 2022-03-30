
#include "jni_utils.h"
#include "jni_env.h"

#include "../object/jfield.h"

namespace jni
{

jfieldID (JNICALL GetFieldID)
        (JNIEnv *, jclass clazz, const char *name, const char *sig) {
    safety_area_guard guard;

    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return nullptr;
    }

    return (jfieldID) _clazz->get_field(name, sig);
}


static inline javsvm::jvalue get_field(jobject obj, jfieldID field) {
    auto _field = (javsvm::jfield *) field;
    if (_field == nullptr) {
        return {.j = 0};
    }

    javsvm::jref _obj = to_object(obj);

    // 不做校验，直接返回
    return _field->get(_obj);
}

static inline void set_field(jobject obj, jfieldID field, javsvm::jvalue val) {
    auto _field = (javsvm::jfield *) field;
    if (_field == nullptr) {
        return;
    }

    javsvm::jref _obj = to_object(obj);

    // 不做校验，直接访问
    _field->set(_obj, val);
}


jfieldID (JNICALL GetStaticFieldID)
        (JNIEnv *, jclass clazz, const char *name, const char *sig) {
    safety_area_guard guard;

    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return nullptr;
    }
    return (jfieldID) _clazz->get_static_field(name, sig);
}

static inline javsvm::jvalue get_static_field(jfieldID field) {
    auto _field = (javsvm::jfield *) field;
    if (_field == nullptr) {
        return {.j = 0};
    }

    return _field->get_static();
}

static inline void set_static_field(jfieldID field, javsvm::jvalue val) {
    auto _field = (javsvm::jfield *) field;
    if (_field == nullptr) {
        return;
    }
    _field->set_static(val);
}


#define type jobject
#define name Object

#include "access_field_gen.h"

#define type jboolean
#define name Boolean
#include "access_field_gen.h"


#define type jbyte
#define name Byte
#include "access_field_gen.h"


#define type jchar
#define name Char
#include "access_field_gen.h"


#define type jshort
#define name Short
#include "access_field_gen.h"


#define type jint
#define name Int
#include "access_field_gen.h"


#define type jlong
#define name Long
#include "access_field_gen.h"


#define type jfloat
#define name Float
#include "access_field_gen.h"


#define type jdouble
#define name Double
#include "access_field_gen.h"

}