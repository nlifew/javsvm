

#ifndef JAVSVM_ZIPUTILS_H
#define JAVSVM_ZIPUTILS_H

#include <stdio.h>

namespace javsvm
{

struct zip_utils
{

private:
    class DataReader;
    class DataWriter;

    class FileReader;
    class FileWriter;    

public:
    static const int COMPRESS_STORED        = 0;
    // static const int COMPRESS_SHRUNK        = 1;
    // static const int COMPRESS_REDUCED_1     = 2;
    // static const int COMPRESS_REDUCED_2     = 3;
    // static const int COMPRESS_REDUCED_3     = 4;
    // static const int COMPRESS_REDUCED_4     = 5;
    // static const int COMPRESS_IMPLODED      = 6;
    // static const int COMPRESS_TOKENIZED     = 7;
    static const int COMPRESS_DEFLATED      = 8;
    // static const int COMPRESS_DEFLATED_64   = 9;
    // static const int COMPRESS_LIB_IMPLODED  = 10;
    // static const int COMPRESS_RESERVED      = 11;
    // static const int COMPRESS_BZIP2         = 12;


    static int compress(int method, void *dst, int *dst_len, 
                            const void *src, int src_len, int *crc32);

    static int compress(int method, FILE *dst, int *dst_len, 
                            const void *src, int src_len, int *crc32);

    static int compress(int method, void *dst, int *dst_len, 
                            FILE *src, int src_len, int *crc32);

    static int compress(int method, FILE *dst, int *dst_len, 
                            FILE *src, int src_len, int *crc32);

    static int unCompress(int method, void *dst, int *dst_len, 
                            const void *src, int src_len, int *crc32);

    static int unCompress(int method, FILE *dst, int *dst_len, 
                            const void *src, int src_len, int *crc32);

    static int unCompress(int method, void *dst, int *dst_len, 
                            FILE *src, int src_len, int *crc32);

    static int unCompress(int method, FILE *dst, int *dst_len, 
                            FILE *src, int src_len, int *crc32);
};


} // namespace javsvm
#endif

