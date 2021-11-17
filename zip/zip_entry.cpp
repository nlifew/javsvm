

#include "zip_entry.h"
#include "io/input_stream.h"
#include "io/output_stream.h"

using namespace javsvm;


zip_entry& zip_entry::operator=(const zip_entry &o)
{

#define _(x) (x = o.x)
    _(made_ver);
    _(extract_ver);
    _(flag);
    _(method);
    _(m_time);
    _(m_date);
    _(crc32);
    _(comp_size);
    _(uncomp_size);
    _(name_len);
    _(extra_len);
    _(comment_len);
    _(disk_num);
    _(internal_attrs);
    _(external_attrs);
    _(lfh_offset);
    // _(m_deleted);
#undef _
    set_name(o.name);
    set_extra(o.extra, o.extra_len);
    set_comment(o.comment);

    return *this;
}

zip_entry::~zip_entry()
{
    set_name(nullptr);
    set_extra(nullptr, 0);
    set_comment(nullptr);
}

void zip_entry::set_compress_method(u2 m)
{
    switch (m) {
        case zip_utils::COMPRESS_STORED:
        case zip_utils::COMPRESS_DEFLATED:
            method = m;
            break;
    }
}

time_t zip_entry::get_modified_time()
{
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    tm.tm_sec = ((m_time & 0x1f) << 1) - 1;
    tm.tm_min = ((m_time >> 5) & 0x3f) - 1;
    tm.tm_hour = (m_time >> 11) - 1;

    tm.tm_mday = (m_date & 0x1f);
    tm.tm_mon = ((m_date >> 5) & 0x0f) - 1;
    tm.tm_year = ((m_date >> 9) & 0x7f) + 80;

    return mktime(&tm);
}

void zip_entry::set_modified_time(time_t time)
{
    struct tm *tm = gmtime(&time);

    m_time = ((tm->tm_sec + 1) >> 1) & 0x1f;
    m_time |= ((tm->tm_min + 1) & 0x3f) << 5;
    m_time |= ((tm->tm_hour + 1) & 0x1f) << 11;

    m_date = (tm->tm_mday & 0x1f);
    m_date |= ((tm->tm_mon + 1) & 0x0f) << 5;
    m_date |= ((tm->tm_year - 80) & 0x7f) << 9;
}

void zip_entry::set_comment(const char *str)
{
    delete[] comment;
    comment = nullptr;
    comment_len = 0;

    if (str && (comment_len = strlen(str)) > 0) {
        comment = new char[comment_len + 1];
        memcpy(comment, str, comment_len);
        comment[comment_len] = '\0';
    }
}


void zip_entry::set_extra(const void *src, u2 len)
{
    delete[] extra;
    extra = nullptr;
    extra_len = 0;

    if (src && (extra_len = len) > 0) {
        extra = new char[len];
        memcpy(extra, src, len);
    }
}


void zip_entry::set_name(const char *str)
{
    delete[] name;
    name = nullptr;
    name_len = 0;
    if (str && (name_len = strlen(str)) > 0) {
        name = new char[name_len + 1];
        memcpy(name, str, name_len);
        name[name_len] = '\0';
    }
}


static void read_string(input_stream& in, char* &str, u2 len)
{
    delete[] str;
    str = nullptr;
    if (len > 0) {
        str = new char[len + 1];
        in.read_bytes(str, len);
        str[len] = '\0';
    }
}


int zip_entry::read_as_cde(input_stream &in)
{
    in.little_endian();
    u4 magic;
    in >> magic;

    if (magic != CDE_MAGIC) {
        LOGE("invalid cde magic %#x, expected %#x\n", magic, CDE_MAGIC);
        return -1;
    }

    in >> made_ver;
    in >> extract_ver;
    in >> flag;
    in >> method;
    in >> m_time;
    in >> m_date;
    in >> crc32;
    in >> comp_size;
    in >> uncomp_size;
    in >> name_len;
    in >> extra_len;
    in >> comment_len;
    in >> disk_num;
    in >> internal_attrs;
    in >> external_attrs;
    in >> lfh_offset;
    

    if (name_len == 0) {
        LOGE("invalid cde name length 0\n");
        return -1;
    }
    read_string(in, name, name_len);
    read_string(in, extra, extra_len);
    read_string(in, comment, comment_len);
    return 0;
}

int zip_entry::read_as_lfh(input_stream& in)
{
    in.little_endian();
    u4 magic;
    in >> magic;

    if (magic != LFH_MAGIC) {
        LOGE("invalid lfh magic %#x, expected %#x\n", magic, LFH_MAGIC);
        return -1;
    }

    in >> extract_ver;
    in >> flag;
    in >> method;
    in >> m_time;
    in >> m_date;
    in >> crc32;
    in >> comp_size;
    in >> uncomp_size;
    in >> name_len;
    in >> extra_len;
    
    if (name_len == 0) {
        LOGE("invalid lfh name length 0\n");
        return -1;
    }
    read_string(in, name, name_len);
    read_string(in, extra, extra_len);
    return 0;
}

void zip_entry::write_as_lfh(output_stream& out)
{
    out << (u4) LFH_MAGIC;
    out << extract_ver;
    out << flag;
    out << method;
    out << m_time;
    out << m_date;
    out << crc32;
    out << comp_size;
    out << uncomp_size;
    out << name_len;
    out << extra_len;

    out.write_bytes(name, name_len);
    out.write_bytes(extra, extra_len);
}


void zip_entry::write_as_cde(output_stream& out)
{
    out << (u4) CDE_MAGIC;
    out << made_ver;
    out << extract_ver;
    out << flag;
    out << method;
    out << m_time;
    out << m_date;
    out << crc32;
    out << comp_size;
    out << uncomp_size;
    out << name_len;
    out << extra_len;
    out << comment_len;
    out << disk_num;
    out << internal_attrs;
    out << external_attrs;
    out << lfh_offset;

    out.write_bytes(name, name_len);
    out.write_bytes(extra, extra_len);
    out.write_bytes(comment, comment_len);
}
