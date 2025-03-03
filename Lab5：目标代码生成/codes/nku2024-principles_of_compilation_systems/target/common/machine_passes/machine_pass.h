#ifndef MACHINE_OTHER_PASS_H
#define MACHINE_OTHER_PASS_H
#include "../machine_instruction_structures/machine.h"
class MachinePass {
protected:
    MachineUnit *unit;//一个机器代码单元
    MachineFunction *current_func;//当前正在处理的函数
    MachineBlock *cur_block;//当前正在处理的基本块

public:
    virtual void Execute() = 0;
    MachinePass(MachineUnit *unit) : unit(unit) {}
};
#endif