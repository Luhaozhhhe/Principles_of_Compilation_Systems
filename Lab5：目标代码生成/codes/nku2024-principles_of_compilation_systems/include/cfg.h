#ifndef CFG_H
#define CFG_H

#include "SysY_tree.h"
#include "basic_block.h"
#include <bitset>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <vector>

class CFG {
public:
    int max_label = 0;
    int max_reg = 0; 
    FuncDefInstruction function_def;
    LLVMBlock ret_block;


    /*this is the pointer to the value of LLVMIR.function_block_map
      you can see it in the LLVMIR::CFGInit()*/
    std::map<int, LLVMBlock> *block_map;
    //是一个指向 std::map<int, LLVMBlock> 类型的指针，很可能用于将某个整数标识符（可能是基本块的编号之类的）映射到对应的 LLVMBlock 类型对象

    // 使用邻接表存图
    std::vector<std::vector<LLVMBlock>> G{};       // control flow graph
    //用于存储控制流图（Control Flow Graph，CFG）的邻接表结构。
    //外层向量的每个元素对应一个基本块节点，而其内层向量存储的是该基本块的后继基本块列表，通过这种方式来表示控制流图中基本块之间的控制流向关系
    std::vector<std::vector<LLVMBlock>> invG{};    // inverse control flow graph
    //用于存储逆控制流图（inverse control flow graph）的邻接表结构。

    void BuildCFG();

    // 获取某个基本块节点的前驱/后继
    std::vector<LLVMBlock> GetPredecessor(LLVMBlock B);//获取基本块的前驱
    std::vector<LLVMBlock> GetPredecessor(int bbid);//获取一个id的前驱
    std::vector<LLVMBlock> GetSuccessor(LLVMBlock B);//获取基本块的后继
    std::vector<LLVMBlock> GetSuccessor(int bbid);//获取一个id的后继
};

#endif