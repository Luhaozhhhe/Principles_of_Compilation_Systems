#ifndef DCE_H
#define DCE_H
#include "../../include/ir.h"
#include "../pass.h"
#include "../../include/cfg.h"
#include "./alias_analysis.h"
class DCEPass : public IRPass {
private:
    // TODO():添加更多你需要的成员变量

public:
    DCEPass(LLVMIR *IR) : IRPass(IR) {}
    void Simple_DCE(CFG* cfg);
    void Execute();
    void SimpleAliasAnalysis(LLVMIR *IR);
};

#endif