//
// Created by nlifew on 2020/10/16.
//

#ifndef JAVSVM_JCLASS_INTERFACE_H
#define JAVSVM_JCLASS_INTERFACE_H

namespace javsvm
{
struct jclass_interface
{
    u2 name_index = 0;

    int read_from(input_stream& r)
    {
        r >> (name_index);
        return 0;
    }
};
}

#endif //JAVSVM_JCLASS_INTERFACE_H
