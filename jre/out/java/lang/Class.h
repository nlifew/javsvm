/**
 * auto generated by javsvm-javah, don't modify it.
 */

#ifndef JAVA_JAVA_LANG_CLASS_H
#define JAVA_JAVA_LANG_CLASS_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * class: java/lang/Class
 * method: desiredAssertionStatus0
 * sig: (Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL
Java_java_lang_Class_desiredAssertionStatus0
    (JNIEnv *env, jclass klass, jclass);

/**
 * class: java/lang/Class
 * method: forName0
 * sig: (Ljava/lang/String;ZLjava/lang/ClassLoader;Ljava/lang/Class;)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL
Java_java_lang_Class_forName0
    (JNIEnv *env, jclass klass, jstring, jboolean, jobject, jclass);

/**
 * class: java/lang/Class
 * method: getAnnotatedInterfaces
 * sig: ()[Ljava/lang/reflect/AnnotatedType;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getAnnotatedInterfaces
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getAnnotatedSuperclass
 * sig: ()Ljava/lang/reflect/AnnotatedType;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getAnnotatedSuperclass
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getAnnotation
 * sig: (Ljava/lang/Class;)Ljava/lang/annotation/Annotation;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getAnnotation
    (JNIEnv *env, jobject self, jclass);

/**
 * class: java/lang/Class
 * method: getAnnotations
 * sig: ()[Ljava/lang/annotation/Annotation;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getAnnotations
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getAnnotationsByType
 * sig: (Ljava/lang/Class;)[Ljava/lang/annotation/Annotation;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getAnnotationsByType
    (JNIEnv *env, jobject self, jclass);

/**
 * class: java/lang/Class
 * method: getClassLoader0
 * sig: ()Ljava/lang/ClassLoader;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getClassLoader0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getClasses0
 * sig: ()[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getClasses0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getComponentType
 * sig: ()Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL
Java_java_lang_Class_getComponentType
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getConstructor0
 * sig: ([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getConstructor0
    (JNIEnv *env, jobject self, jobjectArray);

/**
 * class: java/lang/Class
 * method: getConstructors0
 * sig: ()[Ljava/lang/reflect/Constructor;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getConstructors0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getDeclaredAnnotation
 * sig: (Ljava/lang/Class;)Ljava/lang/annotation/Annotation;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getDeclaredAnnotation
    (JNIEnv *env, jobject self, jclass);

/**
 * class: java/lang/Class
 * method: getDeclaredAnnotations
 * sig: ()[Ljava/lang/annotation/Annotation;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredAnnotations
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getDeclaredAnnotationsByType
 * sig: (Ljava/lang/Class;)[Ljava/lang/annotation/Annotation;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredAnnotationsByType
    (JNIEnv *env, jobject self, jclass);

/**
 * class: java/lang/Class
 * method: getDeclaredClasses0
 * sig: ()[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredClasses0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getDeclaredConstructor0
 * sig: ([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getDeclaredConstructor0
    (JNIEnv *env, jobject self, jobjectArray);

/**
 * class: java/lang/Class
 * method: getDeclaredConstructors0
 * sig: ()[Ljava/lang/reflect/Constructor;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredConstructors0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getDeclaredField0
 * sig: (Ljava/lang/String;)Ljava/lang/reflect/Field;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getDeclaredField0
    (JNIEnv *env, jobject self, jstring);

/**
 * class: java/lang/Class
 * method: getDeclaredFields0
 * sig: ()[Ljava/lang/reflect/Field;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredFields0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getDeclaredMethod0
 * sig: (Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getDeclaredMethod0
    (JNIEnv *env, jobject self, jstring, jobjectArray);

/**
 * class: java/lang/Class
 * method: getDeclaredMethods0
 * sig: ()[Ljava/lang/reflect/Method;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredMethods0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getDeclaringClass0
 * sig: ()Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL
Java_java_lang_Class_getDeclaringClass0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getEnclosingClass
 * sig: ()Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL
Java_java_lang_Class_getEnclosingClass
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getEnclosingConstructor
 * sig: ()Ljava/lang/reflect/Constructor;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getEnclosingConstructor
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getEnclosingMethod
 * sig: ()Ljava/lang/reflect/Method;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getEnclosingMethod
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getField0
 * sig: (Ljava/lang/String;)Ljava/lang/reflect/Field;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getField0
    (JNIEnv *env, jobject self, jstring);

/**
 * class: java/lang/Class
 * method: getFields0
 * sig: ()[Ljava/lang/reflect/Field;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getFields0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getGenericInterfaces
 * sig: ()[Ljava/lang/reflect/Type;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getGenericInterfaces
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getGenericSuperclass
 * sig: ()Ljava/lang/reflect/Type;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getGenericSuperclass
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getInterfaces
 * sig: ()[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getInterfaces
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getMethod0
 * sig: (Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_getMethod0
    (JNIEnv *env, jobject self, jstring, jobjectArray);

/**
 * class: java/lang/Class
 * method: getMethods0
 * sig: ()[Ljava/lang/reflect/Method;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getMethods0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getModifiers0
 * sig: ()I
 */
JNIEXPORT jint JNICALL
Java_java_lang_Class_getModifiers0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getName0
 * sig: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_java_lang_Class_getName0
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getPrimitiveClass
 * sig: (Ljava/lang/String;)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL
Java_java_lang_Class_getPrimitiveClass
    (JNIEnv *env, jclass klass, jstring);

/**
 * class: java/lang/Class
 * method: getSuperclass
 * sig: ()Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL
Java_java_lang_Class_getSuperclass
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: getTypeParameters
 * sig: ()[Ljava/lang/reflect/TypeVariable;
 */
JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getTypeParameters
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: isArray
 * sig: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_java_lang_Class_isArray
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: isAssignableFrom
 * sig: (Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL
Java_java_lang_Class_isAssignableFrom
    (JNIEnv *env, jobject self, jclass);

/**
 * class: java/lang/Class
 * method: isInstance
 * sig: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL
Java_java_lang_Class_isInstance
    (JNIEnv *env, jobject self, jobject);

/**
 * class: java/lang/Class
 * method: isPrimitive
 * sig: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_java_lang_Class_isPrimitive
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Class
 * method: newInstance0
 * sig: ()Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_newInstance0
    (JNIEnv *env, jobject self);



#ifdef __cplusplus
}
#endif
#endif // JAVA_JAVA_LANG_CLASS_H
