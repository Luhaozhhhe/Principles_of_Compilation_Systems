#include "../../include/cfg.h"
#include "dominator_tree.h"
#include <functional>
#include "adce.h"
#include <chrono>
//Reference: https://github.com/yuhuifishash/SysY
// 构建控制依赖图（CDG）的函数，参数为指向CFG（控制流图）的指针
std::vector<std::vector<LLVMBlock>> ADCEPass::BuildCDG(CFG *C) {
    std::vector<std::vector<LLVMBlock>> CDG;
    std::vector<std::vector<LLVMBlock>> CDG_precursor;
    std::vector<int> rd;
    // 获取CFG中的正向图和反向图
    auto G = C->G;
    auto invG = C->invG;

    // std::cout<<"bulid CDG begin"<<std::endl;

    // 定义支配树指针，后续用于获取或构建支配树相关操作
    DominatorTree* dt;
    // 检查是否已经存在该CFG对应的支配树，如果存在则获取它
    if (domtrees->hasDomTree(C)) {
        dt = domtrees->GetDomTree(C);
    }
    else {
        // 如果不存在，先将当前CFG赋值给dt指向的对象
        dt->C = C;
        dt->BuildDominatorTree();
    }
    // std::cout<<"lhz"<<dt->df.size()<<std::endl;

    // 获取支配树副本（解引用dt指针获取其指向的对象内容）
    auto DomTree = *dt;

    // std::cout<<"debug the dom_tree_reverse"<<std::endl;

    // 定义后支配树指针，用于后续获取或构建后支配树相关操作
    DominatorTree* dt_reverse ;
    // 检查是否已经存在该CFG对应的后支配树，如果存在则获取它
    if (domtrees->hasPostDomTree(C)) {
        dt_reverse = domtrees->GetPostDomTree(C);
        //std::cout<<"dt atdomsize: "<<dt_reverse->atdom.size()<<std::endl;

    }
    else {
        // 如果不存在，先将当前CFG赋值给dt_reverse指向的对象
        dt_reverse->C = C;
        dt_reverse->BuildPostDominatorTree();
        //std::cout<<"dt atdomsize: "<<dt_reverse->atdom.size()<<std::endl;
        
    }
    // std::cout<<"lhz"<<dt_reverse->df.size()<<std::endl;
    // std::cout<<"4"<<std::endl;

    // 获取后支配树副本
    auto PostDomTree = *dt_reverse;
    // std::cout<<"atdomsize: "<<PostDomTree.atdom.size()<<std::endl;
    // std::cout<<"dfsize: "<<PostDomTree.df.size()<<std::endl;
    // std::cout<<"5"<<std::endl;

    // 获取CFG中基本块映射表（推测是基本块编号到基本块对象的映射）
    auto blockmap = (*C->block_map);
    // 根据最大标号调整CDG的大小，为存储各基本块之间的控制依赖关系做准备
    CDG.resize(C->max_label + 2);
    // 初始化入度数组，用于记录每个基本块的入度（控制依赖的数量）
    rd.resize(C->max_label + 1, 0);
    // 调整CDG前驱相关容器大小，同样为后续记录控制依赖前驱信息做准备
    CDG_precursor.resize(C->max_label + 1);
    // 遍历所有基本块编号（从0到最大标号）来构建控制依赖图相关信息
    for (int i = 0; i <= C->max_label; ++i) {
        // std::cout<<"i = "<<i<<std::endl;
        // std::cout<<"dfsize: "<<PostDomTree.df.size()<<std::endl;
        
        // 获取当前基本块编号对应的支配边界
        auto domFrontier = PostDomTree.GetDF(i);
        // std::cout<<"66"<<std::endl;

        // 遍历支配边界中的每个基本块编号
        for (auto vbbid : domFrontier) {
            // 在CDG中添加控制依赖关系，即将blockmap[i]这个基本块添加到目标基本块vbbid的依赖列表中
            CDG[vbbid].push_back(blockmap[i]);
            // 如果目标基本块编号不等于当前基本块编号，说明是外部依赖，入度加1
            if (vbbid!= i) {
                rd[i]++;
            }
            // 在CDG前驱容器中添加对应的前驱基本块信息，用于后续反向查找依赖关系等操作
            CDG_precursor[blockmap[i]->block_id].push_back(blockmap[vbbid]);
        }
    }
    // 遍历所有基本块编号，将入度为0的基本块添加到特殊的“终结”基本块（这里用最大标号 + 1表示）的依赖列表中
    for (int i = 0; i <= C->max_label; ++i) {
        if (!rd[i]) {
            CDG[C->max_label + 1].push_back(blockmap[i]);
        }
    }
    // 返回控制依赖图的前驱相关信息（用于后续分析等操作）
    return CDG_precursor;
    // std::cout<<"Build CDG end!!!"<<std::endl;

}

// 查找给定CFG中指定基本块编号对应的基本块里的最后一条指令，作为该基本块的“终结”指令
static Instruction FindTerminal(CFG *C, int bbid) {
    auto blockmap = (*C->block_map);
    auto bb = blockmap[bbid];
    return bb->Instruction_list.back();
}

// 执行活跃变量分析及无用代码消除（ADCE）的主要函数，参数为指向CFG的指针
// Reference: https://github.com/yuhuifishash/SysY
void ADCEPass::ADCE(CFG* C){
    // std::cout<<"1"<<std::endl;

    // 定义指令工作列表，用于存放待分析的指令，采用双端队列方便在两端操作
    std::deque<Instruction> worklist;
    // 定义定义映射表，用于记录寄存器到定义它的指令之间的映射关系
    std::map<int, Instruction> defmap;
    // 定义活跃指令集合，用于标记哪些指令在程序执行过程中是活跃的
    std::set<Instruction> liveInstructionset;
    // 定义活跃基本块集合，用于标记哪些基本块包含活跃指令
    std::set<int> liveBBset;

    // std::cout<<"2"<<std::endl;
    // 获取当前时间，用于记录构建控制依赖图（CDG）开始时间，方便后续统计耗时
    auto end0 = std::chrono::high_resolution_clock::now();
    
    // 调用BuildCDG函数构建控制依赖图，并获取其前驱相关信息
    auto CDG_precursor = BuildCDG(C);
    // std::cout<<"3"<<std::endl;
    

    auto end1 = std::chrono::high_resolution_clock::now();

    // std::cout << "Buildcdg time: "
    //            << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - end0).count()
    //            << " ms" << std::endl;

    // 获取CFG的反向图（之前在BuildCDG函数中也获取过，这里可能在后续多处用到）
    auto invG = C->invG;

    // std::cout<<"invG = C->invG"<<std::endl;

    // 定义支配树指针，后续用于获取或构建支配树相关操作（与BuildCDG函数中类似操作）
    DominatorTree* dt;
    // 检查是否已经存在该CFG对应的支配树，如果存在则获取它
    if (domtrees->hasDomTree(C)) {
        dt = domtrees->GetDomTree(C);
    }
    else {
        dt->C = C;
        dt->BuildDominatorTree();
    }
    // std::cout<<"lhz"<<dt->df.size()<<std::endl;
    
    // 获取支配树副本
    auto DomTree = *dt;

    // 获取支配树中直接支配节点的映射信息
    auto DomTreeidom = DomTree.idom;

    // 定义后支配树指针，用于后续获取或构建后支配树相关操作
    DominatorTree* dt_reverse ;
    // 检查是否已经存在该CFG对应的后支配树，如果存在则获取它
    if (domtrees->hasPostDomTree(C)) {
        dt_reverse = domtrees->GetPostDomTree(C);
    }
    else {
        dt_reverse->C = C;
        dt_reverse->BuildPostDominatorTree();
    }
    // 获取后支配树副本
    auto PostDomTree = *dt_reverse;
    // 获取后支配树中直接支配节点的映射信息
    auto PostDomTreeidom = PostDomTree.idom;

    // std::cout<<"PostDomTreeidom = PostDomTree.idom;"<<std::endl;
    // 获取CFG的基本块映射表
    auto blockmap = *C->block_map;
    // std::cout<<"0"<<std::endl;

    // 遍历基本块映射表中的每个基本块，进行指令相关的初始化和活跃性相关的初步筛选
    for (auto [id, bb] : blockmap) {
        // std::cout<<"1"<<std::endl;

        // 遍历基本块中的每条指令
        for (auto I : bb->Instruction_list) {
            // std::cout<<"2"<<std::endl;

            // 设置指令所属的基本块编号，方便后续根据基本块来分析指令活跃性等情况
            I->SetBlockID(id);
            // std::cout<<"3"<<std::endl;

            // 如果指令是存储指令、调用指令或者返回指令，将其加入工作列表，因为这些指令通常对程序结果有影响，是活跃性分析的重要起点
            if (I->GetOpcode() == BasicInstruction::STORE || I->GetOpcode() == BasicInstruction::CALL || I->GetOpcode() == BasicInstruction::RET) {
                // std::cout<<"4"<<std::endl;

                worklist.push_back(I);
                // std::cout<<"5"<<std::endl;

            }
            // 如果指令有结果寄存器（说明它会定义一个寄存器的值），将寄存器编号和该指令添加到定义映射表中
            if (I->GetResultReg()!= nullptr) {
                // std::cout<<"6"<<std::endl;
                
                defmap[I->GetResultRegNo()] = I;
                // std::cout<<"7"<<std::endl;

            }
        }
    }

    // std::cout<<"8"<<std::endl;

    // 主循环，不断从工作列表中取出指令进行活跃性分析，直到工作列表为空
    while (!worklist.empty()) {
        // std::cout<<"9"<<std::endl;

        // 获取工作列表头部的指令（先进先出原则）
        auto I = worklist.front();
        // std::cout<<"10"<<std::endl;

        // 将该指令从工作列表头部移除
        worklist.pop_front();
        // std::cout<<"11"<<std::endl;

        // 如果该指令已经在活跃指令集合中，说明已经分析过，跳过本次循环
        if (liveInstructionset.find(I)!= liveInstructionset.end()) {
            continue;
        }
        // 将该指令添加到活跃指令集合中，表示当前确定它是活跃的
        liveInstructionset.insert(I);
        // 获取该指令所在的基本块编号
        auto parBBno = I->GetBlockID();
        // 获取该指令所在的基本块对象
        auto parBB = blockmap[I->GetBlockID()];
        // 将该基本块编号添加到活跃基本块集合中
        liveBBset.insert(parBBno);
        // 如果该指令是PHI指令（在SSA形式中用于合并不同路径的值），需要特殊处理其活跃性分析
        if (I->GetOpcode() == BasicInstruction::PHI) {
            auto PhiI = (PhiInstruction *)I;
            for (auto [Labelop, Regop] : PhiI->GetPhiList()) {
                auto Label = (LabelOperand *)Labelop;
                auto Labelno = Label->GetLabelNo();
                auto terminalI = FindTerminal(C, Labelno);
                if (liveInstructionset.find(terminalI) == liveInstructionset.end()) {
                    worklist.push_front(terminalI);
                    liveBBset.insert(Labelno);
                }
            }
        }

        // 如果该指令所在基本块编号不是无效编号（通常 -1表示无效等特殊情况）
        if (parBBno!= -1) {
            // 遍历该基本块在控制依赖图中的前驱基本块
            for (auto CDG_pre : CDG_precursor[parBBno]) {
                auto CDG_preno = CDG_pre->block_id;
                auto terminalI = FindTerminal(C, CDG_preno);
                if (liveInstructionset.find(terminalI) == liveInstructionset.end()) {
                    worklist.push_front(terminalI);
                }
            }
        }

        // 遍历该指令的非结果操作数（即作为输入使用的操作数）
        for (auto op : I->GetNonResultOperands()) {
            // 如果操作数类型是寄存器类型
            if (op->GetOperandType() == BasicOperand::REG) {
                auto Regop = (RegOperand *)op;
                auto Regopno = Regop->GetRegNo();
                // 如果在定义映射表中能找到该寄存器对应的定义指令
                if (defmap.find(Regopno) == defmap.end()) {
                    continue;
                }
                auto DefI = defmap[Regopno];
                if (liveInstructionset.find(DefI) == liveInstructionset.end()) {
                    worklist.push_front(DefI);
                }
            }
        }
    }
    
    // std::cout<<"debug begin!"<<std::endl;

    // 遍历所有基本块，根据活跃性分析结果清理基本块中的无用指令，并调整控制流等
    for (auto [id, bb] : *C->block_map) {
        // std::cout<<"lhz"<<std::endl;

        auto terminalI = FindTerminal(C, id);
        auto tmp_Instruction_list = bb->Instruction_list;
        bb->Instruction_list.clear();
        for (auto I : tmp_Instruction_list) {
            // I->PrintIR(std::cout);
            if (liveInstructionset.find(I) == liveInstructionset.end()) {
                // I->PrintIR(std::cout);
                if (terminalI == I) {
                    // I->PrintIR(std::cout);
                    auto livebbid = PostDomTreeidom[id]->block_id;
                    while (liveBBset.find(livebbid) == liveBBset.end()) {
                        // I->PrintIR(std::cout);
                        livebbid = PostDomTreeidom[id]->block_id;
                    }
                    I = new BrUncondInstruction(GetNewLabelOperand(livebbid));
                    // I->PrintIR(std::cout);
                } else {
                    continue;
                }
            }
            bb->InsertInstruction(1, I);
            // bb->printIR(std::cout);

        }
        // std::cout<<"666"<<std::endl;

    }
    
    // std::cout<<"debug end!"<<std::endl;

    // 清理相关数据结构，释放内存（不过这里对于new创建的对象还缺少对应的delete操作，可能存在内存泄漏问题）
    defmap.clear();
    liveInstructionset.clear();
    liveBBset.clear(); 
}


// 驱动整个ADCE优化过程的函数，遍历一系列的控制流图并调用ADCE函数对每个图进行优化，同时统计耗时
void ADCEPass::Execute()
{
    for (auto [defI, cfg] : llvmIR->llvm_cfg) {
        // std::cout<<"execute pass"<<std::endl;
        auto end0 = std::chrono::high_resolution_clock::now();
        // llvmIR->printIR(std::cout);
        
        ADCE(cfg);
        // llvmIR->printIR(std::cout);
        // std::cout<<"execute pass end"<<std::endl;
        auto end1 = std::chrono::high_resolution_clock::now();
        // std::cout << "ADCE time: "
        //        << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - end0).count()
        //        << " ms" << std::endl;

    }
}