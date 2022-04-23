
#include "jni/jni_utils.h"
#include "java/lang/String.h"

/**
 * class: java/lang/String
 * method: intern
 * sig: ()Ljava/lang/String;
 */
extern "C" JNIEXPORT jstring JNICALL
Java_java_lang_String_intern
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;
    auto ref = javsvm::jvm::get().string.intern(to_object(self));
    return to_object<jstring>(ref);
}

