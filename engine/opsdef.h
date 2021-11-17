

enum jops
{
    nop = 0,        /* 空指令 */
    aconst_null,    /* 将 null 压栈 */
    iconst_m1,      /* 将 int (-1) 压栈 */
    iconst_0,       /* 将 int (0) 压栈 */
    iconst_1,       /* 将 int (1) 压栈 */
    iconst_2,       /* 将 int (2) 压栈 */
    iconst_3,       /* 将 int (3) 压栈 */
    iconst_4,       /* 将 int (4) 压栈 */
    iconst_5,       /* 将 int (5) 压栈 */
    lconst_0,       /* 将 long (0) 压栈 */
    lconst_1,       /* 将 long (1) 压栈 */
    fconst_0,       /* 将 float (0) 压栈 */
    fconst_1,       /* 将 float (1) 压栈 */
    fconst_2,       /* 将 float (2) 压栈 */
    dconst_0,       /* 将 double (0) 压栈 */
    dconst_1,       /* 将 double (1) 压栈 */
    bipush,         /* 将 1 字节常量 (-0x80 - 0x7f) 压栈 */
    sipush,         /* 将 2 字节常量 (-0x8000 - 0x7fff) 压栈 */
    ldc,            /* 将指定 index (1 字节) 常量值从常量池中压栈 (占 1 个操作数槽) */
    ldc_w,          /* 同上，index (2 字节) */
    ldc2_w,         /* 同上，占 2 个操作数栈 */
    iload,          /* 将指定的 int 本地变量压栈 */
    lload,          /* 将指定的 long 本地变量压栈 */
    fload,          /* 将指定的 float 本地变量压栈 */
    dload,          /* 将指定的 double 本地变量压栈 */
    aload,          /* 将指定的 引用类型 本地变量压栈 */
    iload_0,        /* 将第一个 int 本地变量压栈 */
    iload_1,        /* 将第二个 int 本地变量压栈 */
    iload_2,        /* 将第三个 int 本地变量压栈 */
    iload_3,        /* 将第四个 int 本地变量压栈 */
    lload_0,        /* 将第一个 long 本地变量压栈 */
    lload_1,        /* 将第二个 long 本地变量压栈 */
    lload_2,        /* 将第三个 long 本地变量压栈 */
    lload_3,        /* 将第四个 long 本地变量压栈 */
    fload_0,        /* 将第一个 float 本地变量压栈 */
    fload_1,        /* 将第二个 float 本地变量压栈 */
    fload_2,        /* 将第三个 float 本地变量压栈 */
    fload_3,        /* 将第四个 float 本地变量压栈 */
    dload_0,        /* 将第一个 double 本地变量压栈 */
    dload_1,        /* 将第二个 double 本地变量压栈 */
    dload_2,        /* 将第三个 double 本地变量压栈 */
    dload_3,        /* 将第四个 double 本地变量压栈 */
    aload_0,        /* 将第一个 引用类型 本地变量压栈 */
    aload_1,        /* 将第二个 引用类型 本地变量压栈 */
    aload_2,        /* 将第三个 引用类型 本地变量压栈 */
    aload_3,        /* 将第四个 引用类型 本地变量压栈 */
    iaload,         /* 将 int 数组指定索引的值压栈 */
    laload,         /* 将 long 数组指定索引的值压栈 */
    faload,         /* 将 float 数组指定索引的值压栈 */
    daload,         /* 将 double 数组指定索引的值压栈 */
    aaload,         /* 将 引用类型 数组指定索引的值压栈 */
    baload,         /* 将 boolean 或 byte 型数组指定索引的值压栈 */
    caload,         /* 将 char 型数组指定索引的值压栈 */
    saload,         /* 将 short 型数组指定索引的值压栈 */
    istore,         /* 将栈顶 int 数值存入指定本地变量 */
    lstore,         /* 将栈顶 long 数值存入指定本地变量 */
    fstore,         /* 将栈顶 float 数值存入指定本地变量 */
    dstore,         /* 将栈顶 double 数值存入指定本地变量 */
    astore,         /* 将栈顶 引用类型 存入指定本地变量 */
    istore_0,       /* 将栈顶 int 数值存入第一个本地变量 */
    istore_1,       /* 将栈顶 int 数值存入第二个本地变量 */
    istore_2,       /* 将栈顶 int 数值存入第三个本地变量 */
    istore_3,       /* 将栈顶 int 数值存入第四个本地变量 */
    lstore_0,       /* 将栈顶 long 数值存入第一个本地变量 */
    lstore_1,       /* 将栈顶 long 数值存入第二个本地变量 */
    lstore_2,       /* 将栈顶 long 数值存入第三个本地变量 */
    lstore_3,       /* 将栈顶 long 数值存入第四个本地变量 */
    fstore_0,       /* 将栈顶 float 数值存入第一个本地变量 */
    fstore_1,       /* 将栈顶 float 数值存入第二个本地变量 */
    fstore_2,       /* 将栈顶 float 数值存入第三个本地变量 */
    fstore_3,       /* 将栈顶 float 数值存入第四个本地变量 */
    dstore_0,       /* 将栈顶 double 数值存入第一个本地变量 */
    dstore_1,       /* 将栈顶 double 数值存入第二个本地变量 */
    dstore_2,       /* 将栈顶 double 数值存入第三个本地变量 */
    dstore_3,       /* 将栈顶 double 数值存入第四个本地变量 */
    astore_0,       /* 将栈顶 引用类型 存入第一个本地变量 */
    astore_1,       /* 将栈顶 引用类型 存入第二个本地变量 */
    astore_2,       /* 将栈顶 引用类型 存入第三个本地变量 */
    astore_3,       /* 将栈顶 引用类型 存入第四个本地变量 */
    iastore,        /* 将栈顶 int 数值存入指定数组指定位置 */
    lastore,        /* 将栈顶 long 数值存入指定数组指定位置 */
    fastore,        /* 将栈顶 float 数值存入指定数组指定位置 */
    dastore,        /* 将栈顶 double 数值存入指定数组指定位置 */
    aastore,        /* 将栈顶 引用类型 存入指定数组指定位置 */
    bastore,        /* 将栈顶 boolean 或 byte 数值存入指定数组指定位置 */
    castore,        /* 将栈顶 char 数值存入指定数组指定位置 */
    sastore,        /* 将栈顶 short 数值存入指定数组指定位置 */
    pop,            /* 将栈顶一个数弹出 */
    pop2,           /* 将栈顶两个数弹出 */
    dup,            /* 复制栈顶的数并压栈 */
    dup_x1,         /* 复制栈顶的数两次并压栈 */
    dup_x2,         /* 复制栈顶的数三次并压栈 */
    dup2,           /* 复制两个栈顶的数并压栈 */
    dup2_x1,        /* 复制两个栈顶的数两次并压栈 */
    dup2_x2,        /* 复制两个栈顶的数三次并压栈 */
    swap,           /* 交换栈顶两个数 */
    iadd,           /* 两个 int 相加并压栈 */
    ladd,           /* 两个 long 相加并压栈 */
    fadd,           /* 两个 float 相加并压栈 */
    dadd,           /* 两个 double 相加并压栈 */
    isub,           /* 两个 int 相减并压栈 */
    lsub,           /* 两个 long 相减并压栈 */
    fsub,           /* 两个 float 相减并压栈 */
    dsub,           /* 两个 double 相减并压栈 */
    imul,           /* 两个 int 相乘并压栈 */
    lmul,           /* 两个 long 相乘并压栈 */
    fmul,           /* 两个 float 相乘并压栈 */
    dmul,           /* 两个 double 相乘并压栈 */
    idiv,           /* 两个 int 相除并压栈 */
    _ldiv,           /* 两个 long 相除并压栈 */
    fdiv,           /* 两个 float 相除并压栈 */
    ddiv,           /* 两个 double 相除并压栈 */
    irem,           /* 两个 int 取模并压栈 */
    lrem,           /* 两个 long 取模并压栈 */
    frem,           /* 两个 float 取模并压栈 */
    drem,           /* 两个 long 取模并压栈 */
    ineg,           /* 将 int 取负数并压栈 */
    lneg,           /* 将 long 取负数并压栈 */
    fneg,           /* 将 float 取负数并压栈 */
    dneg,           /* 将 double 取负数并压栈 */
    ishl,           /* 将 int 左移指定位数并压栈 */
    lshl,           /* 将 long 左移指定位数并压栈 */
    ishr,           /* 将 int 右移指定位数并压栈 */
    lshr,           /* 将 long 右移指定位数并压栈 */
    iushr,          /* 将 int 无符号右移指定位数并压栈 */
    lushr,          /* 将 long 无符号右移指定位数并压栈 */
    iand,           /* 两个 int 相与并压栈 */
    land,           /* 两个 long 相与并压栈 */
    ior,            /* 两个 int 相或并压栈 */
    lor,            /* 两个 long 相或并压栈 */
    ixor,           /* 两个 int 异与并压栈 */
    lxor,           /* 两个 long 异与并压栈 */
    iinc,           /* 指定 int 变量自增指定值 */
    i2l,            /* 栈顶 int 转为 long 并压栈 */
    i2f,            /* 栈顶 int 转为 float 并压栈 */
    i2d,            /* 栈顶 int 转为 double 并压栈 */
    l2i,            /* 栈顶 long 转为 int 并压栈 */
    l2f,            /* 栈顶 long 转为 float 并压栈 */
    l2d,            /* 栈顶 long 转为 double 并压栈 */
    f2i,            /* 栈顶 float 转为 int 并压栈 */
    f2l,            /* 栈顶 float 转为 long 并压栈 */
    f2d,            /* 栈顶 float 转为 double 并压栈 */
    d2i,            /* 栈顶 double 转为 int 并压栈 */
    d2l,            /* 栈顶 double 转为 long 并压栈 */
    d2f,            /* 栈顶 double 转为 float 并压栈 */
    i2b,            /* 栈顶 int 转为 boolean/byte 并压栈 */
    i2c,            /* 栈顶 int 转为 char 并压栈 */
    i2s,            /* 栈顶 int 转为 short 并压栈 */
    lcmp,           /* 比较栈顶两个 long 并将结果 (-1, 0, 1) 压入栈顶 */
    fcmpl,          /* 比较栈顶两个 float 并将结果 (-1, 0, 1) 压入栈顶. 当其中一个是 NaN 时，压入 -1 */
    fcmpg,          /* 比较栈顶两个 float 并将结果 (-1, 0, 1) 压入栈顶. 当其中一个是 NaN 时，压入 1 */
    dcmpl,          /* 比较栈顶两个 double 并将结果 (-1, 0, 1) 压入栈顶. 当其中一个是 NaN 时，压入 -1 */
    dcmpg,          /* 比较栈顶两个 double 并将结果 (-1, 0, 1) 压入栈顶. 当其中一个是 NaN 时，压入 1 */
    ifeq,           /* 栈顶 int 等于 0 时跳转 */
    ifne,           /* 栈顶 int 不是 0 时跳转 */
    iflt,           /* 栈顶 int 小于 0 时跳转 */
    ifge,           /* 栈顶 int 大于等于 0 时跳转 */
    ifgt,           /* 栈顶 int 大于 0 时跳转 */
    ifle,           /* 栈顶 int 小于等于 0 时跳转 */
    if_icmpeq,      /* 比较栈顶两 int，结果等于 0 时跳转 */
    if_icmpne,      /* 比较栈顶两 int，结果不是 0 时跳转 */
    if_icmplt,      /* 比较栈顶两 int，结果小于 0 时跳转 */
    if_icmpge,      /* 比较栈顶两 int，结果大于等于 0 时跳转 */
    if_icmpgt,      /* 比较栈顶两 int，结果大于 0 时跳转 */
    if_icmple,      /* 比较栈顶两 int，结果小于等于 0 时跳转 */
    if_acmpeq,      /* 比较栈顶两个 引用类型，结果等于 0 时跳转 */
    if_acmpne,      /* 比较栈顶两个 引用类型，结果不是 0 时跳转 */
    _goto,          /* 无条件跳转 */
    jsr,            /* 跳转到指定的 16 位偏移量地址，并将下一条指令地址压栈 */
    ret,            /* 返回到本地变量指定的偏移量地址 */
    tableswitch,    /* switch 跳转，case 值连续 */
    lookupswitch,   /* switch 跳转，case 值不连续 */
    ireturn,        /* 返回 int */
    lreturn,        /* 返回 long */
    freturn,        /* 返回 float */
    dreturn,        /* 返回 double */
    areturn,        /* 返回 引用类型 */
    _return,        /* 返回 void */
    getstatic,      /* 将指定类的静态字段压栈 */
    putstatic,      /* 为指定类的静态字段赋值 */
    getfield,       /* 将指定类的非静态字段压栈 */
    putfield,       /* 为指定类的非静态字段赋值 */
    invokevirtual,  /* 调用虚函数 */
    invokespecial,  /* 调用构造函数，私有函数 */
    invokestatic,   /* 调用静态函数 */
    invokeinterface,/* 调用接口函数 */
    invokedynamic,  /* 调用动态函数 */
    _new,           /* 创建对象，并将引用压栈 */
    newarray,       /* 创建指定类型数组，并将引用压栈 */
    anewarray,      /* 创建引用类型数组，并将引用压栈 */
    arraylength,    /* 获取数组长度并压栈 */
    athrow,         /* 抛出栈顶异常 */
    checkcast,      /* 检查类型转换，失败抛出异常 */
    instanceof,     /* 判断是否是实例，并将结果 (0, 1) 压栈 */
    monitorenter,   /* 获取栈顶元素的锁 */
    monitorexit,    /* 释放锁 */
    wide,           /* 拓展本地变量宽度 */
    multianewarray, /* 创建多维数组并压栈 (操作栈中必须包含各维度的长度值) */
    ifnull,         /* 为 null 时跳转 */
    ifnonnull,      /* 不是 null 时跳转 */
    goto_w,         /* 无条件跳转 (宽索引) */
    jsr_w,          /* 跳转到指定的 32 位偏移量地址，并将下一条指令地址压栈 */
} ;
