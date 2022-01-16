//
// Created by edz on 2021/12/7.
//
#include "vm/jvm.h"
#include "object/jclass.h"
#include "engine/engine.h"
#include "utils/log.h"


using namespace javsvm;


static void sort(jmethod *method)
{
    auto &vm = jvm::get();

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
    run_java(method, nullptr, args_w);

    memset(array_int, 0, sizeof(int) * 10);
    vm.array.get_array_region(array, 0, 10, array_int);

    for (int i : array_int) {
        LOGW("%d\n", i);
    }
}

int main()
{
    putenv(strdup("CLASSPATH=../../../jdk"));

    LOGI("main: start\n");

    jvm &vm = jvm::get();
    vm.attach();

    jclass *Main = vm.bootstrap_loader.load_class("Main");
    assert(Main != nullptr);

    jmethod *qsort = Main->get_static_method("qsort", "([III)V");
    assert(qsort != nullptr);

    sort(qsort);

    jmethod *msort = Main->get_static_method("msort", "([III[I)V");
    assert(msort != nullptr);

    sort(msort);

    return 0;
}