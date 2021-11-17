

#ifndef JAVSVM_JOBJECT_H
#define JAVSVM_JOBJECT_H

#include "../utils/global.h"
//#include "jlock.h"

namespace javsvm
{

struct jclass;


struct jobject
{
    int flag = 0;
//    jlock lock;
    jclass *klass = nullptr;
    char values[0];
};

} // namespace javsvm


#endif
