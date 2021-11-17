//
// Created by nlifew on 2020/10/16.
//

#ifndef JAVSVM_JCLASS_HEADER_H
#define JAVSVM_JCLASS_HEADER_H


namespace javsvm
{

struct jclass_header
{
    static constexpr u4 MAGIC           = 0xCAFEBABE;
    static constexpr u4 MAJOR_VERSION   = 0x34;
    static constexpr u4 MINOR_VERSION   = 0;

    u4 magic = 0;
    u2 minor_version = 0;
    u2 major_version = 0;

    int read_from(input_stream& r)
    {
        r >> magic;
        if (magic != MAGIC) {
            PLOGE("invalid magic %#x, expected %#x\n", magic, MAGIC);
            return -1;
        }

        r >> minor_version;
        if (minor_version != MINOR_VERSION) {
            PLOGE("invalid minor_version %d, expected %d\n", minor_version, MINOR_VERSION);
            return -1;
        }

        r >> major_version;
        if (major_version != MAJOR_VERSION) {
            PLOGE("invalid major_version %d, expected %d\n", major_version, MAJOR_VERSION);
            return -1;
        }
        return 0;
    }
};
};

#endif //JAVSVM_JCLASS_HEADER_H
