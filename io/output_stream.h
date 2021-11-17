

#ifndef JAVSVM_OUTPUT_STREAM_H
#define JAVSVM_OUTPUT_STREAM_H

#include "utils/log.h"
#include "utils/number_utils.h"

namespace javsvm
{

class output_stream
{
private:
    bool m_reverse = false;

public:
    virtual ~output_stream() = default;
    virtual int write_bytes(const void *src, int len) = 0;
    virtual int where() = 0;

    void little_endian() { m_reverse = false; }

    void big_endian() { m_reverse = true; }


    template <typename T>
    void write(T t)
    {
        if (m_reverse) t = number_utils::reverse_endian(t);
        write_bytes(&t, sizeof(T));
    }

    template <typename T>
    output_stream& operator<<(T t)
    {
        write(t);
        return *this;
    }
};


class bytes_output : public output_stream
{
private:
    char *m_buff;
    int m_offset;
    int m_limit;
public:
    bytes_output(void *dst, size_t len) : 
        m_buff((char*) dst), m_offset(0), m_limit(len)
    {
    }

    int where() override { return m_offset; }

    int write_bytes(const void *src, int len) override
    {
        if (m_offset >= m_limit) {
            return -1;
        }
        int n = m_limit - m_offset;
        if (n > len) n = len;

        memcpy(m_buff + m_offset, src, n);
        m_offset += n;
        return n;
    }

};

class file_output : public output_stream
{
private:
    FILE *m_fp;
    bool m_owner;

public:
    explicit file_output(const char *name) : m_owner(true)
    {
        m_fp = fopen(name, "wb");
    }
    
    explicit file_output(FILE *fp) : m_fp(fp), m_owner(false)
    {
    }

    file_output(const file_output& o) : m_fp(o.m_fp), m_owner(false)
    {
    }

    file_output& operator=(const file_output& o)
    {
        if (m_owner) fclose(m_fp);
        m_fp = o.m_fp;
        m_owner = false;
        return *this;
    }
    
    ~file_output() override
    {
        if (m_owner) fclose(m_fp);
        m_fp = nullptr;
        m_owner = false;
    }

    int where() override { return ftell(m_fp); }


    int write_bytes(const void *src, int len) override
    {
        if (ferror(m_fp)) {
            return -1;
        }
        return fwrite(src, 1, len, m_fp);
    }
};

};

#endif