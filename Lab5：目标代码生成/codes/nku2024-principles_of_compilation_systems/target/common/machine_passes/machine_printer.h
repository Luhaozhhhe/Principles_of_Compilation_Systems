#ifndef MACHINE_PRINTER_H
#define MACHINE_PRINTER_H
#include "../machine_instruction_structures/machine.h"
class MachinePrinter {
    // 指令打印基类
protected:
    MachineUnit *printee;//要打印的机器代码单元
    MachineFunction *current_func;//当前正在处理的函数
    MachineBlock *cur_block;//当前正在处理的基本块
    std::ostream &s;//指定输出流
    bool output_physical_reg;//控制是否输出物理寄存器,默认为false

public:
    virtual void emit() = 0;
    MachinePrinter(std::ostream &s, MachineUnit *printee) : s(s), printee(printee), output_physical_reg(false) {}
    void SetOutputPhysicalReg(bool outputPhy) { output_physical_reg = outputPhy; }
    std::ostream &GetPrintStream() { return s; }
};
#endif