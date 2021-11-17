


#include "utils/global.h"
#include "utils/log.h"
#include "zip_utils.h"
#include <time.h>

#ifndef JAVSVM_ZIP_ENTRY_H
#define JAVSVM_ZIP_ENTRY_H

namespace javsvm
{

class input_stream;
class output_stream;

class zip_entry
{
private:
    u2 made_ver = EXTRACT_VERSION;
    u2 extract_ver = EXTRACT_VERSION;
    u2 flag = 0;
    u2 method = zip_utils::COMPRESS_STORED;
    u2 m_time = 0;
    u2 m_date = 0;
    u4 crc32 = 0;
    u4 comp_size = 0;
    u4 uncomp_size = 0;
    u2 name_len = 0;
    u2 extra_len = 0;
    u2 comment_len = 0;
    u2 disk_num = 0;
    u2 internal_attrs = 0;
    u4 external_attrs = 0;
    u4 lfh_offset = 0;

    char* comment = nullptr;
    char *name = nullptr;
    char *extra = nullptr;


public:
    explicit zip_entry(const char *str) { set_name(str); }

    zip_entry(const zip_entry &o) { operator=(o); }

    zip_entry &operator=(const zip_entry &);

    ~zip_entry();

    const char *get_name(int *p_len = nullptr) { if (p_len) *p_len = name_len; return name; }

    const char *get_comment(int *p_len = nullptr) { if (p_len) *p_len = comment_len; return comment; }

    const void *get_extra(int *p_len) { if (p_len) *p_len = extra_len; return extra; }

    int get_compress_method() { return method; }

    int get_compressed_size() { return comp_size; }

    int get_uncompressed_size() { return uncomp_size; }

    int get_flag() { return flag; }

    int get_crc32() { return crc32; }

    int get_extract_version() { return extract_ver; }

    time_t get_modified_time();

private: /* don't support */
    void set_modified_time(time_t time);

    void set_comment(const char *str);

    void set_extra(const void *src, u2 len);

    void set_compress_method(u2 method);

    /* end */

private:    /* for zip_file access */
    friend class zip_file;

    bool m_deleted = false;

    bool is_deleted() { return m_deleted; }

    void mark_deleted() { m_deleted = true; }

    explicit zip_entry() = default;

    void set_name(const char *name);

    static const int EXTRACT_VERSION = 0x14;
    static const int CDE_MAGIC = 0x02014b50;
    static const int LFH_MAGIC = 0x04034b50;

    int read_as_cde(input_stream &in);

    int read_as_lfh(input_stream &in);

    void write_as_lfh(output_stream &out);

    void write_as_cde(output_stream &out);

    /* end */
};

} // namespace javsvm


#endif
