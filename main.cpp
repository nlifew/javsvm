

#include <cstdio>
#include <unistd.h>

#include "vm/jvm.h"
#include "object/jclass.h"
#include "object/jfield.h"
#include "object/jmethod.h"
#include "object/jarray.h"

using namespace javsvm;

int main() {
    chdir("..");
    putenv(strdup("CLASSPATH=test"));

    printf("start\n");

    auto &array = jvm::get().array;
    jclass *klass = array.load_array_class("[[[[[[[[I");

    printf("--------------------------result--------------------------\n");
    for (jclass *cls = klass; cls != nullptr; cls = cls->component_type) {
        printf("'%s'\n", cls->name);
    }

//    jclass *cls = jvm::get().class_loader.load_class("Main2");
//
//    printf("--------------------------fields table--------------------------\n");
//    for (int i = 0; i < cls->field_table_size; i ++) {
//        printf("[%d]/[%d]: %s\n", i, cls->field_table_size, cls->field_tables[i].name);
//    }
//
//    printf("--------------------------methods table--------------------------\n");
//    for (int i = 0; i < cls->method_table_size; i ++) {
//        printf("[%d]/[%d]: %s(%s)\n", i, cls->method_table_size, cls->method_tables[i].name, cls->method_tables[i].sig);
//    }
//
//    printf("--------------------------virtual fields table--------------------------\n");
//    for (int i = 0; i < cls->vtable_size; i ++) {
//        printf("[%d]/[%d]: %s(%s)\n", i, cls->vtable_size, cls->vtable[i]->name, cls->vtable[i]->sig);
//    }
//
//    printf("--------------------------try to create array--------------------------\n");
//    jarray &jarray = jvm::get().array;
//
//    jref intArray = jarray.new_int_array(12);
//    printf("the length of int array is %d\n", jarray.get_array_length(intArray));
//
//    char buff[16 * 8] = { 0 };
//    jarray.get_int_array_region(intArray, 0, 12, (jint*) buff);
//    for (int i = 0; i < 12; i ++) {
//        printf("[%d/%d] -> [%d]\n", i + 1, 12, ((jint*) buff)[i]);
//        ((jint*) buff)[i] = i;
//    }
//    jarray.set_int_array_region(intArray, 0, 12, (jint*) buff);
//    memset(buff, 0, sizeof(buff));
//
//    jarray.get_int_array_region(intArray, 0, 12, (jint*) buff);
//    for (int i = 0; i < 12; i ++) {
//        printf("[%d/%d] -> [%d]\n", i + 1, 12, ((jint*) buff)[i]);
//        ((jint*) buff)[i] = i;
//    }
//
//

    return 0;
}
