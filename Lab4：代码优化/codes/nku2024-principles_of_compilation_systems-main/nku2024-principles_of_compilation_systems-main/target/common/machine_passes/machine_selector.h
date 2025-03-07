#ifndef MACHINE_SELECTOR_H
#define MACHINE_SELECTOR_H
#include "../machine_instruction_structures/machine.h"
class MachineSelector {
    // 指令选择基类
protected:
    MachineUnit *dest;
    MachineFunction *cur_func;
    MachineBlock *cur_block;
    LLVMIR *IR;//存储 LLVM 中间表示，用于作为指令选择的源信息

public:
    MachineSelector(MachineUnit *dest, LLVMIR *IR) : dest(dest), IR(IR) {}
    virtual void SelectInstructionAndBuildCFG() = 0;
    MachineUnit *GetMachineUnit() { return dest; }
};
#endif