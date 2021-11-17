

#ifndef JAVSVM_ZIPFILE_H
#define JAVSVM_ZIPFILE_H

#include "zip_eocd.h"

#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <string>

namespace javsvm
{

class zip_entry;

class zip_file
{
private:
    FILE *m_fp = nullptr;
    int m_flag = 0;

    zip_eocd m_eocd;
    std::vector<zip_entry *> m_entries;
    std::unordered_map<std::string, int> m_index_map;

    int read_entries();

    int index_of(const char* name)
    {
        if (name == nullptr) return -1;
        auto it = m_index_map.find(name);
        return it == m_index_map.end() ? -1 : it->second;
    }

public:
    static const int Z_RDONLY   =   1;
    static const int Z_RDWR     =   1 << 1;
    static const int Z_TRUNC    =   1 << 2;
    static const int Z_CREAT    =   1 << 3;

    zip_file() = default;

    ~zip_file() { close(); }

    zip_file(const zip_file &) = delete;
    zip_file &operator=(const zip_file &) = delete;

    int open(const char *name, int flag);

    int size() { return m_index_map.size(); }

    zip_entry** list(int *p_size);

    zip_entry* get_entry(int index) { return m_entries.at(index); }

    zip_entry* find_entry(const char *name)
    {
        int idx = index_of(name);
        return idx == -1 ? nullptr : get_entry(idx);
    }

    // int add_entry(zip_entry *e, const void *src, int src_len);

    // int add_entry(zip_entry *e, FILE *src, int src_len);

    // int remove_entry(zip_entry *e);

    // int flush(void);

    int uncompress(zip_entry *e, void *dst);

    int uncompress(zip_entry *e, FILE *dst);

    void close(void);

    const char *get_comment(int *p_len = nullptr) { return m_eocd.get_comment(p_len); }

    // void set_comment(const char *str) { m_eocd.set_comment(str); }
};

} // namespace javsvm

#endif

