#ifndef DOMINATOR_TREE_H
#define DOMINATOR_TREE_H
#include "../../include/ir.h"
#include "../pass.h"
#include <set>
#include "bitset.h"
#include <vector>


class DominatorTree {
public:
    CFG *C;
    std::vector<std::vector<LLVMBlock>> dom_tree{};
    std::vector<LLVMBlock> idom{};

    std::vector<DynamicBitset> df;
    std::vector<DynamicBitset> atdom;

    void BuildDominatorTree();
    void BuildPostDominatorTree();
    std::set<int> GetDF(std::set<int> S);             // return DF(S)  S = {id1,id2,id3,...}
    std::set<int> GetDF(int id);                      // return DF(id)
    bool IsDominate(int id1, int id2);                // if blockid1 dominate blockid2, return true, else return false

    // TODO(): add or modify functions and members if you need
};

class DomAnalysis : public IRPass {
public:
    std::map<CFG *, DominatorTree> DomInfo;
    std::map<CFG *, DominatorTree> DomInfo_reverse;


    DomAnalysis(LLVMIR *IR) : IRPass(IR) {}
    void Execute();
    DominatorTree *GetDomTree(CFG *C) { return &DomInfo[C]; }
    DominatorTree *GetPostDomTree(CFG *C) { return &DomInfo_reverse[C]; }
    bool hasDomTree(CFG* C){
        if(DomInfo.find(C)!=DomInfo.end())
        {
            return true;
        }
        return false;
    }
    bool hasPostDomTree(CFG* C){
        if(DomInfo_reverse.find(C)!=DomInfo_reverse.end())
        {
            return true;
        }
        return false;
    }

    // TODO(): add more functions and members if you need
};

//DomAnalysis analysis;

#endif