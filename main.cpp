

#include <cstdio>
#include <unistd.h>

#include "vm/jvm.h"
#include "object/jobject.h"
#include "object/jlock.h"
#include "object/jclass.h"
#include "object/jfield.h"
#include "object/jmethod.h"
#include "object/jarray.h"
#include "engine/engine.h"

#include <thread>
#include <queue>

using namespace javsvm;


int main() {
    chdir("..");
    putenv(strdup("CLASSPATH=test"));

    printf("start\n");
    jvm &vm = jvm::get();
    vm.attach();

    jclass *Main = vm.bootstrap_loader.load_class("Main");
    jmethod *msort = Main->get_static_method("msort", "([III[I)V");
    jmethod *qsort = Main->get_static_method("qsort", "([III)V");

    jref array = vm.array.new_int_array(10);
    int array_int[] = { 5, 8, 4, 0, 1, 3, 6, 9, 2, 7 };
    vm.array.set_array_region(array, 0, 10, array_int);

    slot_t args[4];
    memset(args, 0, sizeof(args));

    *(jref*) (args + 0) = array;
    *(jint*) (args + 1) = 0;
    *(jint*) (args + 2) = 9;
    *(jref*) (args + 3) = nullptr;

    jargs args_w(args);
    run_java(qsort, nullptr, args_w);

    memset(array_int, 0, sizeof(int) * 10);
    vm.array.get_array_region(array, 0, 10, array_int);


    for (int i : array_int) {
        LOGI("%d\n", i);
    }

    return 0;
}
