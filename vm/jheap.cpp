//
// Created by edz on 2021/11/14.
//

#include "jheap.h"
#include "../object/jobject.h"

#include <cstdlib>

using namespace javsvm;


jobject_ptr jheap::lock(jref ref)
{
    auto *ptr = (jobject *) ref;
    return jobject_ptr(*this, ptr);
}


void jheap::unlock(jobject *ptr)
{
}

bool jheap::is_nullptr(jref ref)
{
    auto *ptr = (jobject *) ref;
    return ptr == nullptr;
}

jref jheap::malloc_bytes(int bytes)
{
    auto *obj = (jobject*) malloc(bytes * sizeof(jobject));
    memset(obj, 0, bytes * sizeof(jobject));
    return (jref) obj;
}
