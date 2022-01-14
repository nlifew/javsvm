/*
 * Copyright (c) 1996, 1998, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#ifndef _JAVASOFT_JNI_MD_H_
#define _JAVASOFT_JNI_MD_H_

/* javsvm-changed: define JNIEXPORT，JNIIMPORT，JNICALL on different platform */

#ifdef _WIN32
#define JNIEXPORT __declspec(dllexport)
#define JNIIMPORT __declspec(dllimport)
#define JNICALL __stdcall
#else
#define JNIEXPORT
#define JNIIMPORT
#define JNICALL
#endif

/* javsvm-changed: define JNIEXPORT，JNIIMPORT，JNICALL end */

/* javsvm-changed: use <stdint.h> to instead. */
#include <stdint.h>

typedef int32_t /*long*/ jint;
typedef int64_t /*__int64*/ jlong;
typedef int8_t /*signed char*/ jbyte;

/* javsvm-changed: use <stdint.h> end */

#endif /* !_JAVASOFT_JNI_MD_H_ */
