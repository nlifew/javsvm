

#ifndef JAVSVM_ZIP_EOCD_H
#define JAVSVM_ZIP_EOCD_H

#include "utils/log.h"
#include "utils/global.h"
#include "io/input_stream.h"
#include "io/output_stream.h"

namespace javsvm
{

class zip_eocd
{
private:
    static const int MAGIC = 0x06054B50;

    char *m_comment = nullptr;
    u2 m_comment_length = 0;
    u2 m_entry_num = 0;
    u4 m_cde_size = 0;
    u4 m_cde_offset = 0;

public:
    static const int SIZE = 22;

    zip_eocd() = default;

    zip_eocd(const zip_eocd &o) = delete;

    zip_eocd &operator=(const zip_eocd &o) = delete;

    ~zip_eocd()
    {
        delete[] m_comment;
        m_comment = nullptr;
        m_comment_length = 0;
    }

    int find(const char *buff, int len)
    {
        int i = len - SIZE;
        while (i >= 0 && *(u4*) (buff + i) != MAGIC) {
            i--;
        }
        if (i < 0) {
            LOGE("failed to find EOCD magic %#x\n", MAGIC);
            return -1;
        }

        bytes_input in(buff + i + 4, len - i);
        u2 disk_number = 0;         in >> disk_number;
        u2 center_disk_num = 0;     in >> center_disk_num;
        u2 center_entry_num = 0;    in >> center_entry_num;
        u2 entry_number = 0;        in >> entry_number;
        // u2 disk_number = in.read_short();
        // u2 center_disk_num = in.read_short();
        // u2 center_entry_num = in.read_short();
        // u2 entry_number = in.read_short();

        if (disk_number != 0 || center_disk_num != 0 || 
                center_entry_num != entry_number) {
            LOGE("invalid diskNumber: %d, centerDiskNumber: %d, "
                    "centerEntryNumber: %d, entryNumber: %d\n", 
                    disk_number, center_disk_num, center_entry_num, entry_number);
            return -1;
        }

        m_entry_num = entry_number;

        in >> m_cde_size;
        in >> m_cde_offset;
        in >> m_comment_length;

        // m_cde_size = in.read_int();
        // m_cde_offset = in.read_int();
        // m_comment_length = in.read_short();

        delete[] m_comment;
        m_comment = nullptr;
        if (m_comment_length > 0) {
            m_comment = new char[m_comment_length + 1];
            m_comment[m_comment_length] = '\0';
            in.read_bytes(m_comment, m_comment_length);
        }
        return 0;
    }

    void write_to(output_stream& out)
    {
    #define _(x) out.write(x)
        _((u4) MAGIC);
        _((u4) 0);
        _(m_entry_num);
        _(m_entry_num);
        _(m_cde_size);
        _(m_cde_offset);
        _(m_comment_length);
    #undef _
        out.write_bytes(m_comment, m_comment_length);
    }

    const char *get_comment(int *p_len) 
    {
        if (p_len) *p_len = m_comment_length;
        return m_comment;
    }

    void set_comment(const char *str)
    {
        delete[] m_comment;
        m_comment = nullptr;
        m_comment_length = 0;

        if (str && (m_comment_length = strlen(str)) > 0) {
            m_comment = new char[m_comment_length + 1];
            m_comment[m_comment_length] = '\0';
            memcpy(m_comment, str, m_comment_length);
        }
    }

    void set_entry_num(u2 len) { m_entry_num = len; }

    u4 entry_num() { return m_entry_num; }

    void set_cde_info(u4 offset, u4 len) 
    {
        m_cde_offset = offset;
        m_cde_size = len;
    }

    u4 offset() { return m_cde_offset; }
};

} // namespace javsvm

#endif
