/*
 * Copyright (c) 1994, 2014, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

package java.lang;


import java.lang.annotation.Annotation;
import java.lang.reflect.GenericDeclaration;
import java.lang.reflect.Type;
import java.lang.reflect.AnnotatedElement;
import java.lang.reflect.AnnotatedType;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Constructor;
import java.lang.reflect.TypeVariable;

import java.io.Serializable;
import java.io.InputStream;

public final class Class<T> implements Serializable, GenericDeclaration, Type, AnnotatedElement {


    private static final int ANNOTATION= 0x00002000;
    private static final int ENUM      = 0x00004000;
    private static final int SYNTHETIC = 0x00001000;

    private Class(long nativePtr) {
        mNativePtr = nativePtr;
    }

    private final long mNativePtr;

    public String toString() {
        return (isInterface() ? "interface " : (isPrimitive() ? "" : "class "))
            + getName();
    }

    public String toGenericString() {
        throw new UnsupportedOperationException("toGenericString()");
    }


    public static Class<?> forName(String className)
                throws ClassNotFoundException {
        throw new UnsupportedOperationException("forName(String)");
    }


    public static Class<?> forName(String name, boolean initialize,
                                   ClassLoader loader)
        throws ClassNotFoundException
    {
        throw new UnsupportedOperationException("forName(String, boolean, ClasLoader)");
    }



    public T newInstance()
        throws InstantiationException, IllegalAccessException
    {
        throw new UnsupportedOperationException("newInstance()");
    }

    public native boolean isInstance(Object obj);


    public native boolean isAssignableFrom(Class<?> cls);


    public native boolean isInterface();

    public native boolean isArray();


    public native boolean isPrimitive();

    public boolean isAnnotation() {
        return (getModifiers() & ANNOTATION) != 0;
    }

    public boolean isSynthetic() {
        return (getModifiers() & SYNTHETIC) != 0;
    }

    public String getName() {
        throw new UnsupportedOperationException("getName()");
    }


    public ClassLoader getClassLoader() {
        throw new UnsupportedOperationException("getClassLoader()");
    }

    public TypeVariable<Class<T>>[] getTypeParameters() {
        throw new UnsupportedOperationException("gettypeParameters");
    }

    public native Class<? super T> getSuperclass();

    public Type getGenericSuperclass() {
        throw new UnsupportedOperationException("getGenericSuperclass()");
    }

    public Package getPackage() {
        throw new UnsupportedOperationException("getPackage()");
    }

    public Class<?>[] getInterfaces() {
        throw new UnsupportedOperationException("getInterfaces()");
    }

    public Type[] getGenericInterfaces() {
        throw new UnsupportedOperationException("getGenericInterfaces()");
    }


    public native Class<?> getComponentType();


    public native int getModifiers();


    public native Object[] getSigners();


    public Method getEnclosingMethod() throws SecurityException {
        throw new UnsupportedOperationException("getEnclosingMethod()");
    }


    public Constructor<?> getEnclosingConstructor() throws SecurityException {
        throw new UnsupportedOperationException("getEnclosingConstructor()");
    }




    public Class<?> getDeclaringClass() throws SecurityException {
        throw new UnsupportedOperationException("getDeclaringClass()");
    }



    public Class<?> getEnclosingClass() throws SecurityException {
        throw new UnsupportedOperationException("getEnclosingClass()");
    }

    public String getSimpleName() {
        throw new UnsupportedOperationException("getSimpleName()");
    }


    public String getTypeName() {
        throw new UnsupportedOperationException("getTypeName()");
    }

    public String getCanonicalName() {
        throw new UnsupportedOperationException("getCanonicalName()");
    }

    public boolean isAnonymousClass() {
        throw new UnsupportedOperationException("isAnonymousClass()");
    }

    public boolean isLocalClass() {
        throw new UnsupportedOperationException("isLocalClass()");
    }


    public boolean isMemberClass() {
        throw new UnsupportedOperationException("isMemberClass()");
    }



    public Class<?>[] getClasses() {
        throw new UnsupportedOperationException("getClasses()");
    }

    public Field[] getFields() throws SecurityException {
        throw new UnsupportedOperationException("getFields()");
    }


    public Method[] getMethods() throws SecurityException {
        throw new UnsupportedOperationException("getMethods()");
    }


    public Constructor<?>[] getConstructors() throws SecurityException {
        throw new UnsupportedOperationException("getConstructors()");
    }



    public Field getField(String name)
        throws NoSuchFieldException, SecurityException {
        throw new UnsupportedOperationException("getField(String)");
    }



    public Method getMethod(String name, Class<?>... parameterTypes)
        throws NoSuchMethodException, SecurityException {
        throw new UnsupportedOperationException("getMethod(String, Class...)");
    }



    public Constructor<T> getConstructor(Class<?>... parameterTypes)
        throws NoSuchMethodException, SecurityException {
        throw new UnsupportedOperationException("getConstructor(Class...)");
    }


    public Class<?>[] getDeclaredClasses() throws SecurityException {
        throw new UnsupportedOperationException("getDeclaredClasses()");
    }


    public Field[] getDeclaredFields() throws SecurityException {
        throw new UnsupportedOperationException("getDeclaredFields()");
    }


    public Method[] getDeclaredMethods() throws SecurityException {
        throw new UnsupportedOperationException("getDeclaredMethods()");
    }


    public Constructor<?>[] getDeclaredConstructors() throws SecurityException {
        throw new UnsupportedOperationException("getDeclaredConstructors()");
    }


    public Field getDeclaredField(String name)
        throws NoSuchFieldException, SecurityException {
        throw new UnsupportedOperationException("getDeclaredField(String)");
    }


    public Method getDeclaredMethod(String name, Class<?>... parameterTypes)
        throws NoSuchMethodException, SecurityException {
        throw new UnsupportedOperationException("getDeclaredMethod(String, Class...)");
    }



    public Constructor<T> getDeclaredConstructor(Class<?>... parameterTypes)
        throws NoSuchMethodException, SecurityException {
        throw new UnsupportedOperationException("getDeclaredConstructor(Class...)");
    }


     public InputStream getResourceAsStream(String name) {
         throw new UnsupportedOperationException("getResourceAsStream(String)");
    }


    public java.net.URL getResource(String name) {
        throw new UnsupportedOperationException("getResource(String)");
    }

    public java.security.ProtectionDomain getProtectionDomain() {
        throw new UnsupportedOperationException("getProtectionDomain()");
    }


    public boolean desiredAssertionStatus() {
        throw new UnsupportedOperationException("desiredAssertionStatus()");
    }


    public boolean isEnum() {
        throw new UnsupportedOperationException("isEnum()");
    }

    public T[] getEnumConstants() {
        throw new UnsupportedOperationException("getEnumConstants()");
    }


    public T cast(Object obj) {
        throw new UnsupportedOperationException("case(Object)");
    }


    public <U> Class<? extends U> asSubclass(Class<U> clazz) {
        throw new UnsupportedOperationException("asSubclass(Class)");
    }


    public <A extends Annotation> A getAnnotation(Class<A> annotationClass) {
        throw new UnsupportedOperationException("getAnnotation(Class)");
    }


//    @Override
    public boolean isAnnotationPresent(Class<? extends Annotation> annotationClass) {
        throw new UnsupportedOperationException("isAnnotationPresent(Class)");
    }


//    @Override
    public <A extends Annotation> A[] getAnnotationsByType(Class<A> annotationClass) {
        throw new UnsupportedOperationException("getAnnotationsByType(Class)");
    }


    public Annotation[] getAnnotations() {
        throw new UnsupportedOperationException("getAnnotations()");
    }


//    @Override
    public <A extends Annotation> A getDeclaredAnnotation(Class<A> annotationClass) {
        throw new UnsupportedOperationException("getDeclaredAnnotation(Class)");
    }

//    @Override
    public <A extends Annotation> A[] getDeclaredAnnotationsByType(Class<A> annotationClass) {
        throw new UnsupportedOperationException("getDeclaredAnnotationsByType(Class)");
    }


    public Annotation[] getDeclaredAnnotations()  {
        throw new UnsupportedOperationException("getDeclaredAnnotations()");
    }


    public AnnotatedType getAnnotatedSuperclass() {
        throw new UnsupportedOperationException("getAnnotatedSuperclass()");
    }

    public AnnotatedType[] getAnnotatedInterfaces() {
        throw new UnsupportedOperationException("getAnnotatedInterfaces()");
    }

    // public boolean desiredAssertionStatus() {
    //     return false;
    // }
}
