


#ifndef _UTILS_FILE_UTILS_H
#define _UTILS_FILE_UTILS_H


#include <stdio.h>
#include <sys/stat.h>

namespace javsvm
{


struct file_utils
{


    static char* open(const char *name, long *p_length)
    {
        if (p_length) *p_length = 0;

        FILE *fp = nullptr;
        struct stat st;

        if (stat(name, &st) || (fp = fopen(name, "rb")) == nullptr) {
            return nullptr;
        }

        char *buff = new char[st.st_size];
        long len = fread(buff, 1, st.st_size, fp);
        fclose(fp);

        if (len != st.st_size) {
            delete[] buff;
            return nullptr;
        }

        if (p_length) *p_length = len;
        return buff;
    }


};

};

#endif
