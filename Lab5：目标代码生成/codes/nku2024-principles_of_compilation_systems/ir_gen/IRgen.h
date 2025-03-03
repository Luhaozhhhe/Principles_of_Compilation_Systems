#ifndef IRGEN_H
#define IRGEN_H

#include "../include/SysY_tree.h"
#include "../include/cfg.h"
#include "../include/symtab.h"
#include "../include/type.h"
#include <assert.h>
#include <map>
#include <vector>

class IRgenTable {
public:
    // 如果你无从下手,推荐先阅读LLVMIR类的printIR函数,了解我们是如何输出中间代码的
    // 然后你可以尝试往和输出相关联的变量中随便添加一些函数定义指令, 新建一些基本块或添加几条指令看看输出是怎么变化的
    // 弄懂LLVMIR类是如何存储中间代码后，剩余的就是理解中间代码生成算法了

    // TODO():添加更多你需要的成员变量和成员函数
    SymbolRegTable symbol_reg_table;//symbol->int(查看一个符号的寄存器类型)
    std::map<int, VarAttribute> Reg_VarA_Table;//如果symbol->varA，容易出现作用域、重命名的问题
    Operand current_putin_ptr = nullptr;
    //map<int,VarAttribute>((reg_num),Attribute)
    std::map<int, int> Formal_Array_Table;//映射表，键是寄存器编号，值是数组的元信息
    IRgenTable() {}
};

#endif