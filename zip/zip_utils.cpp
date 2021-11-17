
#include "zip_utils.h"
#include "zip_entry.h"
#include "utils/log.h"

#include <zlib.h>

using namespace javsvm;



#ifndef DEF_MEM_LEVEL
#define DEF_MEM_LEVEL 8
#endif

#define BUFF_SIZE (32 * 1024)


template <typename Reader, typename Writer>
static int compressT(int method, Writer& writer, Reader& reader, int *pCrc32)
{
    int status = -1;
    unsigned char *buff = new unsigned char[BUFF_SIZE * 2];
    unsigned long crc = crc32(0L, Z_NULL, 0);

    if (method == zip_utils::COMPRESS_STORED) {
        int count;
        while ((count = reader.read(buff, sizeof(buff))) != -1) {
            crc = crc32(crc, buff, count);
            int bytes = writer.write(buff, count);
            if (bytes != count) {
                LOGE("failed to write %d bytes, actually %d bytes\n", count, bytes);
                goto bail;
            }
        }
        status = 0;
        goto bail;
    }

    if (method == zip_utils::COMPRESS_DEFLATED) {
        unsigned char *inBuff = buff;
        unsigned char *outBuff = (buff + BUFF_SIZE);

        z_stream zstream;
        memset(&zstream, 0, sizeof(z_stream));

        zstream.next_out = outBuff;
        zstream.avail_out = BUFF_SIZE;
        zstream.data_type = Z_UNKNOWN;

        int zerr = deflateInit2(&zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 
                    -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
        if (zerr == Z_VERSION_ERROR) {
            LOGE("invlid zlib version: %s\n", ZLIB_VERSION);
            goto bail;
        }
        else if (zerr != Z_OK) {
            goto bail;
        }


        bool eof = false;

        while (zerr == Z_OK) {
            if (zstream.avail_in == 0 && ! eof) {
                int count = reader.read(inBuff, BUFF_SIZE);
                if (count == -1) {      // 到达流的末尾，退出
                    status = 0;
                    break;
                }
                eof = count < BUFF_SIZE;
                crc = crc32(crc, inBuff, count);
                zstream.next_in = inBuff;
                zstream.avail_in = count;
            }

            zerr = deflate(&zstream, eof ? Z_FINISH : Z_NO_FLUSH);
            if (zerr != Z_OK && zerr != Z_STREAM_END) {
                break;
            }

            if (zstream.avail_out == 0 || (zerr == Z_STREAM_END && zstream.avail_out != BUFF_SIZE)) {
                int expected = zstream.next_out - outBuff;
                int actually = writer.write(outBuff, expected);
                if (expected != actually) {
                    LOGE("failed to write %d bytes, actually %d bytes\n", expected, actually);
                    break;
                }
                zstream.next_out = outBuff;
                zstream.avail_out = BUFF_SIZE;
            }
        }
        if (zerr == Z_STREAM_END) {
            status = 0;
        }
        deflateEnd(&zstream);
        goto bail;
    }

    LOGE("invalid compress method: %d\n", method);

bail:
    delete[] buff;
    if (pCrc32 != nullptr) {
        *pCrc32 = crc;
    }
    return status;
}


template <typename Reader, typename Writer>
static int unCompressT(int method, Writer& writer, Reader& reader, int *pCrc32)
{
    int status = -1;
    unsigned char *buff = new unsigned char[BUFF_SIZE * 2];
    unsigned long crc = crc32(0L, Z_NULL, 0);

    if (method == zip_utils::COMPRESS_STORED) {
        int count;
        while ((count = reader.read(buff, sizeof(buff))) != -1) {
            crc = crc32(crc, buff, count);
            int bytes = writer.write(buff, count);
            if (bytes != count) {
                LOGE("failed to write %d bytes, actually %d bytes\n", count, bytes);
                goto bail;
            }
        }
        status = 0;
        goto bail;
    }

    if (method == zip_utils::COMPRESS_DEFLATED) {
        unsigned char *inBuff =  buff;
        unsigned char *outBuff = (buff + BUFF_SIZE);

        z_stream zstream;
        memset(&zstream, 0, sizeof(z_stream));

        zstream.next_out = outBuff;
        zstream.avail_out = BUFF_SIZE;
        zstream.data_type = Z_UNKNOWN;

        int zerr = inflateInit2(&zstream, -MAX_WBITS);
        if (zerr == Z_VERSION_ERROR) {
            LOGE("invlid zlib version: %s\n", ZLIB_VERSION);
            goto bail;
        }
        else if (zerr != Z_OK) {
            goto bail;
        }

        while (zerr == Z_OK) {
            if (zstream.avail_in == 0) {
                int count = reader.read(inBuff, BUFF_SIZE);
                if (count == -1) {      // 到达流的末尾
                    status = 0;
                    break;
                }
                zstream.next_in = inBuff;
                zstream.avail_in = count;
            }

            zerr = inflate(&zstream, Z_NO_FLUSH);
            if (zerr != Z_OK && zerr != Z_STREAM_END) {
                break;
            }

            if (zstream.avail_out == 0 || (zerr == Z_STREAM_END && zstream.avail_out != BUFF_SIZE)) {
                int expected = zstream.next_out - outBuff;
                int actually = writer.write(outBuff, expected);
                if (expected != actually) {
                    LOGE("failed to write %d bytes, actually %d bytes\n", expected, actually);
                    break;
                }
                crc = crc32(crc, outBuff, expected);
                zstream.avail_out = BUFF_SIZE;
                zstream.next_out = outBuff;
            }
        }
        if (zerr == Z_STREAM_END) {
            status = 0;
        }
        inflateEnd(&zstream);
        goto bail;
    }

    LOGE("invalid unCompress method: %d\n", method);

bail:
    delete[] buff;
    if (pCrc32 != nullptr) {
        *pCrc32 = crc;
    }
    return status;
}





class zip_utils::DataReader
{
private:
    const char *mData;
    int mLimit;
    int mOffset;

public:
    explicit DataReader(const void *src, int length) {
        mData = (const char*) src;
        mLimit = length;
        mOffset = 0;
    }

    int read(void *dst, int length) {
        int count = mLimit - mOffset;
        if (count <= 0) {    // 已经到达末尾
            return -1;
        }
        count = (count < length) ? count : length;
        memcpy(dst, mData + mOffset, count);
        mOffset += count;
        return count;
    }
};

class zip_utils::DataWriter
{
private:
    char *mData;
    int mOffset;
    int *mLength;

public:
    explicit DataWriter(void *dst, int *length) {
        mData = (char*) dst;
        mOffset = 0;
        mLength = length;
        if (mLength == nullptr) {
            mLength = &mOffset;
        }
    }

    int write(const void *src, int length) {
        memcpy(mData + mOffset, src, length);
        mOffset += length;
        *mLength = mOffset;
        return length;
    }
};

class zip_utils::FileReader
{
private:
    FILE *mFile;
    int mOffset;
    int mLimit;

public:
    explicit FileReader(FILE *src, int length) {
        mFile = src;
        mLimit = length;
        mOffset = 0;
    }

    int read(void *dst, int length) {

        int count = mLimit - mOffset;

        if (feof(mFile) || ferror(mFile) || count <= 0) {
            return -1;
        }
        count = count < length ? count : length;
        count = fread(dst, 1, count, mFile);
        mOffset += count;
        return count;
    }
};

class zip_utils::FileWriter
{
private:
    FILE *mFile;
    int mOffset;
    int *mLength;

public:
    explicit FileWriter(FILE *dst, int *length) {
        mFile = dst;
        mOffset = 0;
        mLength = length;
        if (mLength == nullptr) {
            mLength = &mOffset;
        }
    }

    int write(const void *src, int length) {
        int count = fwrite(src, 1, length, mFile);
        mOffset += count;
        *mLength = mOffset;
        return count;
    }
};



int zip_utils::compress(int method, void *dst, int *dst_len, 
                            const void *src, int src_len, int *crc32)
{
    DataReader reader(src, src_len);
    DataWriter writer(dst, dst_len);
    return compressT(method, writer, reader, crc32);
}


int zip_utils::compress(int method, FILE *dst, int *dst_len, 
                            const void *src, int src_len, int *crc32)
{
    DataReader reader(src, src_len);
    FileWriter writer(dst, dst_len);
    return compressT(method, writer, reader, crc32);
}


int zip_utils::compress(int method, void *dst, int *dst_len, 
                            FILE *src, int src_len, int *crc32)
{
    FileReader reader(src, src_len);
    DataWriter writer(dst, dst_len);
    return compressT(method, writer, reader, crc32);
}


int zip_utils::compress(int method, FILE *dst, int *dst_len, 
                            FILE *src, int src_len, int *crc32)
{
    FileReader reader(src, src_len);
    FileWriter writer(dst, dst_len);
    return compressT(method, writer, reader, crc32);
}


int zip_utils::unCompress(int method, void *dst, int *dst_len, 
                            const void *src, int src_len, int *crc32)
{
    DataReader reader(src, src_len);
    DataWriter writer(dst, dst_len);
    return unCompressT(method, writer, reader, crc32);
}


int zip_utils::unCompress(int method, FILE *dst, int *dst_len, 
                            const void *src, int src_len, int *crc32)
{
    DataReader reader(src, src_len);
    FileWriter writer(dst, dst_len);
    return unCompressT(method, writer, reader, crc32);
}


int zip_utils::unCompress(int method, void *dst, int *dst_len, 
                            FILE *src, int src_len, int *crc32)
{
    FileReader reader(src, src_len);
    DataWriter writer(dst, dst_len);
    return unCompressT(method, writer, reader, crc32);
}


int zip_utils::unCompress(int method, FILE *dst, int *dst_len, 
                            FILE *src, int src_len, int *crc32)
{
    FileReader reader(src, src_len);
    FileWriter writer(dst, dst_len);
    return unCompressT(method, writer, reader, crc32);
}
