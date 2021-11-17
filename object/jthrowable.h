
#ifndef JAVSVM_JTHROWABLE_H
#define JAVSVM_JTHROWABLE_H


#include <exception>
#include <string>
#include <cstdio>

namespace javsvm
{

class jthrowable : public std::exception
{
private:
    std::string m_msg;
    const char *m_class;

public:
    static const int MAX_BUFF_SIZE = 512;

    template <typename ...Args>
    jthrowable(const char *cls, Args&&... args)
    {
        m_class = cls;

        m_msg.reserve(MAX_BUFF_SIZE);
        snprintf(m_msg.c_str(), MAX_BUFF_SIZE, std::forward<Args>(args)...);
    }

    const char *what() const noexcept override
    {
        return m_msg.c_str();
    }
};

};

#endif 
