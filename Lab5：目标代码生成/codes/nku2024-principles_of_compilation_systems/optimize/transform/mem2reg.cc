#include "mem2reg.h"
#include "ir.h"
#include "dominator_tree.h"
#include <tuple>
#include <chrono>
static std::set<Instruction> EraseSet;//需要删除的语句的集合
static std::set<int> common_allocas;//需要留着的语句的集合
static std::map<int, int> mem2reg_map;//定义了新的寄存器与老的寄存器的映射关系
static std::map<PhiInstruction *, int> phi_map;//定义phi指令的映射表





/*
本文件实现了 mem2reg 的简单情况，
包括 alloca 的指针没有被 use，
alloca 的指针的 def 和 use 均在同一基本块中
两种基本情况


*/

//计算出定义和使用的次数
auto Calculate_the_Num_of_Define_and_Use(CFG *C){
    std::map<int, std::set<int>> defs, uses;
    for(auto [block_id,basic_block] : *C->block_map){
        for(auto Block_Instruction : basic_block->Instruction_list){
            if(Block_Instruction->GetOpcode() == BasicInstruction::STORE){
                auto Store_Instruction = (StoreInstruction *)Block_Instruction;
                if(Store_Instruction->GetPointer()->GetOperandType() == BasicOperand::GLOBAL){
                    continue;
                }
                defs[Store_Instruction->GetDefRegNo()].insert(block_id);
                ////std::cout<<"def["<<Store_Instruction->GetDefRegNo()<<"]"<<" insert: "<<block_id<<std::endl;
            }
            else if(Block_Instruction->GetOpcode() == BasicInstruction::LOAD){
                auto Load_Instruction = (LoadInstruction *)Block_Instruction;
                if(Load_Instruction->GetPointer()->GetOperandType() == BasicOperand::GLOBAL){
                    continue;
                }
                uses[Load_Instruction->GetUseRegNo()].insert(block_id);
                
                ////std::cout<<"use["<<Load_Instruction->GetUseRegNo()<<"]"<<" insert: "<<block_id<<std::endl;

            }
        }
    }
    return std::tuple<std::map<int, std::set<int>>, std::map<int, std::set<int>>>(defs, uses);
}

// 检查该条alloca指令是否可以被mem2reg
// eg. 数组不可以mem2reg
// eg. 如果该指针直接被使用不可以mem2reg(在SysY一般不可能发生,SysY不支持指针语法)
bool Mem2RegPass::IsPromotable(CFG *C, Instruction AllocaInst,std::map<int, std::set<int>> defs, std::map<int, std::set<int>> uses) {
    auto AllocaI = (AllocaInstruction *)AllocaInst;
    if (!AllocaI->GetDims().empty()) {
        return false; // 数组类型不能优化
    }

    int regno = ((RegOperand *)AllocaI)->GetRegNo();
    //auto [defs, uses] = Calculate_the_Num_of_Define_and_Use(C);

    // 如果没有任何 `use`，说明这个变量没有被使用，可以删除
    if (uses[regno].empty()) {
        return true;
    }

    if (defs[regno].size() == 1 && uses[regno].size() == 1) {
        int def_block = *defs[regno].begin();
        int use_block = *uses[regno].begin();
        if (def_block == use_block) {
            return true; // 可以优化
        }
    }

    return false;
}

/*
    int a1 = 5,a2 = 3,a3 = 11,b = 4
    return b // a1,a2,a3 is useless
-----------------------------------------------
pseudo IR is:
    %r0 = alloca i32 ;a1
    %r1 = alloca i32 ;a2
    %r2 = alloca i32 ;a3
    %r3 = alloca i32 ;b
    store 5 -> %r0 ;a1 = 5
    store 3 -> %r1 ;a2 = 3
    store 11 -> %r2 ;a3 = 11
    store 4 -> %r3 ;b = 4
    %r4 = load i32 %r3
    ret i32 %r4
--------------------------------------------------
%r0,%r1,%r2只有store, 但没有load,所以可以删去
优化后的IR(pseudo)为:
    %r3 = alloca i32
    store 4 -> %r3
    %r4 - load i32 %r3
    ret i32 %r4
*/



//标记那些在给定寄存器集合（vset）中的寄存器被存储到内存的指令，并将它们添加到 EraseSet 中，供后续处理
void Mem2RegPass::Mem2RegNoUseAlloca(CFG *C,std::set<int> &vset) {

    for (auto [block_id, basic_block] : *C->block_map) {
        for (auto Block_Instruction : basic_block->Instruction_list) {    
            // 遍历每一个基本块的指令集，Block_Instruction是对应的指令
            ////std::cout<<"BlockInstruction:";
            //Block_Instruction->PrintIR(std::cout);
            
            
            int Block_Instruction_Opcode = Block_Instruction->GetOpcode();    // 获取指令的opcode
            if (Block_Instruction_Opcode == BasicInstruction::STORE) {
                auto Store_Instruction = (StoreInstruction *)Block_Instruction;
                if (Store_Instruction->GetPointer()->GetOperandType() != BasicOperand::REG) {
                    continue;
                }
                //int v = ((RegOperand *)Store_Instruction)->GetRegNo();
                int v =Store_Instruction->GetDefRegNo();
                if (vset.find(v) == vset.end()) {
                    continue;
                }
                EraseSet.insert(Block_Instruction);

            }
        }
    }
}

//遍历控制流图（CFG）中的每个基本块，处理指令集并根据条件删除不需要的指令，最终将处理后的指令重新插入基本块的指令列表
void Mem2RegPass::output_the_use_alloca(CFG *C){
    ////std::cout<<"output call!"<<std::endl;
    for (auto [id, bb] : *C->block_map) {
        auto tmp_Instruction_list = bb->Instruction_list;
        bb->Instruction_list.clear();
        for (auto I : tmp_Instruction_list) {
            //I->PrintIR(std::cout);
            ////std::cout<<std::endl;
            if (EraseSet.find(I) != EraseSet.end()) {
                ////std::cout<<"Deleted"<<std::endl;
                //I->PrintIR(//std::cout);
                ////std::cout<<std::endl;

                continue;
            }
            bb->InsertInstruction(1, I);
        }
    }
    EraseSet.clear();
    common_allocas.clear();
    
}

// 基础要求2-1：alloca 的指针没有被use
// 在控制流图（CFG）中查找未被使用的 ALLOC 指令，并将它们标记为不必要的指令
void Mem2RegPass::solve_alloca_ptr_not_use(CFG *C, std::map<int, std::set<int>> defs, std::map<int, std::set<int>> uses){//找到只被store没有被load的，存到一个set里
    LLVMBlock entry_BB = (*C->block_map)[0];
    std::set<int> no_use_vset;
    //auto [defs, uses] = Calculate_the_Num_of_Define_and_Use(C);

    for (auto I : entry_BB->Instruction_list) {
        if (I->GetOpcode()!= BasicInstruction::LLVMIROpcode::ALLOCA) {
            continue;
        }
        auto AllocaI = (AllocaInstruction *)I;
        if (!(AllocaI->GetDims().empty())) {
            continue;
        }
        int v = AllocaI->GetResultRegNo();
        BasicInstruction::LLVMType type = AllocaI->GetDataType();

        if (uses[v].size() == 0) {    // 判断是否没有使用，即只被store
            EraseSet.insert(I);
            no_use_vset.insert(v);  // 将对应的寄存器编号记录到集合中
            continue;
        }

    }

    Mem2RegNoUseAlloca(C, no_use_vset);
    //output_the_use_alloca(C);
    ////std::cout<<"alloc but no use solved"<<std::endl;

}


//通过查找 STORE 和 LOAD 操作，将内存操作替换为寄存器操作，并在适当的情况下删除冗余的内存操作
void Mem2RegPass::Mem2RegUseDefInSameBlock(CFG *C, std::set<int> &vset, int block_id) {
    //std::cout<<"BID:"<<block_id<<" BEGIN"<<std::endl;
    //std::cout<<"vset"<<std::endl;
    for(auto v:vset)
    {
        //std::cout<<"Reg no: "<<v<<std::endl;
    }
    std::map<int, int> current_reg_map;//当前的寄存器使用情况
    for(auto Instruction : (*C->block_map)[block_id]->Instruction_list){
        //std::cout<<"scaning"<<std::endl;
        //Instruction->PrintIR(std::cout);
        //std::cout<<std::endl;

        int Instruction_Opcode = Instruction->GetOpcode();    // 获取指令的opcode

        if (Instruction_Opcode == BasicInstruction::STORE) {
            auto Store_Instruction = (StoreInstruction *)Instruction;
            if (Store_Instruction->GetPointer()->GetOperandType() != BasicOperand::REG) {
                continue;
            }
            int v = ((RegOperand *)Store_Instruction->GetPointer())->GetRegNo();
            //std::cout<<"Store"<<std::endl;
            //Instruction->PrintIR(std::cout);
            if (vset.find(v) == vset.end()) {
                //std::cout<<"Store didn't insert"<<std::endl;
                continue;
            }

            current_reg_map[v] = ((RegOperand *)(Store_Instruction->GetValue()))->GetRegNo();
            //std::cout<<"Store insert"<<std::endl;
            EraseSet.insert(Instruction);

        }

        if (Instruction_Opcode == BasicInstruction::LOAD) {
            auto Load_Instruction = (LoadInstruction *)Instruction;
            if (Load_Instruction->GetPointer()->GetOperandType() != BasicOperand::REG) {
                continue;
            }
            int v = ((RegOperand *)Load_Instruction->GetPointer())->GetRegNo();
            //std::cout<<"LOAD INSTRUCTION: "<<std::endl;
            //Instruction->PrintIR(std::cout);
            if (vset.find(v) == vset.end()) {
                //std::cout<<"Load didn't insert"<<std::endl;
                continue;
            }

            mem2reg_map[((RegOperand*)(Load_Instruction->GetResult()))->GetRegNo()] = current_reg_map[v];
            EraseSet.insert(Instruction);
            //std::cout<<"load insert"<<std::endl;

        }
    }

    //std::cout<<"BID:"<<block_id<<" END"<<std::endl;

}



// 基础要求2-2：alloca 的指针的def和use均在同一基本块中
// 本函数会对每个alloca的内容遍历，看看是不是只在一个块里用了，如果是则会进入Mem2RegUseDefInSameBlock处理
void Mem2RegPass::solve_alloca_ptr_def_and_use_in_same_block(CFG *C,std::map<int, std::set<int>> defs, std::map<int, std::set<int>> uses){
    LLVMBlock entry_BB = (*C->block_map)[0];
    //std::cout<<"find entry_BB"<<std::endl;
    std::map<int, std::set<int>> sameblock_vset_map;
    //auto [defs, uses] = Calculate_the_Num_of_Define_and_Use(C);
    
    for (auto I : entry_BB->Instruction_list) {
        // //std::cout<<"scaning"<<std::endl;
        // I->PrintIR(std::cout);
        // //std::cout<<std::endl;
        if (I->GetOpcode()!= BasicInstruction::LLVMIROpcode::ALLOCA) {
            
            continue;
        }
        auto AllocaI = (AllocaInstruction *)I;
        if (!(AllocaI->GetDims().empty())) {
            continue;
        }
        //std::cout<<"scaning alloc instruction"<<std::endl;
        //AllocaI->PrintIR(std::cout);
        //std::cout<<std::endl;
        ////std::cout<<"find entry_BB"<<std::endl;

        int v = AllocaI->GetResultRegNo();
        ////std::cout<<"v: "<<v<<std::endl;

        BasicInstruction::LLVMType type = AllocaI->GetDataType();

        
        auto alloca_defs = defs[v];
        auto alloca_uses = uses[v];
        if (alloca_defs.size() == 1) {
            int block_id = *(alloca_defs.begin());
            if (alloca_uses.size() == 1 && *(alloca_uses.begin()) == block_id) {    // def and use in the same block
                EraseSet.insert(I);
                //std::cout<<"erase use and def in same block ptr"<<std::endl;
                //I->PrintIR(std::cout);
                //std::cout<<std::endl;
                sameblock_vset_map[block_id].insert(v);
                continue;
            }
        }

    }

    //std::cout<<"sameblock_vset_map: Size: "<< sameblock_vset_map.size()<<std::endl;
    for (auto [bid, vset] : sameblock_vset_map) {
        
        ////std::cout<<std::endl;
        //std::cout<<"including bid: "<<bid<<std::endl;

    }
    for (auto [bid, vset] : sameblock_vset_map) {
        
        ////std::cout<<std::endl;
        //std::cout<<"searching bid: "<<bid<<std::endl;

        Mem2RegUseDefInSameBlock(C, vset, bid);
    }
    //std::cout<<"Finished once "<<std::endl;

    

}


std::map<int,int> Mem2RegPass::calculate_num_of_def(CFG *C){//计算一个CFG中定义的次数
    std::map<int, int> define_num;
    for(auto [block_id, basic_block] : *C->block_map){
        for(auto instruction : basic_block->Instruction_list){
            if(instruction->GetOpcode() == BasicInstruction::STORE){
                auto Store_Instruction = (StoreInstruction *)instruction;
                if (Store_Instruction->GetPointer()->GetOperandType() == BasicOperand::GLOBAL) {
                    continue;
                }
                //对每一个store指令的寄存器编号的定义数量都加一
                define_num[Store_Instruction->GetDefRegNo()]++;  
            }
        }
    }
    return define_num;
}

//在定义块（STORE 操作）中只被使用一次的寄存器,优化内存操作为寄存器操作
void Mem2RegPass::Mem2RegOneDefDomAllUses(CFG *C, std::set<int> &vset) {
    std::map<int, int> v_result_map;
    for (auto [id, bb] : *C->block_map) {
        for (auto I : bb->Instruction_list) {
            if (I->GetOpcode() == BasicInstruction::STORE) {
                auto StoreI = (StoreInstruction *)I;
                if (StoreI->GetPointer()->GetOperandType() != BasicOperand::REG) {
                    continue;
                }
                int v = StoreI->GetDefRegNo();
                if (vset.find(v) == vset.end()) {
                    continue;
                }
                v_result_map[v] = ((RegOperand *)(StoreI->GetValue()))->GetRegNo();
                EraseSet.insert(I);
            }
        }
    }
    for (auto [id, bb] : *C->block_map) {
        for (auto I : bb->Instruction_list) {
            if (I->GetOpcode() == BasicInstruction::LOAD) {
                auto LoadI = (LoadInstruction *)I;
                if (LoadI->GetPointer()->GetOperandType() != BasicOperand::REG) {
                    continue;
                }
                int v = LoadI->GetUseRegNo();
                if (vset.find(v) == vset.end()) {
                    continue;
                }
                mem2reg_map[LoadI->GetResultRegNo()] = v_result_map[v];
                EraseSet.insert(I);
            }
        }
    }
}

//插入phi指令
//Reference: https://github.com/yuhuifishash/SysY
void Mem2RegPass::InsertPhi(CFG *C) {
    std::map<int,int> define_num = calculate_num_of_def(C);//得到对应的寄存器定义的次数
    auto [defs, uses] = Calculate_the_Num_of_Define_and_Use(C);
    //std::cout<<"calcu"<<std::endl;
    LLVMBlock entry_block = (*C->block_map)[0];//入口地址

    std::set<int> onedom_vset;//存储单一定义且支配所有使用的寄存器集合
    //std::cout<<"before for"<<std::endl;

    DominatorTree* dom_tree = domtrees->GetDomTree(C);

    DominatorTree* dom_tree_reverse = domtrees->GetPostDomTree(C);
    //dom_tree->BuildDominatorTree();

    for(auto instruction : entry_block->Instruction_list){
        //std::cout<<"in for"<<std::endl;

        if (instruction->GetOpcode() != BasicInstruction::ALLOCA) {
            continue;
        }
        auto AllocaI = (AllocaInstruction *)instruction;
        if (!(AllocaI->GetDims().empty())) {
            continue;
        }

        int v = AllocaI->GetResultRegNo();
        BasicInstruction::LLVMType type = AllocaI->GetDataType();
        //std::cout<<"after gettype "<<std::endl;

        auto alloca_defs = defs[v];
        auto alloca_uses = uses[v];
        //std::cout<<"before ==1 "<<std::endl;
        //std::cout<<"define_num["<<v<<"]: "<< define_num[v]<<std::endl;

        if (define_num[v] == 1) {    // only def once
            //std::cout<<"before bid "<<std::endl;

            int block_id = *(alloca_defs.begin());
            int dom_flag = 1;

            //DominatorTree* dom_tree = domtrees->GetDomTree(C);
            if(dom_tree->atdom.size()==0)
            {
                auto end1 = std::chrono::high_resolution_clock::now();

                dom_tree->BuildDominatorTree();
                auto end2 = std::chrono::high_resolution_clock::now();
                //std::cout << "single dom time: "<< std::chrono::duration_cast<std::chrono::milliseconds>(end2 - end1).count()<< " ms" << std::endl;
                dom_tree_reverse->BuildPostDominatorTree();

            }
            for (auto load_BBid : alloca_uses) {
                //std::cout<<"before "<<std::endl;
                // std::cout<<"domtrees DomInfo"<<domtrees->DomInfo.size()<<std::endl;

                // DominatorTree* dom_tree = domtrees->GetDomTree(C);
                
                // dom_tree->BuildDominatorTree();

                // std::cout<<"after getDomTree"<<std::endl;
                // std::cout<<"dom_tree atdom size: "<<dom_tree->atdom.size()<<std::endl;
                
                if (dom_tree->IsDominate(block_id, load_BBid) == false) {
                    dom_flag = 0;
                    break;
                }
                // std::cout<<"after isDominate"<<std::endl;


            }
            //std::cout<<"after scaning bbid"<<std::endl;

            if (dom_flag) {    // one def dominate all uses
                
                EraseSet.insert(instruction);
                onedom_vset.insert(v);
                continue;
            }
        }
        // std::cout<<"after ==1 "<<std::endl;

        common_allocas.insert(v);
        //std::cout<<"after ca insert "<<std::endl;

        EraseSet.insert(instruction);
        //std::cout<<"after es insert "<<std::endl;

        std::set<int> F{};
        std::set<int> W = defs[v];

        while (!W.empty()) {
            int BB_1 = *W.begin();
            //std::cout<<"in while"<<std::endl;
            W.erase(BB_1);
            //std::cout<<"after erase"<<std::endl;

            //std::cout<<"domtrees DomInfo: "<<domtrees->DomInfo.size()<<std::endl;
            
            //DominatorTree* dom_tree = domtrees->GetDomTree(C);
            if(dom_tree->atdom.size()==0)
            {
                dom_tree->BuildDominatorTree();
                dom_tree_reverse->BuildPostDominatorTree();

            }
            //std::cout<<dom_tree->C->max_label;
            //std::cout<<"getDomTree"<<std::endl;
            assert(dom_tree!=NULL);

            
            std::set<int> BB_1_DFSet=dom_tree->GetDF(BB_1);
            //std::cout<<"getDF"<<std::endl;

            for (auto BB_2 : BB_1_DFSet) {
                    //std::cout<<"in for"<<std::endl;

                // //std::cout<<v<<" "<<BB_1<<" "<<BB_2<<"\n";
                if (F.find(BB_2) == F.end()) {
                    PhiInstruction *PhiI = new PhiInstruction(type, GetNewRegOperand(++C->max_reg));
                    (*C->block_map)[BB_2]->InsertInstruction(0, PhiI);
                    phi_map[PhiI] = v;
                    F.insert(BB_2);
                    if (defs[v].find(BB_2) == defs[v].end()) {
                        W.insert(BB_2);//为了确认有没有define
                    }
                }
            }
        }
    }
    //std::cout<<"after for"<<std::endl;

    Mem2RegOneDefDomAllUses(C, onedom_vset);//做只使用一次而且被全部支配的寄存器的mem2reg

}







//Reference: https://github.com/yuhuifishash/SysY
int in_allocas(std::set<int> &S, Instruction I) {
    if (I->GetOpcode() == BasicInstruction::LOAD) {
        auto LoadI = (LoadInstruction *)I;
        if (LoadI->GetPointer()->GetOperandType() != BasicOperand::REG) {
            return -1;
        }
        int pointer = LoadI->GetUseRegNo();
        if (S.find(pointer) != S.end()) {
            return pointer;
        }
    }
    if (I->GetOpcode() == BasicInstruction::STORE) {
        auto StoreI = (StoreInstruction *)I;
        if (StoreI->GetPointer()->GetOperandType() != BasicOperand::REG) {
            return -1;
        }
        int pointer = StoreI->GetDefRegNo();
        if (S.find(pointer) != S.end()) {
            return pointer;
        }
    }
    return -1;
}


//Reference: https://github.com/yuhuifishash/SysY
void Mem2RegPass::VarRename(CFG *C) {

    std::map<int, std::map<int, int>> WorkList;
    WorkList.insert({0, std::map<int, int>{}});
    std::vector<int> BBvis;
    BBvis.resize(C->max_label + 1);
    while (!WorkList.empty()) {
        int BB = (*WorkList.begin()).first;
        auto IncomingVals = (*WorkList.begin()).second;
        WorkList.erase(BB);
        if (BBvis[BB]) {
            continue;
        }
        BBvis[BB] = 1;
        for (auto &I : (*C->block_map)[BB]->Instruction_list) {
            if (I->GetOpcode() == BasicInstruction::LOAD) {
                auto LoadI = (LoadInstruction *)I;
                int v = in_allocas(common_allocas, I);
                if (v >= 0) {    // load instruction is in common_allocas
                    // 如果当前指令是 load，找到对应的 alloca v，将用到 load 结果的地方都替换成
                    // IncomingVals[v]
                    EraseSet.insert(LoadI);
                    mem2reg_map[LoadI->GetResultRegNo()] = IncomingVals[v];
                }
            }
            if (I->GetOpcode() == BasicInstruction::STORE) {
                auto StoreI = (StoreInstruction *)I;
                int v = in_allocas(common_allocas, I);
                if (v >= 0) {    // store instruction is in common_allocas
                    // 如果当前指令是 store，找到对应的 alloca v，更新IncomingVals[v] = val,并删除store
                    EraseSet.insert(StoreI);
                    IncomingVals[v] = ((RegOperand *)(StoreI->GetValue()))->GetRegNo();
                }
            }
            if (I->GetOpcode() == BasicInstruction::PHI) {
                auto PhiI = (PhiInstruction *)I;
                if (EraseSet.find(PhiI) != EraseSet.end()) {
                    continue;
                }
                auto it = phi_map.find(PhiI);
                if (it != phi_map.end()) {    // phi instruction is in allocas
                    // 更新IncomingVals[v] = val
                    IncomingVals[it->second] = PhiI->GetResultRegNo();
                }
            }
        }
        for (auto succ : C->G[BB]) {
            int BBv = succ->block_id;
            WorkList.insert({BBv, IncomingVals});
            for (auto I : (*C->block_map)[BBv]->Instruction_list) {
                if (I->GetOpcode() != BasicInstruction::PHI) {
                    break;
                }
                auto PhiI = (PhiInstruction *)I;
                // 找到 phi 对应的 alloca
                auto it = phi_map.find(PhiI);
                if (it != phi_map.end()) {
                    int v = it->second;
                    if (IncomingVals.find(v) == IncomingVals.end()) {
                        EraseSet.insert(PhiI);
                        continue;
                    }
                    // 为 phi 添加前驱块到当前块的边
                    PhiI->InsertPhi(GetNewRegOperand(IncomingVals[v]), GetNewLabelOperand(BB));
                }
            }
        }
    }

    auto total_start = std::chrono::high_resolution_clock::now();

    for (auto [id, bb] : *C->block_map) {
        for (auto I : bb->Instruction_list) {
            if (I->GetOpcode() == BasicInstruction::LOAD && 
                ((LoadInstruction *)I)->GetPointer()->GetOperandType() == BasicOperand::REG) 
            {
                auto LoadInstru = (LoadInstruction* )I;
                int result = ((RegOperand *)LoadInstru->GetResult())->GetRegNo();
                if (mem2reg_map.find(result) != mem2reg_map.end()) {
                    int result2 = mem2reg_map[result];
                    while (mem2reg_map.find(result2) != mem2reg_map.end()) {
                        mem2reg_map[result] = mem2reg_map[result2];
                        result2 = mem2reg_map[result];
                    }
                }
            }
        }
    }
    

    auto end0 = std::chrono::high_resolution_clock::now();

    // std::cout << "LOAD REPLACE time: "
    //           << std::chrono::duration_cast<std::chrono::milliseconds>(end0 - total_start).count()
    //           << " ms" << std::endl;

    for (auto [id, bb] : *C->block_map) {
        auto tmp_Ins_list = bb->Instruction_list;
        bb->Instruction_list.clear();
        for (auto I : tmp_Ins_list) {
            if (EraseSet.find(I) != EraseSet.end()) {
                continue;
            }
            bb->InsertInstruction(1, I);
            I->ReplaceReg(mem2reg_map);
        }
    }
    

    auto end1 = std::chrono::high_resolution_clock::now();

    // std::cout << "replace all reg time: "
    //           << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - end0).count()
    //           << " ms" << std::endl;

    output_the_use_alloca(C);
    EraseSet.clear();
    mem2reg_map.clear();
    common_allocas.clear();
    phi_map.clear();
    
}


void Mem2RegPass::Mem2Reg(CFG *C) {
    auto total_start = std::chrono::high_resolution_clock::now();

    auto [defs, uses] = Calculate_the_Num_of_Define_and_Use(C);
    auto end0 = std::chrono::high_resolution_clock::now();

    // std::cout << "Calculate_the_Num_of_Define_and_Use execution time: "
    //           << std::chrono::duration_cast<std::chrono::milliseconds>(end0 - total_start).count()
    //           << " ms" << std::endl;


    solve_alloca_ptr_not_use(C,defs,uses);//第一步，将那些没有使用的寄存器给删除
    auto end1 = std::chrono::high_resolution_clock::now();
    // std::cout << "solve_alloca_ptr_not_use execution time: "
    //           << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - end0).count()
    //           << " ms" << std::endl;
    

    solve_alloca_ptr_def_and_use_in_same_block(C,defs,uses);//第二步，将那些在同一个块中定义和使用的寄存器删除
    auto end2 = std::chrono::high_resolution_clock::now();
    // std::cout << "solve_alloca_ptr_def_and_use_in_same_block execution time: "
    //           << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - end1).count()
    //           << " ms" << std::endl;

    InsertPhi(C);  //第三步，完成phi指令的插入
    // std::cout<<"insert phi finished"<<std::endl;
    VarRename(C);//第四步，完成变量重命名的任务
    // std::cout<<"varname finished"<<std::endl;
    
    auto end3 = std::chrono::high_resolution_clock::now();
    // std::cout << "VarRename execution time: "
    //           << std::chrono::duration_cast<std::chrono::milliseconds>(end3 - end2).count()
    //           << " ms" << std::endl;
    auto total_end = std::chrono::high_resolution_clock::now();
    // std::cout << "Total Mem2Reg function execution time: "
    //           << std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start).count()
    //           << " ms" << std::endl;
}

void Mem2RegPass::Execute() {
    
    for (auto [defI, cfg] : llvmIR->llvm_cfg) {
        Mem2Reg(cfg);
        //std::cout<<"finish mem2reg!"<<std::endl;
    }
}