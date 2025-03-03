#include "fast_linear_scan.h"
bool IntervalsPrioCmp(LiveInterval a, LiveInterval b) { return a.begin()->begin > b.begin()->begin; }
FastLinearScan::FastLinearScan(MachineUnit *unit, PhysicalRegistersAllocTools *phy, SpillCodeGen *spiller)
    : RegisterAllocation(unit, phy, spiller), unalloc_queue(IntervalsPrioCmp) {}
bool FastLinearScan::DoAllocInCurrentFunc() {
    bool spilled = false;
    auto mfun = current_func;
    PRINT("FastLinearScan: %s", mfun->getFunctionName().c_str());
    // std::cerr<<"FastLinearScan: "<<mfun->getFunctionName()<<"\n";
    phy_regs_tools->clear();
    for (auto interval : intervals) {
        Assert(interval.first == interval.second.getReg());
        if (interval.first.is_virtual) {
            // 需要分配的虚拟寄存器
            unalloc_queue.push(interval.second);
        } else {
            // 预先占用已经存在的物理寄存器
            phy_regs_tools->OccupyReg(interval.first.reg_no, interval.second);
        }
    }
    // TODO: 进行线性扫描寄存器分配, 为每个虚拟寄存器选择合适的物理寄存器或者将其溢出到合适的栈地址中
    // 该函数中只需正确设置alloc_result，并不需要实际生成溢出代码
    // TODO("LinearScan");
    // dReference: https://github.com/yuhuifishash/SysY
    while (!unalloc_queue.empty()) {
        // 根据寄存器的生存区间（live intervals），逐一遍历和分配寄存器。
        // 如果区间冲突（生存期重叠），则需要解决冲突：要么分配其他寄存器，要么将寄存器内容溢出到内存。

        auto current = unalloc_queue.top(); // 获取当前优先级最高的区间
        unalloc_queue.pop();
        auto cur_vreg = current.getReg();
        std::vector<int> prefered_regs, noprefer_regs;
        // 尝试分配物理寄存器
        int idle_reg = phy_regs_tools->getIdleReg(current);
        if (idle_reg != -1) 
        {
            // 分配到物理寄存器，直接占据
            phy_regs_tools->OccupyReg(idle_reg, current);
            AllocPhyReg(mfun, cur_vreg, idle_reg);
        } 
        else 
        {
            // 没有空闲的物理寄存器，需要存到栈上，需要注意增加栈偏移
            spilled = true;
            int mem = phy_regs_tools->getIdleMem(current);
            
            phy_regs_tools->OccupyMem(mem, current);

            AllocStack(mfun, cur_vreg, mem);

            double spill_weight = CalculateSpillWeight(current);
            auto spill_interval = current;
            for (auto other : phy_regs_tools->getConflictIntervals(current)) {
                double other_weight = CalculateSpillWeight(other);
                if (spill_weight > other_weight && other.getReg().is_virtual) {
                    spill_weight = other_weight;
                    spill_interval = other;
                }
            }
            // 找到优先级最低的，进行替换
            
            if (!(current.getReg() == spill_interval.getReg())) {
                phy_regs_tools->swapRegspill(
                                                getAllocResultInReg(mfun, spill_interval.getReg()), 
                                                spill_interval, 
                                                mem,
                                                cur_vreg.getDataWidth(), 
                                                current
                                            );
                //将被替换的溢出，现在的分配寄存器（如果没有能替换的就溢出了），但是这里并不会给interval1分配内存，所以还要占据内存
                swapAllocResult(mfun, current.getReg(), spill_interval.getReg()); //替换时还要交换Alloc结果
                int spill_mem = phy_regs_tools->getIdleMem(spill_interval); 
                phy_regs_tools->OccupyMem(spill_mem, spill_interval);
                AllocStack(mfun, spill_interval.getReg(), spill_mem);
                //栈的增量需要看mem的占据情况
            }
        }
    }

    // 返回是否发生溢出
    return spilled;
}

// 计算溢出权重
double FastLinearScan::CalculateSpillWeight(LiveInterval interval) {
    return (double)interval.getReferenceCount() / interval.getIntervalLen();
}
