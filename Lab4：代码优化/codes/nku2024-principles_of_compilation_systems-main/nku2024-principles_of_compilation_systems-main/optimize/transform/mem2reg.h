#ifndef MEM2REG_H
#define MEM2REG_H
#include "../../include/ir.h"
#include "../pass.h"

#include "../analysis/dominator_tree.h"

class Mem2RegPass : public IRPass {
private:
    DomAnalysis *domtrees;
    // TODO():添加更多你需要的成员变量
    bool IsPromotable(CFG *C, Instruction AllocaInst, std::map<int, std::set<int>> defs, std::map<int, std::set<int>> uses);
    void Mem2RegNoUseAlloca(CFG *C, std::set<int> &vset);
    void Mem2RegUseDefInSameBlock(CFG *C, std::set<int> &vset, int block_id);
    void Mem2RegOneDefDomAllUses(CFG *C, std::set<int> &vset);
    void InsertPhi(CFG *C);
    void VarRename(CFG *C);
    void Mem2Reg(CFG *C);
    void solve_alloca_ptr_not_use(CFG *C, std::map<int, std::set<int>> defs, std::map<int, std::set<int>> uses);
    void output_the_use_alloca(CFG *C);
    void solve_alloca_ptr_def_and_use_in_same_block(CFG *C, std::map<int, std::set<int>> defs, std::map<int, std::set<int>> uses);
    std::map<int,int> calculate_num_of_def(CFG *C);//计算一个CFG中define的次数



public:
    Mem2RegPass(LLVMIR *IR, DomAnalysis *dom) : IRPass(IR), domtrees(dom) {  }
    void Execute();
};

#endif