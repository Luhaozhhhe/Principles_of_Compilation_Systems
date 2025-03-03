#include "../include/Instruction.h"
#include "../include/basic_block.h"
#include <assert.h>
#include <unordered_map>

static std::unordered_map<int, RegOperand *> RegOperandMap;
static std::map<int, LabelOperand *> LabelOperandMap;
static std::map<std::string, GlobalOperand *> GlobalOperandMap;

RegOperand *GetNewRegOperand(int RegNo) {
    auto it = RegOperandMap.find(RegNo);
    if (it == RegOperandMap.end()) {
        auto R = new RegOperand(RegNo);
        RegOperandMap[RegNo] = R;
        return R;
    } else {
        return it->second;
    }
}

LabelOperand *GetNewLabelOperand(int LabelNo) {
    auto it = LabelOperandMap.find(LabelNo);
    if (it == LabelOperandMap.end()) {
        auto L = new LabelOperand(LabelNo);
        LabelOperandMap[LabelNo] = L;
        return L;
    } else {
        return it->second;
    }
}

GlobalOperand *GetNewGlobalOperand(std::string name) {
    auto it = GlobalOperandMap.find(name);
    if (it == GlobalOperandMap.end()) {
        auto G = new GlobalOperand(name);
        GlobalOperandMap[name] = G;
        return G;
    } else {
        return it->second;
    }
}

void IRgenArithmeticI32(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int reg1, int reg2, int result_reg) {
    B->InsertInstruction(1, new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::I32, GetNewRegOperand(reg1),
                                                      GetNewRegOperand(reg2), GetNewRegOperand(result_reg)));
}

void IRgenArithmeticF32(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int reg1, int reg2, int result_reg) {
    B->InsertInstruction(1,
                         new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::FLOAT32, GetNewRegOperand(reg1),
                                                   GetNewRegOperand(reg2), GetNewRegOperand(result_reg)));
}

void IRgenArithmeticI32ImmLeft(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int val1, int reg2, int result_reg) {
    B->InsertInstruction(1, new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::I32, new ImmI32Operand(val1),
                                                      GetNewRegOperand(reg2), GetNewRegOperand(result_reg)));
}

void IRgenArithmeticF32ImmLeft(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, float val1, int reg2,
                               int result_reg) {
    B->InsertInstruction(1,
                         new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::FLOAT32, new ImmF32Operand(val1),
                                                   GetNewRegOperand(reg2), GetNewRegOperand(result_reg)));
}

void IRgenArithmeticI32ImmAll(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int val1, int val2, int result_reg) {
    B->InsertInstruction(1, new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::I32, new ImmI32Operand(val1),
                                                      new ImmI32Operand(val2), GetNewRegOperand(result_reg)));
}

void IRgenArithmeticF32ImmAll(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, float val1, float val2,
                              int result_reg) {
    B->InsertInstruction(1,
                         new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::FLOAT32, new ImmF32Operand(val1),
                                                   new ImmF32Operand(val2), GetNewRegOperand(result_reg)));
}

void IRgenIcmp(LLVMBlock B, BasicInstruction::IcmpCond cmp_op, int reg1, int reg2, int result_reg) {
    B->InsertInstruction(1, new IcmpInstruction(BasicInstruction::LLVMType::I32, GetNewRegOperand(reg1),
                                                GetNewRegOperand(reg2), cmp_op, GetNewRegOperand(result_reg)));
}

void IRgenFcmp(LLVMBlock B, BasicInstruction::FcmpCond cmp_op, int reg1, int reg2, int result_reg) {
    B->InsertInstruction(1, new FcmpInstruction(BasicInstruction::LLVMType::FLOAT32, GetNewRegOperand(reg1),
                                                GetNewRegOperand(reg2), cmp_op, GetNewRegOperand(result_reg)));
}

void IRgenIcmpImmRight(LLVMBlock B, BasicInstruction::IcmpCond cmp_op, int reg1, int val2, int result_reg) {
    B->InsertInstruction(1, new IcmpInstruction(BasicInstruction::LLVMType::I32, GetNewRegOperand(reg1),
                                                new ImmI32Operand(val2), cmp_op, GetNewRegOperand(result_reg)));
}

void IRgenFcmpImmRight(LLVMBlock B, BasicInstruction::FcmpCond cmp_op, int reg1, float val2, int result_reg) {
    B->InsertInstruction(1, new FcmpInstruction(BasicInstruction::LLVMType::FLOAT32, GetNewRegOperand(reg1),
                                                new ImmF32Operand(val2), cmp_op, GetNewRegOperand(result_reg)));
}

void IRgenFptosi(LLVMBlock B, int src, int dst) {
    B->InsertInstruction(1, new FptosiInstruction(GetNewRegOperand(dst), GetNewRegOperand(src)));
}
void IRgenFpext(LLVMBlock B, int src, int dst) {
    B->InsertInstruction(1, new FpextInstruction(GetNewRegOperand(dst), GetNewRegOperand(src)));
}

void IRgenSitofp(LLVMBlock B, int src, int dst) {
    B->InsertInstruction(1, new SitofpInstruction(GetNewRegOperand(dst), GetNewRegOperand(src)));
}

void IRgenZextI1toI32(LLVMBlock B, int src, int dst) {
    B->InsertInstruction(1, new ZextInstruction(BasicInstruction::LLVMType::I32, GetNewRegOperand(dst),
                                                BasicInstruction::LLVMType::I1, GetNewRegOperand(src)));
}

void IRgenGetElementptrIndexI32(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr,
                        std::vector<int> dims, std::vector<Operand> indexs) {
    B->InsertInstruction(1, new GetElementptrInstruction(type, GetNewRegOperand(result_reg), ptr, dims, indexs, BasicInstruction::I32));
}

void IRgenGetElementptrIndexI64(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr,
                        std::vector<int> dims, std::vector<Operand> indexs) {
    B->InsertInstruction(1, new GetElementptrInstruction(type, GetNewRegOperand(result_reg), ptr, dims, indexs, BasicInstruction::I64));
}

void IRgenLoad(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr) {
    B->InsertInstruction(1, new LoadInstruction(type, ptr, GetNewRegOperand(result_reg)));
}

void IRgenStore(LLVMBlock B, BasicInstruction::LLVMType type, int value_reg, Operand ptr) {
    B->InsertInstruction(1, new StoreInstruction(type, ptr, GetNewRegOperand(value_reg)));
}

void IRgenStore(LLVMBlock B, BasicInstruction::LLVMType type, Operand value, Operand ptr) {
    B->InsertInstruction(1, new StoreInstruction(type, ptr, value));
}

void IRgenCall(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg,
               std::vector<std::pair<enum BasicInstruction::LLVMType, Operand>> args, std::string name) {
    B->InsertInstruction(1, new CallInstruction(type, GetNewRegOperand(result_reg), name, args));
}

void IRgenCallVoid(LLVMBlock B, BasicInstruction::LLVMType type,
                   std::vector<std::pair<enum BasicInstruction::LLVMType, Operand>> args, std::string name) {
    B->InsertInstruction(1, new CallInstruction(type, GetNewRegOperand(-1), name, args));
}

void IRgenCallNoArgs(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, std::string name) {
    B->InsertInstruction(1, new CallInstruction(type, GetNewRegOperand(result_reg), name));
}

void IRgenCallVoidNoArgs(LLVMBlock B, BasicInstruction::LLVMType type, std::string name) {
    B->InsertInstruction(1, new CallInstruction(type, GetNewRegOperand(-1), name));
}

void IRgenRetReg(LLVMBlock B, BasicInstruction::LLVMType type, int reg) {
    B->InsertInstruction(1, new RetInstruction(type, GetNewRegOperand(reg)));
}

void IRgenRetImmInt(LLVMBlock B, BasicInstruction::LLVMType type, int val) {
    B->InsertInstruction(1, new RetInstruction(type, new ImmI32Operand(val)));
}

void IRgenRetImmFloat(LLVMBlock B, BasicInstruction::LLVMType type, float val) {
    B->InsertInstruction(1, new RetInstruction(type, new ImmF32Operand(val)));
}

void IRgenRetVoid(LLVMBlock B) {
    B->InsertInstruction(1, new RetInstruction(BasicInstruction::LLVMType::VOID, nullptr));
}

void IRgenBRUnCond(LLVMBlock B, int dst_label) {
    B->InsertInstruction(1, new BrUncondInstruction(GetNewLabelOperand(dst_label)));
}

void IRgenBrCond(LLVMBlock B, int cond_reg, int true_label, int false_label) {
    B->InsertInstruction(1, new BrCondInstruction(GetNewRegOperand(cond_reg), GetNewLabelOperand(true_label),
                                                  GetNewLabelOperand(false_label)));
}

void IRgenAlloca(LLVMBlock B, BasicInstruction::LLVMType type, int reg) {
    B->InsertInstruction(0, new AllocaInstruction(type, GetNewRegOperand(reg)));
}

void IRgenAllocaArray(LLVMBlock B, BasicInstruction::LLVMType type, int reg, std::vector<int> dims) {
    B->InsertInstruction(0, new AllocaInstruction(type, dims, GetNewRegOperand(reg)));
}

//增加数组array的irgen情况
void IRgenGetElementptr(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr, std::vector<int> dims,
                        std::vector<Operand> indexs) {
    B->InsertInstruction(1, new GetElementptrInstruction(type, GetNewRegOperand(result_reg), ptr, dims, indexs,BasicInstruction::I32));
}

Operand RegOperand::CopyOperand() { return GetNewRegOperand(reg_no); }

Operand ImmI32Operand::CopyOperand() { return new ImmI32Operand(immVal); }

Operand ImmF32Operand::CopyOperand() { return new ImmF32Operand(immVal); }

Operand ImmI64Operand::CopyOperand() { return new ImmI64Operand(immVal); }

Operand LabelOperand::CopyOperand() { return GetNewLabelOperand(label_no); }

Operand GlobalOperand::CopyOperand() { return GetNewGlobalOperand(name); }

int AllocaInstruction::GetAllocaSize() {
    int sz = 1;
    for (auto d : dims) {
        sz *= d;
    }
    return sz;
}

std::pair<int, int> GetElementptrInstruction::GetConstIndexes() {
    int ans = 0;
    int Siz = 1;

    for (auto sz : dims) {
        Siz *= sz;
    }

    for (int i = 0; i < indexes.size(); i++) {
        // std::cerr<<i<<" "<<indexes[i]->getFullName()<<"\n";
        if (indexes[i]->GetOperandType() == BasicOperand::IMMI32) {
            ans += (((ImmI32Operand *)indexes[i])->GetIntImmVal()) * Siz;
        } else if (indexes[i]->GetOperandType() == BasicOperand::REG) {
            return {-1, -1};
        }
        if (i < dims.size()) {
            Siz /= dims[i];
        }
    }

    return {ans, Siz};
}

int CallInstruction::GetResultRegNo() {
    if (result != NULL)
        return ((RegOperand *)result)->GetRegNo();
    else
        return -1;
}

std::vector<Operand> LoadInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    ret.push_back(pointer);
    return ret;
}

std::vector<Operand> StoreInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    ret.push_back(pointer);
    ret.push_back(value);
    return ret;
}

std::vector<Operand> ArithmeticInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    ret.push_back(op1);
    ret.push_back(op2);
    if (op3 != nullptr) {
        ret.push_back(op3);
    }
    return ret;
}

std::vector<Operand> IcmpInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    ret.push_back(op1);
    ret.push_back(op2);
    // ret.push_back(cond);
    return ret;
}

std::vector<Operand> FcmpInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    ret.push_back(op1);
    ret.push_back(op2);
    // ret.push_back(cond);
    return ret;
}

std::vector<Operand> PhiInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    for (auto label_val_pair : phi_list) {
        ret.push_back(label_val_pair.second);
    }
    return ret;
}

std::vector<Operand> BrCondInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    ret.push_back(cond);
    return ret;
}

std::vector<Operand> CallInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    for (auto arg_pair : args) {
        ret.push_back(arg_pair.second);
    }
    return ret;
}

std::vector<Operand> RetInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    if (ret_val != NULL)
        ret.push_back(ret_val);
    return ret;
}


std::vector<Operand> GetElementptrInstruction::GetNonResultOperands() {
    std::vector<Operand> ret(indexes);
    ret.push_back(ptrval);
    return ret;
}

std::vector<Operand> FunctionDefineInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    std::cerr << "func_define_Instruction get_nonresult_operands()\n";
    return ret;
}

std::vector<Operand> FunctionDeclareInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    std::cerr << "func_declare_Instruction get_nonresult_operands()\n";
    return ret;
}

std::vector<Operand> FptosiInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    ret.push_back(value);
    return ret;
}

std::vector<Operand> FpextInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    ret.push_back(value);
    return ret;
}

std::vector<Operand> SitofpInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    ret.push_back(value);
    return ret;
}


std::vector<Operand> ZextInstruction::GetNonResultOperands() {
    std::vector<Operand> ret;
    ret.push_back(value);
    return ret;
}







