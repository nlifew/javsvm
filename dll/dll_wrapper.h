

#ifndef JAVSVM_dll_wrapper_H
#define JAVSVM_dll_wrapper_H


namespace javsvm
{

class dll_wrapper
{
private:
    void *m_native_ptr;
    void *find(const char *func, int /* unused */);

public:
    explicit dll_wrapper(const char *dll);
    ~dll_wrapper();

    dll_wrapper(const dll_wrapper &o);
    dll_wrapper &operator=(const dll_wrapper &);

    template <typename T>
    T find(const char *func) { return (T)find(func, 0); }

    bool ok() { return m_native_ptr; }
};

};

#endif
