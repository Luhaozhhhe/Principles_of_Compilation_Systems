#ifndef ALIAS_ANALYSIS_H
#define ALIAS_ANALYSIS_H
#include "../../include/ir.h"
#include <assert.h>
//Reference: https://github.com/yuhuifishash/SysY
class AliasAnalyser {
protected:
    LLVMIR *IR;

public:
    enum AliasResult {
        NoAlias = 1,
        MayAlias = 2,
        MustAlias = 3,
    };

    enum ModRefResult {
        NoModRef = 0,
        Ref = 1,
        Mod = 2,
        ModRef = 3,
    };

    void SetLLVMIR(LLVMIR *ir) { this->IR = ir; }

    virtual void AliasAnalysis() = 0;

    virtual AliasResult QueryAlias(Operand op1, Operand op2, CFG *C) = 0;

    virtual ModRefResult QueryInstModRef(Instruction I, Operand op, CFG *C) = 0;

    virtual bool CFG_isReadMem(CFG *C) = 0;
    virtual bool CFG_isWriteMem(CFG *C) = 0;
    virtual bool CFG_isIndependent(CFG *C) = 0;
    virtual bool CFG_isNoSideEffect(CFG *C) = 0;
    virtual bool CFG_haveExternalCall(CFG *C) = 0;
    virtual bool is_localptrs(CFG *C, Operand ptr) = 0;
    virtual std::vector<Operand> GetWritePtrs(CFG *C) = 0;
    virtual std::vector<Operand> GetReadPtrs(CFG *C) = 0;

    virtual void PrintAAResult(bool is_printptr) = 0;
    virtual void AAtest() = 0;
};

class PtrRegMemInfo {
public:
    bool is_fullmem = false;

    bool is_local = true;

    std::vector<Operand> PossiblePtrs;

    bool InsertNewPtrs(Operand op, std::map<int, PtrRegMemInfo> &ptrmap, CFG *C);
    bool PushPossiblePtr(Operand op);

    void PrintDebugInfo();
};

class FunctionMemRWInfo {
public:
    bool have_external_call = false;

    std::vector<Operand> ReadPtrs;
    std::vector<Operand> WritePtrs;

    bool InsertNewReadPtrs(Operand op);
    bool InsertNewWritePtrs(Operand op);
    bool InsertNewReadPtrs(std::vector<Operand> ops);
    bool InsertNewWritePtrs(std::vector<Operand> ops);
    bool MergeCall(CallInstruction *CallI, FunctionMemRWInfo rwinfo, std::map<int, PtrRegMemInfo> &ptrmap);

    bool isIndependent() { return (!have_external_call) && ReadPtrs.size() == 0 && WritePtrs.size() == 0; }
    bool isNoSideEffect() { return (!have_external_call) && WritePtrs.size() == 0; }
    bool isReadMem() { return ReadPtrs.size() != 0 || have_external_call; }
    bool isWriteMem() { return WritePtrs.size() != 0 || have_external_call; }
};

class SimpleAliasAnalyser : public AliasAnalyser {
private:
    std::map<CFG *, FunctionMemRWInfo> CFGMemRWMap;
    std::map<CFG *, std::map<int, PtrRegMemInfo>> PtrRegMemMap;
    std::map<CFG *, std::map<int, Instruction>> CFGResultMap;
    void AliasAnalysis(CFG *C);
    bool IsSamePtrWithDiffConstIndex(Operand p1, Operand p2, CFG *C);

public:
    virtual void AliasAnalysis();

    virtual AliasResult QueryAlias(Operand op1, Operand op2, CFG *C);

    virtual ModRefResult QueryInstModRef(Instruction I, Operand op, CFG *C);

    virtual bool CFG_isReadMem(CFG *C) { return CFGMemRWMap[C].isReadMem(); }
    virtual bool CFG_isWriteMem(CFG *C) { return CFGMemRWMap[C].isWriteMem(); }
    virtual bool CFG_isIndependent(CFG *C) { return CFGMemRWMap[C].isIndependent(); }
    virtual bool CFG_isNoSideEffect(CFG *C) { 
        // std::cout<<"cfg_isnosideeffect called"<<std::endl;
        
        return CFGMemRWMap[C].isNoSideEffect(); }
    virtual bool CFG_haveExternalCall(CFG *C) { return CFGMemRWMap[C].have_external_call; }
    virtual bool is_localptrs(CFG *C, Operand ptr) {
        if (ptr->GetOperandType() == BasicOperand::REG) {
            auto regno = ((RegOperand *)ptr)->GetRegNo();
            return PtrRegMemMap[C][regno].is_local;
        }
        return false;
    }
    virtual std::vector<Operand> GetWritePtrs(CFG *C) { return CFGMemRWMap[C].WritePtrs; }
    virtual std::vector<Operand> GetReadPtrs(CFG *C) { return CFGMemRWMap[C].ReadPtrs; }

    virtual void PrintAAResult(bool is_printptr);
    virtual void AAtest();
};

#endif