

#ifndef JAVSVM_JAVAH_H
#define JAVSVM_JAVAH_H

#include <map>
#include <list>
#include <string>
#include <cstring>

#include "object/jmethod.h"
#include "class/jclass_file.h"

struct args_t
{
    const char *file = "";
    const char *output = "";

    enum mode_t {
        none, h, cpp, list,
    };
    mode_t mode = h;

    int return_if_empty = 0;

    static mode_t get_mode(const char *str) noexcept {
        if (strcmp(str, "list") == 0) return list;
        if (strcmp(str, "h") == 0) return h;
        if (strcmp(str, "cpp") == 0) return cpp;
        return none;
    }
};

struct context_t
{
    const char *class_name;
    std::map<std::string, std::list<javsvm::jmethod>> methods;

    javsvm::jclass_file class_file;

    int open(const char *name) noexcept;

    void print_as_list(FILE *fout) const noexcept;

    void print_as_h(FILE *fout) const noexcept;

    void print_as_cpp(FILE *fout) const noexcept;
};

#endif