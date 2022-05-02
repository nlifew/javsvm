

#include "jni/jni_utils.h"
#include "java/lang/Class.h"
#include "object/jfield.h"


#include <unordered_set>


static javsvm::jclass *class_of(javsvm::jref ref) noexcept
{
    static javsvm::jfield *mNativePtr = nullptr;
    if (mNativePtr == nullptr) {
        mNativePtr = javsvm::jvm::get()
                .bootstrap_loader
                .load_class("java/lang/Class")
                ->get_field("mNativePtr", "J");
    }
    return (javsvm::jclass *) mNativePtr->get(ref).j;
}

static inline javsvm::jclass *class_of(::jobject klass) noexcept
{
    return class_of(to_object(klass));
}

static std::string gen_method_sig(jobjectArray class_array) noexcept
{
    auto ptr = javsvm::jheap::cast(to_object(class_array));

    int size, ele_size;
    auto array = (javsvm::jref *) javsvm::jarray::storage_of(ptr, &size, &ele_size);

    std::unordered_map<std::string, char> primitive_types = {
            {"boolean", 'Z' },
            {"byte",    'B' },
            {"char",    'C' },
            {"short",   'S' },
            {"int",     'I' },
            {"long",    'J' },
            {"float",   'F' },
            { "double", 'D' },
    };

    std::string sig;
    sig += "(";
    for (int i = 0; i < size; ++i) {
        auto klass = class_of(array[i]);
        while (klass->component_type != nullptr) {
            sig += "[";
            klass = klass->component_type;
        }
        const auto &it = primitive_types.find(klass->name);
        if (it != primitive_types.end()) {
            sig += it->second;
        }
        else {
            sig += "L";
            sig += klass->name;
            sig += ";";
        }
    }
    sig += ")";
    return sig;
}


/**
 * class: java/lang/Class
 * method: desiredAssertionStatus0
 * sig: (Ljava/lang/Class;)Z
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_java_lang_Class_desiredAssertionStatus0
    (JNIEnv *, jclass, jclass)
{
#ifdef NDEBUG
    return false;
#else
    return true;
#endif
}

/**
 * class: java/lang/Class
 * method: forName0
 * sig: (Ljava/lang/String;ZLjava/lang/ClassLoader;Ljava/lang/Class;)Ljava/lang/Class;
 */
extern "C" JNIEXPORT jclass JNICALL
Java_java_lang_Class_forName0
        (JNIEnv *, jclass, jstring name,
         jboolean initialize, jobject loader, jclass)
{
    safety_area_guard guard;

    auto _name = javsvm::jstring::utf8(to_object(name));
    std::unique_ptr<const char, void(*)(const char *)> name_guard(
            _name, [](const char *ptr) { delete[] ptr; });

    if (javsvm::check_exception() != nullptr) {
        return nullptr;
    }

    // 替换 '.' 为 '/'
    for (int i = 0; _name[i] != '\0'; ++i) {
        if (_name[i] == '.') {
            const_cast<char*>(_name)[i] = '/';
        }
    }

    javsvm::jref result = nullptr;

    if (loader == nullptr) {
        // 使用系统类加载器
        auto klass = javsvm::jvm::get().bootstrap_loader.load_class(_name);
        if (klass != nullptr) {
            result = klass->object.get();
        }
    }
    else {
        // 直接调用 ClassLoader.loadClass(String)
        auto java_lang_ClassLoader = javsvm::jvm::get().bootstrap_loader
                .load_class("java/lang/ClassLoader");
        auto loadClass = java_lang_ClassLoader->get_method(
                "loadClass",
                "(Ljava/lang/String;Z)Ljava/lang/Class;");

        javsvm::slot_t _args[3];
        javsvm::jargs args = _args;

        args.next<javsvm::jref>() = to_object(loader);
        args.next<javsvm::jref>() = to_object(name);
        args.next<javsvm::jboolean>() = initialize;
        result = loadClass->invoke_virtual(to_object(loader), args).l;
    }
    if (result == nullptr) {
        javsvm::throw_exp(ClassNotFoundException, _name);
        return nullptr;
    }
    return to_object<jclass>(result);
}

/**
 * class: java/lang/Class
 * method: getAnnotatedInterfaces
 * sig: ()[Ljava/lang/reflect/AnnotatedType;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getAnnotatedInterfaces
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getAnnotatedInterfaces()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getAnnotatedSuperclass
 * sig: ()Ljava/lang/reflect/AnnotatedType;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getAnnotatedSuperclass
    (JNIEnv *, jobject self)
{
// todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getAnnotatedSuperclass()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getAnnotation
 * sig: (Ljava/lang/Class;)Ljava/lang/annotation/Annotation;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getAnnotation
    (JNIEnv *, jobject self, jclass)
{
// todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getAnnotation()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getAnnotations
 * sig: ()[Ljava/lang/annotation/Annotation;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getAnnotations
    (JNIEnv *, jobject self)
{
// todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getAnnotations()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getAnnotationsByType
 * sig: (Ljava/lang/Class;)[Ljava/lang/annotation/Annotation;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getAnnotationsByType
    (JNIEnv *, jobject self, jclass)
{
// todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getAnnotationByType()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getClassLoader0
 * sig: ()Ljava/lang/ClassLoader;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getClassLoader0
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;
    auto klass = class_of(self);
    return to_object(klass->loader.get());
}

/**
 * class: java/lang/Class
 * method: getClasses0
 * sig: ()[Ljava/lang/Class;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getClasses0
    (JNIEnv *, jobject self)
{
// todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getClasses0()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getComponentType
 * sig: ()Ljava/lang/Class;
 */
extern "C" JNIEXPORT jclass JNICALL
Java_java_lang_Class_getComponentType
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;
    auto component = class_of(self)->component_type;
    return component == nullptr ? nullptr :
           to_object<jclass>(component->object.get());
}

/**
 * class: java/lang/Class
 * method: getConstructor0
 * sig: ([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getConstructor0
    (JNIEnv *, jobject self, jobjectArray args)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getConstructor0()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getConstructors0
 * sig: ()[Ljava/lang/reflect/Constructor;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getConstructors0
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getConstructors0()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaredAnnotation
 * sig: (Ljava/lang/Class;)Ljava/lang/annotation/Annotation;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getDeclaredAnnotation
    (JNIEnv *, jobject self, jclass)
{
// todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getDeclaredAnnotation()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaredAnnotations
 * sig: ()[Ljava/lang/annotation/Annotation;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredAnnotations
    (JNIEnv *, jobject self)
{
// todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getDeclaredAnnotations()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaredAnnotationsByType
 * sig: (Ljava/lang/Class;)[Ljava/lang/annotation/Annotation;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredAnnotationsByType
    (JNIEnv *, jobject self, jclass)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getDeclaredAnnotationsByType()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaredClasses0
 * sig: ()[Ljava/lang/Class;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredClasses0
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getDeclaredClasses0()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaredConstructor0
 * sig: ([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getDeclaredConstructor0
    (JNIEnv *, jobject self, jobjectArray)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getDeclaredConstructor0()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaredConstructors0
 * sig: ()[Ljava/lang/reflect/Constructor;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredConstructors0
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getDeclaredConstructors0()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaredField0
 * sig: (Ljava/lang/String;)Ljava/lang/reflect/Field;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getDeclaredField0
    (JNIEnv *, jobject self, jstring)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getDeclaredField0()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaredFields0
 * sig: ()[Ljava/lang/reflect/Field;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredFields0
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getDeclaredFields0()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaredMethod0
 * sig: (Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getDeclaredMethod0
    (JNIEnv *, jobject self, jstring, jobjectArray)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaredMethods0
 * sig: ()[Ljava/lang/reflect/Method;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getDeclaredMethods0
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getDeclaringClass0
 * sig: ()Ljava/lang/Class;
 */
extern "C" JNIEXPORT jclass JNICALL
Java_java_lang_Class_getDeclaringClass0
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getEnclosingClass
 * sig: ()Ljava/lang/Class;
 */
extern "C" JNIEXPORT jclass JNICALL
Java_java_lang_Class_getEnclosingClass
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getEnclosingConstructor
 * sig: ()Ljava/lang/reflect/Constructor;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getEnclosingConstructor
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;

}

/**
 * class: java/lang/Class
 * method: getEnclosingMethod
 * sig: ()Ljava/lang/reflect/Method;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getEnclosingMethod
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;

}

/**
 * class: java/lang/Class
 * method: getField0
 * sig: (Ljava/lang/String;)Ljava/lang/reflect/Field;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getField0
    (JNIEnv *, jobject self, jstring)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;

}

/**
 * class: java/lang/Class
 * method: getFields0
 * sig: ()[Ljava/lang/reflect/Field;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getFields0
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;

}

/**
 * class: java/lang/Class
 * method: getGenericInterfaces
 * sig: ()[Ljava/lang/reflect/Type;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getGenericInterfaces
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;

}

/**
 * class: java/lang/Class
 * method: getGenericSuperclass
 * sig: ()Ljava/lang/reflect/Type;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getGenericSuperclass
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getInterfaces
 * sig: ()[Ljava/lang/Class;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getInterfaces
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;

}

/**
 * class: java/lang/Class
 * method: getMethod0
 * sig: (Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Class_getMethod0
    (JNIEnv *, jobject self, jstring, jobjectArray)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;

}

/**
 * class: java/lang/Class
 * method: getMethods0
 * sig: ()[Ljava/lang/reflect/Method;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getMethods0
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: getModifiers0
 * sig: ()I
 */
extern "C" JNIEXPORT jint JNICALL
Java_java_lang_Class_getModifiers0
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;
    return (jint) class_of(self)->access_flag;
}

/**
 * class: java/lang/Class
 * method: getName0
 * sig: ()Ljava/lang/String;
 */
extern "C" JNIEXPORT jstring JNICALL
Java_java_lang_Class_getName0
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;
    auto klass = class_of(self);
    auto name = javsvm::jvm::get().string.new_string(klass->name);
    return to_object<jstring>(name);
}

/**
 * class: java/lang/Class
 * method: getPrimitiveClass
 * sig: (Ljava/lang/String;)Ljava/lang/Class;
 */
extern "C" JNIEXPORT jclass JNICALL
Java_java_lang_Class_getPrimitiveClass
    (JNIEnv *, jclass, jstring str)
{
    safety_area_guard guard;
    auto name = javsvm::jstring::utf8(to_object(str));
    std::unique_ptr<const char, void(*)(const char *)> name_guard(
            name, [](const char *ptr) { delete[] ptr; });

    if (javsvm::check_exception() != nullptr) {
        return nullptr;
    }
    if (! javsvm::bootstrap_loader::is_primitive_type(name)) {
        char buff[512];
        snprintf(buff, sizeof(buff), "type '%s' is NOT primitive class", name);
        javsvm::throw_exp(IllegalArgumentException, buff);
        return nullptr;
    }
    auto klass = javsvm::jvm::get().bootstrap_loader.load_class(name);
    assert(klass != nullptr);
    return to_object<jclass>(klass->object.get());
}

/**
 * class: java/lang/Class
 * method: getSuperclass
 * sig: ()Ljava/lang/Class;
 */
extern "C" JNIEXPORT jclass JNICALL
Java_java_lang_Class_getSuperclass
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;

    auto klass = class_of(self)->super_class;
    if ((klass->access_flag & javsvm::jclass_file::ACC_INTERFACE) != 0) {
        return nullptr;
    }
    if (klass->super_class == nullptr) {
        return nullptr;
    }
    return to_object<jclass>(klass->super_class->object.get());
}

/**
 * class: java/lang/Class
 * method: getTypeParameters
 * sig: ()[Ljava/lang/reflect/TypeVariable;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Class_getTypeParameters
    (JNIEnv *, jobject self)
{
    // todo
    safety_area_guard guard;
    javsvm::throw_err(UnsupportedOperationException, "getTypeParameters()");
    return nullptr;
}

/**
 * class: java/lang/Class
 * method: isArray
 * sig: ()Z
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_java_lang_Class_isArray
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;
    auto klass = class_of(self);
    return klass->component_type != nullptr;
}

/**
 * class: java/lang/Class
 * method: isAssignableFrom
 * sig: (Ljava/lang/Class;)Z
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_java_lang_Class_isAssignableFrom
    (JNIEnv *, jobject self, jclass sub)
{
    safety_area_guard guard;
    auto parent = class_of(self);
    auto child = class_of(sub);
    return parent->is_assign_from(child);
}

/**
 * class: java/lang/Class
 * method: isInstance
 * sig: (Ljava/lang/Object;)Z
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_java_lang_Class_isInstance
    (JNIEnv *, jobject self, jobject obj)
{
    safety_area_guard guard;
    auto parent = class_of(self);
    auto instance = to_object(obj);
    return parent->is_instance(instance);
}

/**
 * class: java/lang/Class
 * method: isPrimitive
 * sig: ()Z
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_java_lang_Class_isPrimitive
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;
    auto klass = class_of(self);
    return javsvm::bootstrap_loader::is_primitive_type(klass->name);
}



/**
 * class: java/lang/Class
 * method: newInstance0
 * sig: ()Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Class_newInstance0
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;

    auto klass = class_of(self);

    javsvm::jmethod m = {
            .name = "<init>",
            .sig = "()V"
    };

    using cmp_t = int(*)(const void *, const void *);
    auto result = (javsvm::jmethod *) bsearch(
            &m,klass->method_tables,
            klass->method_table_size,sizeof(javsvm::jmethod),
            (cmp_t) javsvm::jmethod::compare_to);
    if (result == nullptr) {
        javsvm::throw_exp(InstantiationException, klass->name);
        return nullptr;
    }
    auto obj = klass->new_instance(result);
    if (javsvm::check_exception()) {
        javsvm::throw_exp(InstantiationException, klass->name);
        return nullptr;
    }
    return to_object(obj);
}
