#include "simplify_cfg.h"

void SimplifyCFGPass::Execute() {
    for (auto [defI, cfg] : llvmIR->llvm_cfg) {
        //std::cout<<defI->GetFunctionName()<<std::endl;
        EliminateUnreachedBlocksInsts(cfg);
    }
}

// 删除从函数入口开始到达不了的基本块和指令
// 不需要考虑控制流为常量的情况，你只需要从函数入口基本块(0号基本块)开始dfs，将没有被访问到的基本块和指令删去即可


void SimplifyCFGPass::EliminateUnreachedBlocksInsts(CFG *C) { 
    //TODO("EliminateUnreachedBlocksInsts"); 
    if (!C->block_map) {
        //std::cout << "Error: block_map is null!" << std::endl;
        return;
    }

    std::set<int> visited; // 保存已访问的基本块 ID
    std::stack<LLVMBlock> dfs_stack;
   
    dfs_stack.push((*C->block_map->find(0)).second); // 假设入口基本块是编号 0 的块
    //std::cout<<"begin dfs:"<<std::endl;
    // 执行 DFS
    while (!dfs_stack.empty()) {
        LLVMBlock current_block = dfs_stack.top();
        dfs_stack.pop();
        //std::cout<<"dfsing bid: "<<current_block->block_id<<std::endl;
        // 如果该基本块已经访问过，跳过
        if (visited.count(current_block->block_id)) {
            //std::cout<<"bid: "<<current_block->block_id<<" has visited"<<std::endl;

            continue;
        }
        //std::cout<<"bid: "<<current_block->block_id<<" has not visited"<<std::endl;

        // 标记当前基本块为已访问
        visited.insert(current_block->block_id);

        // 遍历当前基本块的指令
        for (auto instr_it = current_block->Instruction_list.begin(); instr_it != current_block->Instruction_list.end();) {
            //(*instr_it)->PrintIR(std::cout);
            if ((*instr_it)->GetOpcode() == BasicInstruction::LLVMIROpcode::BR_UNCOND) {
                //std::cout<<"br_uncond"<<std::endl;

                // 如果是无条件跳转指令，添加跳转目标到栈
                int target_block_id = ((LabelOperand *)((BrUncondInstruction*)*instr_it)->GetDestLabel())->GetLabelNo();
                if (!visited.count(target_block_id)) {
                    dfs_stack.push((*C->block_map->find(target_block_id)).second);
                }

                // 删除当前跳转指令
                //instr_it = current_block->Instruction_list.erase(instr_it);
                //std::cout<<"erase ins: ";
                //(*instr_it)->PrintIR(std::cout);
                //std::cout<<std::endl;
                if(instr_it + 1!=current_block->Instruction_list.end())
                {
                    instr_it = current_block->Instruction_list.erase(instr_it + 1, current_block->Instruction_list.end());

                    //std::cout << "erase all subsequent instructions after br in current_block:"<< current_block->block_id << std::endl;

                    break; // 
                }
                else{
                    break;
                }

            }
            else if((*instr_it)->GetOpcode() == BasicInstruction::LLVMIROpcode::BR_COND){//有条件跳转，分为两个分支进行入栈
                //std::cout<<"br_cond"<<std::endl;

                int true_id = ((LabelOperand *)((BrCondInstruction*)*instr_it)->GetTrueLabel())->GetLabelNo(); // 获取跳转目标基本块
                int false_id = ((LabelOperand *)((BrCondInstruction*)*instr_it)->GetFalseLabel())->GetLabelNo(); // 获取跳转目标基本块 

                if (!visited.count(true_id)) {
                    dfs_stack.push((*C->block_map->find(true_id)).second);
                }
                if (!visited.count(false_id)) {
                    dfs_stack.push((*C->block_map->find(false_id)).second);
                }
                
                if(instr_it + 1!=current_block->Instruction_list.end())
                {
                    instr_it = current_block->Instruction_list.erase(instr_it + 1, current_block->Instruction_list.end());

                    //std::cout << "erase all subsequent instructions after br_cond in current_block:"<< current_block->block_id << std::endl;

                    break; // 
                }
                else{
                    break;
                }

            } 
            else if ((*instr_it)->GetOpcode() == BasicInstruction::LLVMIROpcode::RET) {
                // 如果是 RET 指令，删除该指令之后的所有指令
                instr_it = current_block->Instruction_list.erase(instr_it + 1, current_block->Instruction_list.end());
                //std::cout << "erase all subsequent instructions after RET in current_block:"<< current_block->block_id << std::endl;
                break; // 由于 RET 后面不再有指令了，跳出循环
            } else {
                // 如果是其他指令，继续遍历
                ++instr_it;
                //std::cout<<"++"<<std::endl;
            }
        }
       
        // // 遍历当前基本块的后继基本块，进行 DFS
        // for (const auto& successor : C->GetSuccessor(current_block->block_id)) {
        //     if (!visited.count(successor->block_id)) {
        //         dfs_stack.push(successor);
        //         std::cout<<"from "<<current_block->block_id<<" push: "<<successor->block_id<<std::endl;
        //     }
        // }
    }

    // 删除所有未访问的基本块
    for (auto it = C->block_map->begin(); it != C->block_map->end();) {
        if (visited.count(it->first) == 0) {
            //std::cout<<"erase block: "<< it->second->block_id <<std::endl;
            it = C->block_map->erase(it); // 删除未访问的基本块
            
        } 
        else {
            //std::cout<<"block : "<< it->second->block_id << " visited: " << visited.count(it->first) <<std::endl;
            ++it;
        }
        //std::cout<<"reached block_id: "<<it->second->block_id<<std::endl;

    }
}