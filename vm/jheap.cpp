//
// Created by edz on 2021/11/14.
//

#include "jheap.h"
#include "../object/jobject.h"

#include <cstdlib>
#include <new>

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
    return lock(ref) == nullptr;
}

jref jheap::malloc_bytes(int bytes)
{
    void *ptr = malloc(bytes + sizeof(jobject));
    memset(ptr, 0, bytes + sizeof(jobject));
    new (ptr) jobject;
    return (jref) ptr;
}
