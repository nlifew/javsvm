


static jfieldID (JNICALL GetFieldID)
        (JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
    auto _clazz = to_class(clazz);
    if (_clazz == nullptr || name == nullptr || sig == nullptr) {
        return nullptr;
    }

    return (jfieldID) _clazz->get_field(name, sig);
}


static inline javsvm::jvalue get_field(JNIEnv *, jobject obj, jfieldID field)
{
    auto _field = (javsvm::jfield *) field;
    if (_field == nullptr) {
        return { .j = 0 };
    }

    javsvm::jref _obj = to_object(obj);

    // openjdk8 并没有对此做校验，但我们需要
    if (! _field->clazz->is_instance(_obj)) {
        javsvm::throw_exp("java/lang/IncompatibleClassChangeError", "");
        return { .j = 0 };
    }

    return _field->get(_obj);
}

static inline void set_field(JNIEnv *, jobject obj, jfieldID field, javsvm::jvalue val)
{
    auto _field = (javsvm::jfield *) field;
    if (_field == nullptr) {
        return;
    }

    javsvm::jref _obj = to_object(obj);

    // openjdk8 并没有对此做校验，但我们需要
    if (! _field->clazz->is_instance(_obj)) {
        javsvm::throw_exp("java/lang/IncompatibleClassChangeError", "");
        return;
    }
    _field->set(_obj, val);
}



static jfieldID (JNICALL GetStaticFieldID)
        (JNIEnv *, jclass clazz, const char *name, const char *sig)
{
    auto _clazz = to_class(clazz);
    if (_clazz == nullptr || name == nullptr || sig == nullptr) {
        return nullptr;
    }

    return (jfieldID) _clazz->get_static_field(name, sig);
}

static inline javsvm::jvalue get_static_field(JNIEnv *, jclass, jfieldID field)
{
    auto _field = (javsvm::jfield *) field;
    if (_field == nullptr) {
        return { .j = 0 };
    }

    return _field->get(nullptr);
}

static inline void set_static_field(JNIEnv *, jclass, jfieldID field, javsvm::jvalue val)
{
    auto _field = (javsvm::jfield *) field;
    if (_field == nullptr) {
        return;
    }
    _field->set(nullptr, *(javsvm::jvalue *) &val);
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
