//
// Created by edz on 2022/1/31.
//

#include "safety_point.h"
#include "../utils/log.h"

#include <cstdlib>
#include <csignal>
#include <unistd.h>

using namespace javsvm;

struct safety_struct
{
    struct sigaction m_sigaction_backup{};

    safety_struct() noexcept
    {
        // 注册信号处理函数
        register_sigsegv_handler();
    }

    ~safety_struct() noexcept
    {
        unregister_sigsegv_handler();
    }

    static int s_safety_point_trap;

    static void handle_sigsegv(int) noexcept
    {
         // todo:
    }

private:
    int register_sigsegv_handler() noexcept
    {
        struct sigaction sig {};
        sig.sa_handler = handle_sigsegv;
        sigemptyset(&sig.sa_mask);
        sig.sa_flags = 0;

        if (sigaction(SIGSEGV, &sig, &m_sigaction_backup) < 0) {
            PLOGE("register_sigsegv_handler: fail and abort\n");
            exit(1);
        }
        javsvm::safety_point_trap = (char*) &s_safety_point_trap;
        return 0;
    }

    void unregister_sigsegv_handler() noexcept
    {
        sigaction(SIGSEGV, &m_sigaction_backup, nullptr);
        javsvm::safety_point_trap = nullptr;
    }
};

[[maybe_unused]]
static safety_struct s_safety_struct;

int safety_struct::s_safety_point_trap = 0;