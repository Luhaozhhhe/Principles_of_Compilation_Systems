
#include "dce.h"
//Reference: https://github.com/yuhuifishash/SysY

extern std::map<std::string, CFG *> CFGMap;
extern AliasAnalyser *alias_analyser;


void DCEPass::Simple_DCE(CFG* C){
    std::map<int, Instruction> ResultMap;
    std::set<int> WorkList;
    std::map<int, int> RegUseCount;
    std::set<Instruction> EraseSet;

    for (auto [id, bb] : *C->block_map) {
        for (auto I : bb->Instruction_list) {
            int v = I->GetResultRegNo();
            if (v != -1) {
                WorkList.insert(v);
                ResultMap[v] = I;
            }
            for (auto op : I->GetNonResultOperands()) {
                if (op->GetOperandType() == BasicOperand::REG) {
                    RegUseCount[((RegOperand *)op)->GetRegNo()]++;
                }
            }
        }
    }
    
    // std::cout<<"1"<<std::endl;

    while (!WorkList.empty()) {
        // std::cout<<"2"<<std::endl;

        int v = *WorkList.begin();
        WorkList.erase(v);
        if (RegUseCount[v] == 0) {
            // std::cout<<"3"<<std::endl;

            auto resultI = ResultMap[v];
            if (resultI == nullptr) {
                continue;
            } 
            if (resultI->GetOpcode() == BasicInstruction::CALL) {//检查有没有副作用，比如外部调用 或者写指针
                // std::cout<<"4"<<std::endl;
                                                   
                auto CallI = (CallInstruction *)resultI;
                //std::cout<<"5"<<std::endl;

                //std::cout<<"CallI func name: "<<CallI->GetFunctionName()<<std::endl;
                //std::cout<<"CFGMap find: "<<(CFGMap.find(CallI->GetFunctionName()) == CFGMap.end())<<std::endl;


                if (CFGMap.find(CallI->GetFunctionName()) == CFGMap.end()) {

                    // std::cout<<"6"<<std::endl;

                    continue;
                }

                // std::cout<<"lhz"<<std::endl;

                auto target_cfg = CFGMap[CallI->GetFunctionName()];
                // std::cout<<"10"<<std::endl;
                if(!alias_analyser)
                {
                    // std::cout<<"alias is null"<<std::endl;
                    
                }
                // std::cout<<"alias cfg"<<!alias_analyser->CFG_isNoSideEffect(target_cfg)<<std::endl;
                //

                if (!alias_analyser->CFG_isNoSideEffect(target_cfg)) {
                    // std::cout<<"7"<<std::endl;

                    continue;
                }
                // std::cout<<"8"<<std::endl;

            }

            EraseSet.insert(resultI);
            for (auto op : resultI->GetNonResultOperands()) {
                if (op->GetOperandType() == BasicOperand::REG) {
                    int x = ((RegOperand *)op)->GetRegNo();
                    RegUseCount[x]--;//非结果的操作数，计数--
                    WorkList.insert(x);
                }
            }
        }
    }

    // std::cout<<"2"<<std::endl;

    for (auto [id, bb] : *C->block_map) {
        // std::cout<<"3"<<std::endl;

        auto tmp_Instruction_list = bb->Instruction_list;
        bb->Instruction_list.clear();
        for (auto I : tmp_Instruction_list) {
            if (EraseSet.find(I) != EraseSet.end()) {
                continue;
            }
            bb->InsertInstruction(1, I);
        }
    }
}

void DCEPass::Execute()
{
    SimpleAliasAnalysis(llvmIR);
    for (auto [defI, cfg] : llvmIR->llvm_cfg) {
        // std::cout<<"begin execute dce pass"<<std::endl;
        Simple_DCE(cfg);
    }
    
}