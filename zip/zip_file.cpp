

#include "zip_entry.h"
#include "zip_file.h"
#include "zip_utils.h"
#include "io/input_stream.h"
#include "io/output_stream.h"
#include "utils/log.h"
#include <unistd.h>
#include <memory>

using namespace javsvm;



#define HAS_FLAG(f, x) (f & x)


int zip_file::open(const char *name, int flag)
{
    close();

    bool is_new_file = true;

    if (HAS_FLAG(flag, zip_file::Z_RDONLY)) {
        m_fp = fopen(name, "rb");
        is_new_file = false;
    }
    else if (HAS_FLAG(flag, zip_file::Z_RDWR)) {
        if (access(name, F_OK) == 0) {
            is_new_file = HAS_FLAG(flag, zip_file::Z_TRUNC);
            m_fp = fopen(name, is_new_file ? "wb+" : "rb+");
        }
        else if (HAS_FLAG(flag, zip_file::Z_CREAT)) {
            m_fp = fopen(name, "wb+");
            is_new_file = true;
        }
        else {
            LOGE("file %s doesn't exist, use Z_CREAT to create one\n", name);
            return -1;
        }
    }
    else {
        LOGE("use Z_RDONLY or Z_RDWR to open a zip\n");
        return -1;
    }
    
    if (m_fp == nullptr) {
        PLOGE("can't open file %s\n", name);
        return -1;
    }
    m_flag = flag;

    if (! is_new_file && read_entries() != 0) {
        close();
        return -1;
    }
    return 0;
}

int zip_file::read_entries()
{
    fseek(m_fp, 0, SEEK_END);
    long file_len = ftell(m_fp);

    int buff_len = 65535 + zip_eocd::SIZE;
    char *buff = new char[buff_len];
    std::unique_ptr<char, void (*)(char *)> buff_guard(buff,
                        [](char *p) { delete[] p; });

    fseek(m_fp, file_len > buff_len ? -buff_len : -file_len, SEEK_END);
    int bytes = fread(buff, 1, buff_len, m_fp);

    if (m_eocd.find(buff, bytes) != 0) {
        LOGE("can't find zip EndOfCenteralDirectory magic\n");
        return -1;
    }

    fseek(m_fp, m_eocd.offset(), SEEK_SET);
    m_entries.reserve(m_eocd.entry_num());
    file_input in(m_fp);

    for (int i = 0, n = m_eocd.entry_num(); i < n; i++) {
        zip_entry *e = new zip_entry;
        if (e->read_as_cde(in) != 0) {
            delete e;
            return -1;
        }
        m_entries.push_back(e);
        m_index_map[e->get_name()] = i;
    }

    return 0;
}

int zip_file::uncompress(zip_entry *e, FILE *dst)
{
    if (e == nullptr || m_fp == nullptr) {
        return -1;
    }

    fseek(m_fp, e->lfh_offset, SEEK_SET);

    zip_entry tmp;
    file_input in(m_fp);

    if (tmp.read_as_lfh(in) != 0) {
        PLOGE("failed to read lfh from zip_entry\n");
        return -1;
    }

    int size = 0, crc32 = 0;
    zip_utils::unCompress(e->method, dst, &size, m_fp, e->comp_size, &crc32);
    
    if (size != (int) e->uncomp_size || crc32 != (int) e->crc32) {
        PLOGE("uncompress failed, expected uncompressed_size [%d], actually [%d]"
                "expected crc32 [%#08x], actually [%#08x]\n", 
                e->uncomp_size, size, e->crc32, crc32);
        return -1;
    }
    return 0;
}

int zip_file::uncompress(zip_entry *e, void *dst)
{
    if (e == nullptr || m_fp == nullptr) {
        return -1;
    }

    fseek(m_fp, e->lfh_offset, SEEK_SET);

    zip_entry tmp;
    file_input in(m_fp);

    if (tmp.read_as_lfh(in) != 0) {
        PLOGE("failed to read lfh from zip_entry\n");
        return -1;
    }

    int size = 0, crc32 = 0;
    zip_utils::unCompress(e->method, dst, &size, m_fp, e->comp_size, &crc32);
    
    if (size != (int) e->uncomp_size || crc32 != (int) e->crc32) {
        PLOGE("uncompress failed, expected uncompressed_size [%d], actually [%d]"
                "expected crc32 [%#08x], actually [%#08x]\n", 
                e->uncomp_size, size, e->crc32, crc32);
        return -1;
    }
    return 0;
}




#if 0
static int fmove(FILE *fp, int src, int len, char *buff, int buff_len)
{
    int pos = ftell(fp);
    if (pos == src || len == 0) {
        fseek(fp, len, SEEK_CUR);
        return len;
    }
    if (pos > src) {
        return -1;
    }

    int consumed = 0;
    while (consumed < len) {
        fseek(fp, src + consumed, SEEK_SET);

        int expected = std::min(len - consumed, buff_len); // (len - consumed > buff_len) ? buff_len : (len - consumed);
        int actually = fread(buff, 1, expected, fp);

        if (expected != actually) {
            LOGE("failed to read %d bytes, actually %d bytes\n", expected, actually);
            break;
        }
        fseek(fp, consumed + pos, SEEK_SET);
        // fwrite(buff, 1, expected, tmp);
        actually = fwrite(buff, 1, expected, fp);

        if (actually != expected) {
            LOGE("failed to write %d bytes, actually %d bytes\n", expected, actually);
            break;
        }
        consumed += actually;
    }
    
    return consumed;
}


int zip_file::flush(void)
{
    if (m_fp == nullptr || ! HAS_FLAG(m_flag, zip_file::Z_RDWR)) {
        return -1;
    }

    // 1. 调整 vector 和 map，去除已移除的 entry
    m_index_map.clear();
    int N = 0;
    for (int i = 0, n = m_entries.size(); i < n; i++) {
        auto e = m_entries[i];
        if (e->is_deleted()) {
            delete e;
            continue;
        }
        m_entries[N] = e;
        m_index_map[e->get_name()] = N ++;
    }
    m_entries.resize(N);

    // 2. 向文件中写入 LFH 和 文件内容
    fseek(m_fp, 0, SEEK_SET);
    file_output fout(m_fp);
    char *buff = new char[32 * 1024];
    std::unique_ptr<char, void (*)(char *)> buff_guard(buff,
                        [](char *p) { delete[] p; });

    for (auto it : m_entries) {
        long pos = ftell(m_fp);
        it->write_as_lfh(fout);
        if (fmove(m_fp, it->lfh_offset, it->uncomp_size, buff, 32*1024) != 0) {
            return -1;
        }
        it->lfh_offset = pos;
    }

    // 3. 向文件中写入 CDE
    long pos = ftell(m_fp);
    for (auto it : m_entries) {
        it->write_as_cde(fout);
    }

    // 4. 重建 eocd
    m_eocd.set_entry_num(N);
    m_eocd.set_cde_info(pos, ftell(m_fp) - pos);
    m_eocd.write_to(fout);

    return 0;
}
#endif 


void zip_file::close(void)
{
    for (auto it : m_entries) {
        delete it;
    }
    if (m_fp) {
        fclose(m_fp);
        m_fp = nullptr;
    }
    m_entries.clear();
    m_index_map.clear();
    m_flag = 0;
}

zip_entry** zip_file::list(int *p_size)
{
    int N = size();
    zip_entry **array = new zip_entry *[N];
    int idx = 0;
    for (auto it : m_entries) {
        if (! it->is_deleted()) {
            array[idx ++] = it;
        }
    }
    if (p_size) *p_size = N;
    return array;
}

#if 0
int zip_file::remove_entry(zip_entry *e)
{
    if (e == nullptr || ! HAS_FLAG(m_flag, zip_file::Z_RDWR)) {
        return -1;
    }

    auto it = m_index_map.find(e->get_name());
    if (it == m_index_map.end()) {
        return -1;
    }

    // 我们并不需要从 vector 中把这个 entry 移除，
    // 这样可以避免 vector 中内存的移动，
    // 以及 map 中 index 的更新。
    // 至于这个 entry，简单将其标记为 '已移除' 就可以
    // 在下次 flush() 时会自动释放掉

    e = m_entries.at(it->second);
    e->mark_deleted();
    m_index_map.erase(it);

    return 0;
}



int zip_file::add_entry(zip_entry *e, const void *src, int src_len)
{
    if (e == nullptr || ! HAS_FLAG(m_flag, zip_file::Z_RDWR)) {
        return -1;
    }

    remove_entry(e);

    
}

int add_entry(zip_entry *e, FILE *src, int src_len)
{

}


#endif 