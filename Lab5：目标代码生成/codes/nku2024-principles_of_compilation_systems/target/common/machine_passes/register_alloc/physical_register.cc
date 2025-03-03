#include "physical_register.h"


int PhysicalRegistersAllocTools::getIdleReg(LiveInterval interval) {
    //TODO("getIdleReg");
    auto valid_regs = getValidRegs(interval); // 获取所有合法寄存器
    for (int reg : valid_regs) {
        //遍历所有可能的寄存器，看看是不是冲突
        bool is_idle = true;
        for (const auto &occupied_interval : phy_occupied[reg]) {
            if (interval & occupied_interval) {
                is_idle = false;
                break;
            }
        }
        if (is_idle) {
            return reg; // 找到空闲寄存器
        }
    }
    return -1; // 没有找到空闲寄存器
}

int PhysicalRegistersAllocTools::getIdleMem(LiveInterval interval) { 
    // TODO("getIdleMem"); 
    int offset = 0;
    for (const auto &intervals : mem_occupied) {
        //遍历所有可能的内存，看看是不是有不冲突的地方，不然就需要再偏移，分配新位置
        bool is_idle = true;
        for (const auto &occupied_interval : intervals) {
            if (interval & occupied_interval) {
                is_idle = false;
                break;
            }
        }
        if (is_idle) {
            return offset; // 找到空闲内存
        }
        ++offset;
    }
    // 如果没有找到空闲内存，分配新内存位置
    mem_occupied.emplace_back();
    return mem_occupied.size() - 1;//offset+2-1的位置
}

int PhysicalRegistersAllocTools::swapRegspill(int p_reg1, LiveInterval interval1, int offset_spill2, int size,
                                              LiveInterval interval2) {

    // //TODO("swapRegspill");
    if (ReleaseReg(p_reg1, interval1) && OccupyMem(offset_spill2, interval1)) {
        OccupyReg(p_reg1, interval2);
        return 0; // 成功交换
    }
    ERROR("swap reg wrong");
    return -1; // 交换失败
}

std::vector<LiveInterval> PhysicalRegistersAllocTools::getConflictIntervals(LiveInterval interval) {
    std::vector<LiveInterval> result;
    for (auto phy_intervals : phy_occupied) {
        for (auto other_interval : phy_intervals) {
            if (interval.getReg().type == other_interval.getReg().type && (interval & other_interval)) {
                result.push_back(other_interval);
            }
        }
    }
    return result;
}

bool PhysicalRegistersAllocTools::ReleaseMem(int offset, LiveInterval interval) {
    //TODO("ReleaseMem");
    auto &Intervals = mem_occupied[offset];
    for (auto it = Intervals.begin(); it != Intervals.end(); ++it) {
        if ((*it).getReg() == interval.getReg()) {
            Intervals.erase(it);
            
            return true; // 释放成功
        }
    }
    return false; // 没找到对应区间
}


//Reference: https://github.com/yuhuifishash/SysY
bool PhysicalRegistersAllocTools::OccupyReg(int phy_id, LiveInterval interval) {
    // 你需要保证interval不与phy_id已有的冲突
    // 或者增加判断分配失败返回false的代码
    phy_occupied[phy_id].push_back(interval);   //一个interval对应的是一个寄存器，本质是推入了一系列不冲突的寄存器
    return true;
}
//Reference: https://github.com/yuhuifishash/SysY
bool PhysicalRegistersAllocTools::ReleaseReg(int phy_id, LiveInterval interval) { 
    //TODO("ReleaseReg"); 
    //就是把phy_occupied中的删掉
    auto it = phy_occupied[phy_id].begin();
    for (; it != phy_occupied[phy_id].end(); ++it) {
        if ((*it).getReg() == interval.getReg()) {
            phy_occupied[phy_id].erase(it);
            return true;
        }
    }
    //ERROR("ReleaseReg false");
    return false;
}
//Reference: https://github.com/yuhuifishash/SysY
bool PhysicalRegistersAllocTools::OccupyMem(int offset, LiveInterval interval) {
    //TODO("OccupyMem");
    // for (const auto &existing_interval : mem_occupied[offset]) {
    //     if (interval & existing_interval) {
    //         //ERROR("OccupyMem false");
    //         return false; // 如果冲突，分配失败
    //     }
    // }
    // // 添加到内存占用记录
    // mem_occupied[offset].push_back(interval);
    // return true;
    //每个数据都按2*2=4Bytes算，占据两个offset
    for (int i = offset; i < offset + 2; i++) {
        while (i >= mem_occupied.size()) {
            mem_occupied.push_back({});//让mem_occupy的值增加。
        }
        mem_occupied[i].push_back(interval);//在应该占用的地方（offset）放置上对应的interval
        //mem_occupied
        //0    offset - 1, offset,    offset + 1,    
        //{}...{}        , {interval}, {interval}
    }
    return true;
}
