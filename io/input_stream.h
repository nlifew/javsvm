



#ifndef _UTILS_INPUT_STREAM_H
#define _UTILS_INPUT_STREAM_H


#include "../utils/number_utils.h"
#include "../utils/log.h"

namespace javsvm
{

class input_stream
{
protected:
    bool m_reverse = false;
public:

    virtual ~input_stream() = default;

    virtual long read_bytes(void *dst, int len) = 0;

    virtual long where() const = 0;

    void little_endian() { m_reverse = false; }

    void big_endian() { m_reverse = true; }

    template<typename T>
    T read()
    {
        T t;
        (*this) >> t;
        return t;
    }

    template <typename T>
    input_stream& operator>>(T& t)
    {
        read_bytes(&t, sizeof(T));
        if (m_reverse) t = number_utils::reverse_endian(t);
        return *this;
    }
};



class bytes_input : public input_stream
{
private:
    const char *m_buff;
    int m_offset;
    int m_limit;

public:
    bytes_input(const void *buff, int len) :
            m_buff(static_cast<const char *>(buff)),
            m_offset(0),
            m_limit(len)
    {
    }

    long where() const override { return m_offset; }

    long read_bytes(void *dst, int len) override
    {
        if (m_offset >= m_limit) return -1;

        int n = m_limit - m_offset;
        if (n > len) n = len;

        memcpy(dst, m_buff + m_offset, n);
        m_offset += n;
        return n;
    }
};

class file_input : public input_stream
{
private:
    FILE *m_fp;
    bool m_owner;

public:

    explicit file_input(const char *name): m_owner(true)
    {
        m_fp = fopen(name, "rb");
    }

    ~file_input() override
    {
        if (m_owner) fclose(m_fp);
        m_fp = nullptr;
        m_owner = false;
    }

    long where() const override { return (int) ftell(m_fp); }

    long read_bytes(void *dst, int len) override
    {
        if (m_fp == nullptr /* || feof(m_fp) || ferror(m_fp) */) {
            return -1;
        }
        return (int) fread(dst, 1, len, m_fp);
    }
};

}
#endif



