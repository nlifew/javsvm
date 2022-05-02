
#include "jni/jni_utils.h"
#include "java/lang/Float.h"

/**
 * class: java/lang/Float
 * method: floatToRawIntBits
 * sig: (F)I
 */
extern "C" JNIEXPORT jint JNICALL
Java_java_lang_Float_floatToRawIntBits
    (JNIEnv *, jclass, jfloat f)
{
    return *(jint *) &f;
}

/**
 * class: java/lang/Float
 * method: intBitsToFloat
 * sig: (I)F
 */
extern "C" JNIEXPORT jfloat JNICALL
Java_java_lang_Float_intBitsToFloat
    (JNIEnv *, jclass, jint in)
{
    return *(jfloat *) &in;
}

