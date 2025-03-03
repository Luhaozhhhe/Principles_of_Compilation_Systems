#ifndef RISCV64_INSTSELECT_H
#define RISCV64_INSTSELECT_H
#include "../../common/machine_passes/machine_selector.h"
#include "../../common/machine_instruction_structures/machine.h"
#include "../riscv64.h"

class RiscV64Selector : public MachineSelector {
private:
    int cur_offset;    // 局部变量在栈中的偏移
    // 你需要保证在每个函数的指令选择结束后, cur_offset的值为局部变量所占栈空间的大小
    //Reference: https://github.com/yuhuifishash/SysY

    //用于重用寄存器，如果操作数的号有过寄存器申请，就用对应的寄存器，否则就申请一个。
    std::map<int, Register> llvmreg_reuse_table;

    //用于记录alloc操作数号的偏移量
    std::map<int, int> llvm_riscv_allocas;//added

    //用于记录一个比较的结果存的是哪个指令的比较结果
    std::map<Register, Instruction> cmp_context;//added

    Register GetNewReg(MachineDataType, bool save_across_call = false);

    // TODO(): 添加更多你需要的成员变量和函数
public:
    Register GetllvmReg(int, MachineDataType);

    RiscV64Selector(MachineUnit *dest, LLVMIR *IR) : MachineSelector(dest, IR) {}
    void SelectInstructionAndBuildCFG();//遍历每一个函数的每一个块，对里面的指令进行选择，并构建CFG，Parents关系。
    void ClearFunctionSelectState();
    template <class INSPTR> void ConvertAndAppend(INSPTR);
};
#endif