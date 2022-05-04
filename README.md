
## Just A Very Simple Virtual Machine

个人出于兴趣写的玩具级 jvm，目前还在业余开发中～

### 环境

macos arm64
目前打算支持 macos_arm64, macos_x64, linux_x64, windows_x64，
但由于依赖汇编，目前只支持了 macos_arm64。

### 编译环境 

IDE 开发环境是 CLion，导入项目后执行任务 main 即可编译。如果没有 CLion，则需要安装 CMake，
然后按照下面的命令行编译。
```shell
git clone https://github.com/nlifew/javsvm.git
cd javsvm
mkdir cmake-build-release && cd cmake-build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
NOTE: 运行时需要显式指定环境变量 CLASSPATH。这个 CLASSPATH 和传统 jre 的 CLASSPATH 不一样。
javsvm 相比 openjdk 的 jre 做了相当多的处理，因此要将 CLASSPATH
指定为 `项目目录/jre/out`

```shell
cd cmake-build-release
CLASSPATH=../jre/out ./main Main
```

### 项目结构

* class. 即 .class 文件的解析器，负责解析 .class 文件格式，
并转为对应的内存形式;

* classloader. 类加载器，负责根据类名寻找 .class 文件，
并解析成类，函数和字段的类型表示。也支持数组类和基本数据类型的加载;

* concurrent. 工具类，高性能的多线程工具，包括自旋锁(atomic_lock)，
线程安全的 map (concurrent_map)，线程安全的集合 (concurrent_set),
无锁复用池(pool)，递归读写锁 (recursive_lock) 等;

* dll. 动态链接库加载器，封装了 windows 和 posix 两种动态库加载实现;

* engine. 引擎，包括字节码解释执行引擎 (run_java)，异常抛出机制(throw) 和 jni 执行引擎;

* gc. 垃圾收集器。包含根节点枚举类 (gc_root) 和 "标记-整理" 算法实现 (gc_thread);

* io. 工具类，负责基础的 io 操作;

* jni. 为 <jni.h> 中的函数提供实现和包装;

* jre. java 核心 API (java.lang.*, java.util.*, java.io.*) 的实现类。
相比 openjdk 做了诸多调整;

* object. java 中类 (class)、方法 (method) 和 字段 (field) 的表示。
包括 instance_of, hashcode, invoke_method, put/get_field，以及字符串池
和数组访问的实现；

* test. 测试使用的;

* utils. 工具类集合，包括字符串处理，数字格式转换，log 等；

* vm. 虚拟机层面的实现，包括虚拟机实例 (jvm)，线程私有环境
(jenv)，堆 (jheap)，方法区 (method_area)，栈 (stack) 等;

* zip. 访问 jar 文件使用的工具类;

### 尚未实现的部分（留下的坑）

* gc: finalize 函数的执行，弱引用/虚引用/软引用和引用队列的交互;

* 解释引擎: invokedynamic, multiarray, 除 0 异常等;

* jre: openjdk 源码的适配（目前只提供了支持 hello world 的最小环境，
如 io/System 等，反射/classloader/class 相关 API 虽然
在 native 层已实现，但没有暴露到 java 层）;

* zip/jar: 还没加

* 类加载器: 小字节和占位内存的优化还没加;


天坑路漫漫，唉，溜了溜了