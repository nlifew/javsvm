

#include "dll_loader.h"
#include "../utils/log.h"
#include "../vm/jvm.h"
#include "../jni/jni_utils.h"

#include <memory>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <vector>


#if WINDOWS
#include <wtypes.h>
#include <winbase.h>
#else
#include <dlfcn.h>
#endif


using namespace javsvm;

static std::string trim(const char *name)
{
    LOGI("trim: input string is '%s'\n", name);

    size_t name_len;
    if (name == nullptr || (name_len = strlen(name)) == 0) {
        LOGI("trim: input string is null or empty, return\n");
        return "";
    }

    char *buff = new char[name_len + 1];
    std::unique_ptr<char, void(*)(const char*)> buff_guard(
            buff, [](const char *ptr) { delete[] ptr; });

    memcpy(buff, name, name_len + 1);

#if WINDOWS
    // 在 windows 环境下，将所有的斜杠替换为反斜杠
    for (int i = 0; i < name_len; i ++) {
        if (buff[i] == '/') buff[i] = '\\';
    }
#endif

    // 判断是不是根目录
#if WINDOWS
    bool is_absolute_path = false;
    for (int i = 1; i < name_len; i ++) {
        if (buff[i] == '\\') {
            is_absolute_path = buff[i - 1] == ':';
            break;
        }
    }
#else
    bool is_absolute_path = buff[0] == '/';
#endif
    LOGI("trim: is absolute path: %d\n", is_absolute_path);

    std::vector<const char*> stack;

#if WINDOWS
    const char *token = "\\";
#else
    const char *token = "/";
#endif

    std::unique_ptr<char, void(*)(char*)> cwd_guard(
            nullptr, [](char *ptr) { free(ptr); });

    if (! is_absolute_path) {
        // 如果不是绝对路径，使用栈结构拼接得到绝对路径
        char *cwd = getcwd(nullptr, 0);
        cwd_guard.reset(cwd);

        for (char *walk = strtok(cwd, token); walk; walk = strtok(nullptr, token)) {
            stack.push_back(walk);
        }
    }

    for (char *walk = strtok(buff, token); walk; walk = strtok(nullptr, token)) {
        if (strcmp(walk, ".") == 0) {
            // nothing to do.
        }
        else if (strcmp(walk, "..") == 0) {
            if (!stack.empty()) stack.pop_back();
        }
        else {
            stack.push_back(walk);
        }
    }
    std::string path;
#if !WINDOWS
    path.append(token);
#endif
    for (const auto &it : stack) {
        path.append(it).append(token);
    }
    path.erase(path.size() - 1);

    LOGI("trim: '%s' -> '%s'\n", name, path.c_str());
    return path;
}



static void* load_library0(const char *name) noexcept
{
#if WINDOWS
    return LoadLibraryA(name);
#else
    return dlopen(name, RTLD_LAZY | RTLD_GLOBAL);
#endif
}


void* dll_loader::load_library(const char *name)
{
    LOGI("load_library: start with '%s'\n", name);
    std::string key = std::move(trim(name));

    if (key.empty()) {
        LOGI("load_library: empty path, abort\n");
        return nullptr;
    }

    {
        std::shared_lock rd_lock(m_lock);

        const auto &it = m_cache.find(key);
        if (it != m_cache.end()) {
            LOGI("load_library: found in cache, nothing to do\n");
            return it->second;
        }
    }

    std::unique_lock wr_lock(m_lock);
    {
        const auto &it = m_cache.find(key);
        if (it != m_cache.end()) {
            LOGI("load_library: found in cache when double check, nothing to do\n");
            return it->second;
        }
    }

    void *native_ptr = ::load_library0(key.c_str());

    if (native_ptr == nullptr) {
        PLOGE("load_library: failed to load library '%s'\n", key.c_str());
        return nullptr;
    }
    m_cache[key] = native_ptr;

    // 准备调用 JNI_OnLoad()
    if (call_JNI_OnLoad(native_ptr) < 0) {
        PLOGE("load_library: call JNI_OnLoad error\n");
        exit(1);
    }
    return native_ptr;
}

int dll_loader::call_JNI_OnLoad(void *library) noexcept
{
    LOGD("call_JNI_OnLoad: start with library %p\n", library);

    using JNI_OnLoad_t = jint (*) (JavaVM*, void*);
    auto JNI_OnLoad = (JNI_OnLoad_t) find_symbol("JNI_OnLoad", library);

    if (JNI_OnLoad == nullptr) {
        // 如果没有导出 JNI_OnLoad，直接返回即可
        LOGD("call_JNI_OnLoad: JNI_OnLoad is nullptr, ignore\n");
        return 0;
    }

    LOGD("call_JNI_OnLoad: will call\n");

    auto status = JNI_OnLoad(jni::java_vm, nullptr);
    if (status < 0) {
        LOGE("call_JNI_OnLoad: JNI 层返回异常 %d\n", status);
    }
    return status;
}

static inline void free_library0(void *native_ptr)
{
#if WINDOWS
    FreeLibrary((HMODULE) native_ptr);
#else
    dlclose(native_ptr);
#endif
}

dll_loader::~dll_loader()
{
//    std::unique_lock wr_lock(m_lock);
    for (const auto &it : m_cache) {
        free_library0(it.second);
    }
}

void dll_loader::free_library(const char *name)
{
    std::string name_s = std::move(trim(name));
    if (name_s.empty()) {
        return;
    }

    std::unique_lock wr_lock(m_lock);
    auto it = m_cache.find(name_s);

    if (it != m_cache.end()) {
        free_library0(it->second);
        m_cache.erase(it);
    }
}


static inline void* find_symbol0(void *native_ptr, const char *symbol)
{
#if WINDOWS
    return GetProcAddress((HMODULE) native_ptr, symbol);
#else
    return dlsym(native_ptr, symbol);
#endif
}

void *dll_loader::find_symbol(const char *symbol, void *native_ptr)
{
    if (native_ptr != nullptr) {
        return ::find_symbol0(native_ptr, symbol);
    }
    std::shared_lock rd_lock(m_lock);
    for (const auto &it : m_cache) {
        auto sym = ::find_symbol0(it.second, symbol);
        if (sym != nullptr) {
            return sym;
        }
    }
    return nullptr;
}

