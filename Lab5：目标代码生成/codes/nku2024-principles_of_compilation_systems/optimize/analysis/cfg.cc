#include "../../include/Instruction.h"
#include "../../include/ir.h"
#include "../transform/simplify_cfg.h"
#include "../../include/cfg.h"


#include <assert.h>

extern std::map<FuncDefInstruction, int> max_label_map;
extern std::map<FuncDefInstruction, int> max_reg_map;
std::map<std::string, CFG *> CFGMap;



void LLVMIR::CFGInit() {
    for (auto &[defI, bb_map] : function_block_map) {
        CFG *cfg = new CFG();
        cfg->block_map = &bb_map;
        cfg->function_def = defI;

        cfg->max_reg = max_reg_map[defI];
        cfg->max_label = max_label_map[defI];

        CFGMap[defI->GetFunctionName()] = cfg;
        //TODO("init your members in class CFG if you need");
        llvm_cfg[defI] = cfg;
    }
}

void LLVMIR::BuildCFG() {

    for (auto [defI, cfg] : llvm_cfg) {
        // std::cout<<defI->GetFunctionName()<<std::endl;
        cfg->BuildCFG();
    }
}

void CFG::BuildCFG() { 
    //TODO("BuildCFG"); 
    /*
    FuncDefInstruction function_def;
    std::map<int, LLVMBlock> *block_map;
    // 使用邻接表存图
    std::vector<std::vector<LLVMBlock>> G{};       // control flow graph
    std::vector<std::vector<LLVMBlock>> invG{}; 
    */
   if (!block_map || block_map->empty()) {
        std::cerr << "Error: block_map is null or empty!" << std::endl;
        return;
    }

    // 初始化控制流图和逆控制流图
    int block_count = block_map->size();
    // std::cout <<"block_size: "<< block_count << std::endl;
    // for(auto [bid,BB]: *block_map)
    // {
    //     std::cout<<"bid: "<<bid<<std::endl;
    // }
    G.clear();
    invG.clear();
    G.resize(max_label+1);
    invG.resize(max_label+1);
    
    // 遍历每个基本块并根据跳转指令建立图
    for (auto [id, Bid] : *block_map) {
        //std::cout << "current id: " << id  << ", current bid: " << Bid << std::endl;
        if (Bid->Instruction_list.empty()) continue;

        // 获取当前块的最后一条指令
        auto last_instr = Bid->Instruction_list.back();
        // std::cout << "Last instruction: " << std::endl;
        // last_instr->PrintIR(std::cout);
        // std::cout<<"print"<<std::endl;
        
        // 根据最后一条指令确定后继块
        if (last_instr->GetOpcode() == BasicInstruction::LLVMIROpcode::BR_UNCOND) {
            //std::cout<<"br uncond"<<std::endl;

            int target_id = ((LabelOperand *)((BrUncondInstruction*)last_instr)->GetDestLabel())->GetLabelNo(); // 获取跳转目标基本块
            G[id].push_back((*block_map)[target_id]);
            invG[target_id].push_back(Bid);

        } 
        else if (last_instr->GetOpcode() == BasicInstruction::LLVMIROpcode::BR_COND) {
            //std::cout<<"brcond"<<std::endl;
            //std::cout<<"1"<<std::endl;
            //std::cout<<"2"<<std::endl;
            //std::cout<<"3"<<std::endl;
            


            int true_id = ((LabelOperand *)((BrCondInstruction*)last_instr)->GetTrueLabel())->GetLabelNo(); // 获取跳转目标基本块

            int false_id = ((LabelOperand *)((BrCondInstruction*)last_instr)->GetFalseLabel())->GetLabelNo(); // 获取跳转目标基本块    
            //std::cout<<"4"<<std::endl;
            G[id].push_back((*block_map)[true_id]);
            G[id].push_back((*block_map)[false_id]);
            //std::cout<<"5"<<std::endl;
            //std::cout<<"invG size:"<<invG.size()<<std::endl;
            //std::cout<<"true id:"<<true_id<<std::endl;
            
            invG[true_id].push_back(Bid);
            invG[false_id].push_back(Bid);
            //std::cout<<"6"<<std::endl;

        } else if (last_instr->GetOpcode() == BasicInstruction::LLVMIROpcode::RET) {
            // Return 指令没有后继块
            ret_block = Bid;
            //std::cout<<"ret has no G"<<std::endl;
        }
        else{
            // std::cout<<"BULID CFG Wrong"<<std::endl;
        }
        //std::cout<<"this block finished"<<std::endl;

    }

    //std::cout<<"built"<<std::endl;
}

std::vector<LLVMBlock> CFG::GetPredecessor(LLVMBlock B) { return invG[B->block_id]; }

std::vector<LLVMBlock> CFG::GetPredecessor(int bbid) { return invG[bbid]; }

std::vector<LLVMBlock> CFG::GetSuccessor(LLVMBlock B) { return G[B->block_id]; }

std::vector<LLVMBlock> CFG::GetSuccessor(int bbid) { return G[bbid]; }