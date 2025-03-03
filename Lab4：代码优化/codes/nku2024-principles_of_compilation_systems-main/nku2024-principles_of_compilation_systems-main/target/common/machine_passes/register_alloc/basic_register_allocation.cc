#include "basic_register_allocation.h"

void RegisterAllocation::Execute() {
    // 你需要保证此时不存在phi指令
    assert(not_allocated_funcs.size()==0);
    for (auto func : unit->functions) {
        not_allocated_funcs.push(func);
    }
    while (!not_allocated_funcs.empty()) {
        current_func = not_allocated_funcs.front();
        numbertoins.clear();
        // 对每条指令进行编号，一个函数里不会重复
        InstructionNumber(unit, numbertoins).ExecuteInFunc(current_func);

        // 需要清除之前分配的结果
        alloc_result[current_func].clear();
        not_allocated_funcs.pop();

        // 计算活跃区间
        UpdateIntervalsInCurrentFunc();

        if (DoAllocInCurrentFunc()) {    // 尝试进行分配
            // 如果发生溢出，插入spill指令后将所有物理寄存器退回到虚拟寄存器，重新分配
            spiller->ExecuteInFunc(current_func, &alloc_result[current_func]);    // 生成溢出代码
            current_func->AddStackSize(phy_regs_tools->getSpillSize());                 // 调整栈的大小
            not_allocated_funcs.push(current_func);                               // 重新分配直到不再spill

        }
    }
    // 重写虚拟寄存器，全部转换为物理寄存器
    VirtualRegisterRewrite(unit, alloc_result).Execute();
}

void InstructionNumber::Execute() {
    for (auto func : unit->functions) {
        ExecuteInFunc(func);
    }
}

void InstructionNumber::ExecuteInFunc(MachineFunction *func) {
    // 对每个指令进行编号(用于计算活跃区间)
    int count_begin = 0;
    current_func = func;
    auto it = func->getMachineCFG()->getBFSIterator();
    it->open();
    while (it->hasNext()) {
        auto mcfg_node = it->next();
        auto mblock = mcfg_node->Mblock;
        // 每个基本块开头会占据一个编号
        this->numbertoins[count_begin] = InstructionNumberEntry(nullptr, true);
        count_begin++;
        for (auto ins : *mblock) {
            this->numbertoins[count_begin] = InstructionNumberEntry(ins, false);
            ins->setNumber(count_begin++);
        }
    }
}

void RegisterAllocation::UpdateIntervalsInCurrentFunc() {
    intervals.clear();
    //intervals.reserve(10000);
    auto mfun = current_func;
    auto mcfg = mfun->getMachineCFG();

    Liveness liveness(mfun);

    auto it = mcfg->getReverseIterator(mcfg->getBFSIterator());
    it->open();

    std::map<Register, int> last_def, last_use;

    while (it->hasNext()) {
        auto mcfg_node = it->next();
        auto mblock = mcfg_node->Mblock;
        auto cur_id = mcfg_node->Mblock->getLabelId();
        for (auto reg : liveness.GetOUT(cur_id)) {
            if (intervals.find(reg) == intervals.end()) 
            {
                intervals[reg] = LiveInterval(reg);
                //申请一个新的Interval保存reg的Interval
            }
            if (last_use.find(reg) == last_use.end()) 
            {

                intervals[reg].PushFront(mblock->getBlockInNumber(), mblock->getBlockOutNumber());
                //第一条指令的号码-最后一条指令的号码为一段push进入
            } 
            else 
            {
                intervals[reg].PushFront(mblock->getBlockInNumber(), mblock->getBlockOutNumber());
            }
            last_use[reg] = mblock->getBlockOutNumber();//更新最后一次使用
        }
        //反向遍历指令
        for (auto reverse_it = mcfg_node->Mblock->ReverseBegin(); reverse_it != mcfg_node->Mblock->ReverseEnd(); ++reverse_it) 
        {
            auto ins = *reverse_it;
            for (auto reg : ins->GetWriteReg()) 
            {
                last_def[*reg] = ins->getNumber();

                if (intervals.find(*reg) == intervals.end()) 
                {
                    intervals[*reg] = LiveInterval(*reg);
                }

                if (last_use.find(*reg) != last_use.end()) 
                {
                    last_use.erase(*reg);//第一次找到，就说明确实是最后一次用
                    intervals[*reg].SetMostBegin(ins->getNumber());
                } 
                else 
                {
                    intervals[*reg].PushFront(ins->getNumber(), ins->getNumber());
                }
                //反正就要把这个区间放到intervals里面
                intervals[*reg].IncreaseReferenceCount(1);
                //增加引用计数
            }
            for (auto reg : ins->GetReadReg()) //read类似，但是更新的是use
            {
                if (intervals.find(*reg) == intervals.end()) 
                {
                    intervals[*reg] = LiveInterval(*reg);
                }

                if (last_use.find(*reg) != last_use.end()) 
                {

                } 
                else 
                {
                    intervals[*reg].PushFront(mblock->getBlockInNumber(), ins->getNumber());
                }

                last_use[*reg] = ins->getNumber();

                intervals[*reg].IncreaseReferenceCount(1);
            }
        }
        last_use.clear();
        last_def.clear();
    }
    // 你可以在这里输出intervals的值来获得活跃变量分析的结果
    // 观察结果可能对你寄存器分配算法的编写有一定帮助
}