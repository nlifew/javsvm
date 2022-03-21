//
// Created by edz on 2022/1/31.
//

#ifndef JAVSVM_SAFETY_POINT_H
#define JAVSVM_SAFETY_POINT_H

#include "../utils/global.h"

namespace javsvm
{



/**
 * 检查安全点.
 * 字节码执行引擎应当在必要的地方插入 check_safety_point() 函数，以及时响应 gc 的请求。
 *
 * 众所周知，安全点的选取不能太多，也不能太少: 太多会造成不必要的性能损失，太少则会延长
 * gc 线程等待时间。能插入安全点的地方通常有下面几种:
 *
 * 1. 函数调用开始处;
 * 2. 函数调用结束处;
 * 3. 抛出异常处;
 * 4. 非 count 循环回跳处;
 * 5. 调用 new 指令处.
 *
 * 下面的实现是向 safety_point_trap 指向的内存写入一条数据。在 gc 线程不活跃时，
 * 性能消耗大约是 2-3 条汇编指令; gc 线程活跃时会将 safety_point_trap 指向的内存
 * 设置为不可写，这会让该线程进入我们设定好的异常处理函数，在那里可以将其挂起。
 */
extern volatile char *safety_point_trap;
#define check_safety_point() (safety_point_trap[0] = '\0')


/**
 * 表示当前线程已经进入了安全区，其执行状态(挂起、恢复等) 不再受到 jvm 的管理和控制。
 * 在进入安全区后，gc 线程在 Stop-The-World 时不会再等待这个线程。
 * 事实上，对于虚拟机执行引擎，导致这种 "线程不受 jvm 控制" 的情景只有为数不多几种情况:
 * 1. 执行 native 函数时;
 * 2. 线程附加到 jvm 之后，执行第一个 java 函数之前;
 * 3. 开始获取锁 (monitor) 之后，获取到锁之前(此时这个线程是有可能被挂起的);
 * 4. java 线程执行完最后一个 java 函数之后，从虚拟机分离之前。
 */
void enter_safety_area() noexcept;

/**
 * 表示当前线程即将离开安全区。在离开安全区之前，该线程会去检查安全点，
 * 如果 gc 线程正在活跃，则会挂起线程直到 gc 执行完。
 * 和 enter_safety_area() 相对的，leave_safety_area() 也有几种应用情景:
 * 1. 线程执行完 native 函数，即将返回 java 函数时;
 * 2. 线程开始执行第一个 java 函数时;
 * 3. 线程获取到锁 (monitor) 之后
 */
void leave_safety_area() noexcept;


/**
 * 但所有线程都停下来时的回调
 */


}; // namespace javsvm


#endif //JAVSVM_SAFETY_POINT_H
