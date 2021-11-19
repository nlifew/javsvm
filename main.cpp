

#include <cstdio>
#include <unistd.h>

#include "vm/jvm.h"
#include "object/jobject.h"
#include "object/jclass.h"
#include "object/jfield.h"
#include "object/jmethod.h"
#include "object/jarray.h"


#include <thread>

using namespace javsvm;

jobject *_obj = nullptr;

int _count = 0;

static void func()
{
    jvm::get().attach();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    _obj->lock();
    for (int i = 0; i < 1000; i ++) {
        _count ++;
    }
    _obj->unlock();
    jvm::get().detach();
}

int main() {
    chdir("..");
    putenv(strdup("CLASSPATH=test"));

    printf("start\n");

    jvm &vm = jvm::get();
    jenv &env = vm.attach();

    // 加载类
    jclass *Main = vm.bootstrap_loader.load_class("Main");

    printf("--------------------------load finish--------------------------\n");
    jref ref = Main->new_instance();
    auto obj = vm.heap.lock(ref);
    _obj = obj.get();

    std::thread *array[12];

    for (auto &i : array) {
        i = new std::thread(func);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    for (auto &i : array) {
        i->join();
        delete i;
    }

    printf("%d\n", _count);

    return 0;
}
