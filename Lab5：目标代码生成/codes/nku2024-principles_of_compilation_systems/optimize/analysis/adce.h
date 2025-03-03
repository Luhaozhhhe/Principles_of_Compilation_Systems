#ifndef ADCE_H
#define ADCE_H
#include "../../include/ir.h"
#include "../pass.h"
#include "../../include/cfg.h"
#include "./alias_analysis.h"
#include "dominator_tree.h"

class ADCEPass : public IRPass {
private:
    DomAnalysis *domtrees;

public:
    ADCEPass(LLVMIR *IR) : IRPass(IR) {}
    void ADCE(CFG* cfg);
    void Execute();
    std::vector<std::vector<LLVMBlock>> BuildCDG(CFG *C);
};

#endif