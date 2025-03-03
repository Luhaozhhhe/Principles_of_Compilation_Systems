#include "IRgen.h"
#include "../include/ir.h"
#include "semant.h"



/*
-------------------------------------------------------------
中间代码生成 现在还没有写完的部分：
- AddExp_plus OK
- AddExp_sub OK
- MulExp_mul OK
- MulExp_div OK
- MulExp_mod OK
- RelExp_leq OK
- RelExp_lt OK
- RelExp_geq OK
- RelExp_gt OK
- EqExp_eq OK
- EqExp_neq OK
- LAndExp_and OK
- LOrExp_or OK
- Lval——这个比较难
- FuncRParams OK
- Func_call——这个比较难
- UnaryExp_plus OK
- UnaryExp_neg OK
- UnaryExp_not OK
- IntConst OK
- FloatConst OK
- StringConst 涉及到全局变量的字符串 待会看
- assign_stmt OK
- block_stmt OK
- ifelse_stmt OK
- if_stmt OK
- while_stmt OK
- continue_stmt OK
- break_stmt OK
- return_stmt OK
- return_stmt_void OK
- ConstInitVal OK
- ConstInitVal_exp OK
- VarInitVal OK
- VarInitVal_exp OK
- VarDef_no_init OK
- VarDef OK
- ConstDef OK
- VarDecl——这个比较难
- ConstDecl——这个比较难
- BlockItem_Decl OK
- BlockItem_Stmt OK
- __Block OK
- __FuncFParam OK
- __FuncDef


-------------------------------------------------------------
*/



extern SemantTable semant_table;    // 也许你会需要一些语义分析的信息

IRgenTable irgen_table;    // 中间代码生成的辅助变量
LLVMIR llvmIR;             // 我们需要在这个变量中生成中间代码


//extern std::map<int, FunctionDefineInstruction*> function_funcDefIns_map;
int max_reg = -1;

int max_block_label = -1;
int curr_block_label = 0;

int temp_reg = -1;
int temp_fol = -1;
static int loop_start_label = -1;    // 处理continue语句的label
static int loop_end_label = -1;      // 处理break语句的label

std::map<FuncDefInstruction, int> max_label_map{};
std::map<FuncDefInstruction, int> max_reg_map{};

static FuncDefInstruction function_now;

//类型转换：type类型转llvm类型
static BasicInstruction::LLVMType type2llvm(Type::ty TY)
{
    switch(TY)
    {
        case 0:
            return BasicInstruction::LLVMType::VOID;
        case 1:
            return BasicInstruction::LLVMType::I32;
        case 2:
            return BasicInstruction::LLVMType::FLOAT32;
        case 3:
            return BasicInstruction::LLVMType::I1;
        case 4:
            return BasicInstruction::LLVMType::PTR; 
        case 5:
            return BasicInstruction::LLVMType::DOUBLE;
        default:
            return BasicInstruction::LLVMType::VOID;
    }
}

//类型转换：llvm类型转type类型
static Type::ty llvm2type(BasicInstruction::LLVMType llvmty)
{
    switch(llvmty)
    {
        case 4:
            return Type::ty::VOID;
        case 1:
            return Type::ty::INT;
        case 2:
            return Type::ty::FLOAT;
        case 6:
            return Type::ty::BOOL;
        case 3:
            return Type::ty::PTR;
        case 8:
            return Type::ty::DOUBLE;
        default:
            assert(false);
        
    }
}

void AddLibFunctionDeclare();

// 一些指令，用于在基本块B末尾生成一条新指令
// 生成整数类型的算术指令（如加法、减法等），操作数来自两个寄存器
void IRgenArithmeticI32(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int reg1, int reg2, int result_reg);

// 生成浮点类型的算术指令（如加法、减法等），操作数来自两个寄存器
void IRgenArithmeticF32(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int reg1, int reg2, int result_reg);

// 生成整数类型的算术指令，左操作数为立即数，右操作数为寄存器
void IRgenArithmeticI32ImmLeft(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int val1, int reg2, int result_reg);

// 生成浮点类型的算术指令，左操作数为立即数，右操作数为寄存器
void IRgenArithmeticF32ImmLeft(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, float val1, int reg2, int result_reg);

// 生成整数类型的算术指令，两个操作数均为立即数
void IRgenArithmeticI32ImmAll(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int val1, int val2, int result_reg);

// 生成浮点类型的算术指令，两个操作数均为立即数
void IRgenArithmeticF32ImmAll(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, float val1, float val2, int result_reg);


// 生成整数类型的比较指令（如等于、大于等于等），操作数来自两个寄存器
void IRgenIcmp(LLVMBlock B, BasicInstruction::IcmpCond cmp_op, int reg1, int reg2, int result_reg);

// 生成浮点类型的比较指令（如等于、大于等于等），操作数来自两个寄存器
void IRgenFcmp(LLVMBlock B, BasicInstruction::FcmpCond cmp_op, int reg1, int reg2, int result_reg);

// 生成整数类型的比较指令，右操作数为立即数
void IRgenIcmpImmRight(LLVMBlock B, BasicInstruction::IcmpCond cmp_op, int reg1, int val2, int result_reg);

// 生成浮点类型的比较指令，右操作数为立即数
void IRgenFcmpImmRight(LLVMBlock B, BasicInstruction::FcmpCond cmp_op, int reg1, float val2, int result_reg);


// 生成浮点数到整数的类型转换指令
void IRgenFptosi(LLVMBlock B, int src, int dst);

// 生成整数到浮点数的类型转换指令
void IRgenSitofp(LLVMBlock B, int src, int dst);

// 将布尔值（i1）扩展为 32 位整数（i32）
void IRgenZextI1toI32(LLVMBlock B, int src, int dst);


// 根据索引生成数组元素地址（整数索引，指针类型为 i32）
void IRgenGetElementptrIndexI32(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr,
                                std::vector<int> dims, std::vector<Operand> indexs);

// 根据索引生成数组元素地址（整数索引，指针类型为 i64）
void IRgenGetElementptrIndexI64(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr,
                                std::vector<int> dims, std::vector<Operand> indexs);


// 从内存地址加载值到目标寄存器
void IRgenLoad(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr);

// 将寄存器值存储到指定内存地址
void IRgenStore(LLVMBlock B, BasicInstruction::LLVMType type, int value_reg, Operand ptr);

// 将立即数存储到指定内存地址
void IRgenStore(LLVMBlock B, BasicInstruction::LLVMType type, Operand value, Operand ptr);


// 调用函数（有返回值、有参数）
void IRgenCall(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg,
               std::vector<std::pair<enum BasicInstruction::LLVMType, Operand>> args, std::string name);

// 调用函数（无返回值、有参数）
void IRgenCallVoid(LLVMBlock B, BasicInstruction::LLVMType type,
                   std::vector<std::pair<enum BasicInstruction::LLVMType, Operand>> args, std::string name);

// 调用函数（有返回值、无参数）
void IRgenCallNoArgs(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, std::string name);

// 调用函数（无返回值、无参数）
void IRgenCallVoidNoArgs(LLVMBlock B, BasicInstruction::LLVMType type, std::string name);


// 返回寄存器值
void IRgenRetReg(LLVMBlock B, BasicInstruction::LLVMType type, int reg);

// 返回立即数（整数类型）
void IRgenRetImmInt(LLVMBlock B, BasicInstruction::LLVMType type, int val);

// 返回立即数（浮点类型）
void IRgenRetImmFloat(LLVMBlock B, BasicInstruction::LLVMType type, float val);

// 返回 void 类型
void IRgenRetVoid(LLVMBlock B);


// 无条件跳转到目标基本块
void IRgenBRUnCond(LLVMBlock B, int dst_label);

// 条件跳转，根据条件寄存器决定跳转到 `true_label` 或 `false_label`
void IRgenBrCond(LLVMBlock B, int cond_reg, int true_label, int false_label);


// 在栈上为指定类型分配空间
void IRgenAlloca(LLVMBlock B, BasicInstruction::LLVMType type, int reg);

// 在栈上为数组分配空间
void IRgenAllocaArray(LLVMBlock B, BasicInstruction::LLVMType type, int reg, std::vector<int> dims);

// 根据索引生成数组元素地址
void IRgenGetElementptr(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr, std::vector<int> dims,
                        std::vector<Operand> indexs);

// 根据寄存器编号创建一个新的寄存器操作数
RegOperand *GetNewRegOperand(int RegNo);


//根据源类型 (type_src) 和目标类型 (type_dst) 的不同，生成相应的中间表示（IR）指令来转换数据类型，处理赋值语句等stmt
void IRgenTypeConverse(LLVMBlock B, Type::ty type_src, Type::ty type_dst, int src) {
    switch (type_src) {
        case Type::INT:
            switch (type_dst) {
                case Type::INT:
                    return;
                case Type::FLOAT:
                    IRgenSitofp(B, src, ++max_reg);
                    break;
                case Type::BOOL:
                    IRgenIcmpImmRight(B, BasicInstruction::IcmpCond::ne, src, 0, ++max_reg);
                    break;
                default:
                    
                    break;
            }
            break;

        case Type::FLOAT:
            switch (type_dst) {
                case Type::INT:
                    IRgenFptosi(B, src, ++max_reg);
                    break;
                case Type::FLOAT:
                    return;
                case Type::BOOL:
                    IRgenFcmpImmRight(B, BasicInstruction::FcmpCond::ONE, src, 0, ++max_reg);
                    break;
                case Type::DOUBLE:
                    //IRgenFpext(B, src, ++max_reg);
                    break;
                default:
                    
                    break;
            }
            break;

        case Type::BOOL:
            switch (type_dst) {
                case Type::INT:
                    IRgenZextI1toI32(B, src, ++max_reg);
                    break;
                case Type::FLOAT:
                    IRgenZextI1toI32(B, src, ++max_reg);
                    src = max_reg;  // 更新 src 为最新的寄存器
                    IRgenSitofp(B, src, ++max_reg);
                    break;
                case Type::BOOL:
                    return;
                default:
                    
                    break;
            }
            break;
        
        default:
            
            break;
    }
}

//插入指令，在指令那个文件调用到了
void BasicBlock::InsertInstruction(int pos, Instruction Ins) {
    assert(pos == 0 || pos == 1);
    if (pos == 0) {
        Instruction_list.push_front(Ins);//在前面插入指令
    } else if (pos == 1) {
        Instruction_list.push_back(Ins);//在末尾插入指令
    }
}

/*
二元运算指令生成的伪代码：
    假设现在的语法树节点是：AddExp_plus
    该语法树表示 addexp + mulexp

    addexp->codeIR()
    mulexp->codeIR()
    假设mulexp生成完后，我们应该在基本块B0继续插入指令。
    addexp的结果存储在r0寄存器中，mulexp的结果存储在r1寄存器中
    生成一条指令r2 = r0 + r1，并将该指令插入基本块B0末尾。
    标注后续应该在基本块B0插入指令，当前节点的结果寄存器为r2。
    (如果考虑支持浮点数，需要查看语法树节点的类型来判断此时是否需要隐式类型转换)
*/

/*
while语句指令生成的伪代码：
    while的语法树节点为while(cond)stmt

    假设当前我们应该在B0基本块开始插入指令
    新建三个基本块Bcond，Bbody，Bend
    在B0基本块末尾插入一条无条件跳转指令，跳转到Bcond

    设置当前我们应该在Bcond开始插入指令
    cond->codeIR()    //在调用该函数前你可能需要设置真假值出口
    假设cond生成完后，我们应该在B1基本块继续插入指令，Bcond的结果为r0
    如果r0的类型不为bool，在B1末尾生成一条比较语句，比较r0是否为真。
    在B1末尾生成一条条件跳转语句，如果为真，跳转到Bbody，如果为假，跳转到Bend

    设置当前我们应该在Bbody开始插入指令
    stmt->codeIR()
    假设当stmt生成完后，我们应该在B2基本块继续插入指令
    在B2末尾生成一条无条件跳转语句，跳转到Bcond

    设置当前我们应该在Bend开始插入指令
*/

void __Program::codeIR() {
    AddLibFunctionDeclare();//首先在最前面插入一些库函数的声明
    std::vector<CompUnit> comp_vector = *comp_list;
    for (auto comp : comp_vector) {
        comp->codeIR();
    }
}

void Exp::codeIR() { addexp->codeIR(); }


//处理二元运算，暴力枚举，通过枚举左右节点的类型，去进行不同的转换
void AddExp_plus::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);//首先定位到当前的block块

    //如果是两个int类型的话，就直接对子节点进行ir生成，然后分配寄存器的编号
    if(addexp->attribute.T.type == Type::INT && mulexp->attribute.T.type == Type::INT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::ADD, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::INT && mulexp->attribute.T.type == Type::FLOAT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // 将int型转换为float型
        reg1 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FADD, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::INT && mulexp->attribute.T.type == Type::BOOL){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool转int
        reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::ADD, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::FLOAT && mulexp->attribute.T.type == Type::INT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // int转float
        reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FADD, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::FLOAT && mulexp->attribute.T.type == Type::FLOAT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FADD, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::FLOAT && mulexp->attribute.T.type == Type::BOOL){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg); 
        reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        //todo:wrong
        reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FADD, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::BOOL && mulexp->attribute.T.type == Type::INT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::ADD, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::BOOL && mulexp->attribute.T.type == Type::FLOAT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float
        reg1 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FADD, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::BOOL && mulexp->attribute.T.type == Type::BOOL){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::ADD, reg1, reg2, ++max_reg);

    }
    else{
        assert(false);//停止
    }


}

void AddExp_sub::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);

    if(addexp->attribute.T.type == Type::INT && mulexp->attribute.T.type == Type::INT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::SUB, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::INT && mulexp->attribute.T.type == Type::FLOAT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // a int->float
        reg1 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FSUB, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::INT && mulexp->attribute.T.type == Type::BOOL){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::SUB, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::FLOAT && mulexp->attribute.T.type == Type::INT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // a int->float
        reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FSUB, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::FLOAT && mulexp->attribute.T.type == Type::FLOAT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FSUB, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::FLOAT && mulexp->attribute.T.type == Type::BOOL){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FSUB, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::BOOL && mulexp->attribute.T.type == Type::INT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::SUB, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::BOOL && mulexp->attribute.T.type == Type::FLOAT){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float

        reg1 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FSUB, reg1, reg2, ++max_reg);

    }

    else if(addexp->attribute.T.type == Type::BOOL && mulexp->attribute.T.type == Type::BOOL){
        addexp->codeIR();
        int reg1 = max_reg;
        mulexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::SUB, reg1, reg2, ++max_reg);

    }
    else{
        assert(false);//停止
    }
}

void MulExp_mul::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);


    if(mulexp->attribute.T.type == Type::INT && unary_exp->attribute.T.type == Type::INT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::MUL, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::INT && unary_exp->attribute.T.type == Type::FLOAT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // a int->float
        reg1 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FMUL, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::INT && unary_exp->attribute.T.type == Type::BOOL){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::MUL, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::FLOAT && unary_exp->attribute.T.type == Type::INT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // a int->float

        reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FMUL, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::FLOAT && unary_exp->attribute.T.type == Type::FLOAT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FMUL, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::FLOAT && unary_exp->attribute.T.type == Type::BOOL){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FMUL, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::BOOL && unary_exp->attribute.T.type == Type::INT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::MUL, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::BOOL && unary_exp->attribute.T.type == Type::FLOAT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float

        reg1 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FMUL, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::BOOL && unary_exp->attribute.T.type == Type::BOOL){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int

        reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::MUL, reg1, reg2, ++max_reg);

    }
    else{
        assert(false);//停止
    }
}

void MulExp_div::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);


    if(mulexp->attribute.T.type == Type::INT && unary_exp->attribute.T.type == Type::INT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::DIV, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::INT && unary_exp->attribute.T.type == Type::FLOAT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // a int->float
        reg1 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FDIV, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::INT && unary_exp->attribute.T.type == Type::BOOL){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::DIV, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::FLOAT && unary_exp->attribute.T.type == Type::INT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // a int->float
        reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FDIV, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::FLOAT && unary_exp->attribute.T.type == Type::FLOAT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FDIV, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::FLOAT && unary_exp->attribute.T.type == Type::BOOL){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        reg2 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FDIV, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::BOOL && unary_exp->attribute.T.type == Type::INT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int

        reg1 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::DIV, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::BOOL && unary_exp->attribute.T.type == Type::FLOAT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float

        reg1 = max_reg;
        IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FDIV, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::BOOL && unary_exp->attribute.T.type == Type::BOOL){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::DIV, reg1, reg2, ++max_reg);

    }
    else{
        assert(false);//停止
    }
}

void MulExp_mod::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);

    if(mulexp->attribute.T.type == Type::INT && unary_exp->attribute.T.type == Type::INT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::MOD, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::INT && unary_exp->attribute.T.type == Type::FLOAT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // a int->float
        reg1 = max_reg;
        assert(false);

    }

    else if(mulexp->attribute.T.type == Type::INT && unary_exp->attribute.T.type == Type::BOOL){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::MOD, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::FLOAT && unary_exp->attribute.T.type == Type::INT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // a int->float
        reg2 = max_reg;
        assert(false);

    }

    else if(mulexp->attribute.T.type == Type::FLOAT && unary_exp->attribute.T.type == Type::FLOAT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        assert(false);

    }

    else if(mulexp->attribute.T.type == Type::FLOAT && unary_exp->attribute.T.type == Type::BOOL){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        reg2 = max_reg;
        assert(false);

    }

    else if(mulexp->attribute.T.type == Type::BOOL && unary_exp->attribute.T.type == Type::INT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::MOD, reg1, reg2, ++max_reg);

    }

    else if(mulexp->attribute.T.type == Type::BOOL && unary_exp->attribute.T.type == Type::FLOAT){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float
        reg1 = max_reg;
        assert(false);

    }

    else if(mulexp->attribute.T.type == Type::BOOL && unary_exp->attribute.T.type == Type::BOOL){
        mulexp->codeIR();
        int reg1 = max_reg;
        unary_exp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;
        IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::MOD, reg1, reg2, ++max_reg);

    }
    else{
        assert(false);//停止
    }
}


void RelExp_leq::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);

    if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sle, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // a int->float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OLE, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sle, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // b int->float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OLE, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OLE, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OLE, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sle, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OLE, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sle, reg1, reg2, ++max_reg);

    }

    else{
        assert(false);//停止
    }
}

void RelExp_lt::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);

    if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::slt, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // a int->float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OLT, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::slt, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // b int->float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OLT, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OLT, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OLT, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::slt, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OLT, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::slt, reg1, reg2, ++max_reg);

    }

    else{
        assert(false);//停止
    }
}

void RelExp_geq::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);

    if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sge, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // a int->float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OGE, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sge, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // b int->float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OGE, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OGE, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OGE, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sge, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OGE, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sge, reg1, reg2, ++max_reg);

    }

    else{
        assert(false);//停止
    }

}

void RelExp_gt::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);

    if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sgt, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // a int->float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OGT, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::INT && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sgt, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // b int->float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OGT, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OGT, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::FLOAT && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OGT, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::INT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sgt, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::FLOAT){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OGT, reg1, reg2, ++max_reg);

    }

    else if(relexp->attribute.T.type == Type::BOOL && addexp->attribute.T.type == Type::BOOL){
        relexp->codeIR();
        int reg1 = max_reg;
        addexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::sgt, reg1, reg2, ++max_reg);

    }

    else{
        assert(false);//停止
    }

}

void EqExp_eq::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);

    if(eqexp->attribute.T.type == Type::INT && relexp->attribute.T.type == Type::INT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::eq, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::INT && relexp->attribute.T.type == Type::FLOAT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // a int->float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OEQ, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::INT && relexp->attribute.T.type == Type::BOOL){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::eq, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::FLOAT && relexp->attribute.T.type == Type::INT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // b int->float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OEQ, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::FLOAT && relexp->attribute.T.type == Type::FLOAT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OEQ, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::FLOAT && relexp->attribute.T.type == Type::BOOL){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OEQ, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::BOOL && relexp->attribute.T.type == Type::INT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::eq, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::BOOL && relexp->attribute.T.type == Type::FLOAT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::OEQ, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::BOOL && relexp->attribute.T.type == Type::BOOL){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::eq, reg1, reg2, ++max_reg);

    }

    else{
        assert(false);//停止
    }
}

void EqExp_neq::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);

    if(eqexp->attribute.T.type == Type::INT && relexp->attribute.T.type == Type::INT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::ne, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::INT && relexp->attribute.T.type == Type::FLOAT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg1, ++max_reg);    // a int->float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::ONE, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::INT && relexp->attribute.T.type == Type::BOOL){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::ne, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::FLOAT && relexp->attribute.T.type == Type::INT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenSitofp(B, reg2, ++max_reg);    // b int->float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::ONE, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::FLOAT && relexp->attribute.T.type == Type::FLOAT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::ONE, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::FLOAT && relexp->attribute.T.type == Type::BOOL){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenSitofp(B, reg2, ++max_reg);    // int -> float
        reg2 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::ONE, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::BOOL && relexp->attribute.T.type == Type::INT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::ne, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::BOOL && relexp->attribute.T.type == Type::FLOAT){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenSitofp(B, reg1, ++max_reg);    // int -> float
        reg1 = max_reg;

        IRgenFcmp(B, BasicInstruction::FcmpCond::ONE, reg1, reg2, ++max_reg);

    }

    else if(eqexp->attribute.T.type == Type::BOOL && relexp->attribute.T.type == Type::BOOL){
        eqexp->codeIR();
        int reg1 = max_reg;
        relexp->codeIR();
        int reg2 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
        reg1 = max_reg;

        IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
        reg2 = max_reg;

        IRgenIcmp(B, BasicInstruction::IcmpCond::ne, reg1, reg2, ++max_reg);

    }

    else{
        assert(false);//停止
    }
}

//处理二元短路逻辑 与
void LAndExp_and::codeIR() {
    assert((true_label != -1) && (false_label != -1));//确保 true_label 和 false_label 已经被设置
    //int and_begin_label = curr_block_label;//定义初始的label数
    int start_label = curr_block_label;//start_label表示当前的label
    
    LLVMBlock t = llvmIR.NewBlock(function_now, max_block_label);//由于要跳转，new一个块
    int left_label = t->block_id;//然后把label赋值给left_label

    landexp->true_label = left_label;//表示左操作数为真的情况下需要跳转到的标签
    landexp->false_label = this->false_label;//如果false的话就直接跳转到整体的假分支
    landexp->codeIR();//对左值进行中间代码的生成
    LLVMBlock B1 = llvmIR.GetBlock(function_now, curr_block_label);//获取当前的块
    IRgenTypeConverse(B1, landexp->attribute.T.type, Type::BOOL, max_reg);//为了符合条件跳转，我们进行类型转换，转换为bool型
    IRgenBrCond(B1, max_reg, landexp->true_label, landexp->false_label);//设置条件跳转指令

    curr_block_label = left_label;
    //接下来开始判断右值的true和false
    eqexp->true_label = this->true_label;
    eqexp->false_label = this->false_label;

    eqexp->codeIR();//对右值进行中间代码生成

    LLVMBlock B2 = llvmIR.GetBlock(function_now, curr_block_label);

    Type::ty temp = eqexp->attribute.T.type;
    IRgenTypeConverse(B2, temp, Type::BOOL, max_reg);

}

// 处理二元短路逻辑 或
void LOrExp_or::codeIR() {//跟前面的整体逻辑是一样的，只不过变成了，只要有true的就赋值为true
    assert((true_label != -1) && (false_label != -1));
    //int and_begin_label = curr_block_label;//定义初始的label数
    LLVMBlock B0 = llvmIR.NewBlock(function_now, max_block_label);//新开一个块
    lorexp->true_label = this->true_label;//左边只要true了就全true了
    lorexp->false_label = B0->block_id;//false的话就赋值为右边的块
    lorexp->codeIR();//对左值进行中间代码的生成

    LLVMBlock B1 = llvmIR.GetBlock(function_now, curr_block_label);
    IRgenTypeConverse(B1, lorexp->attribute.T.type, Type::BOOL, max_reg);
    IRgenBrCond(B1, max_reg, this->true_label, B0->block_id);

    curr_block_label = B0->block_id;
    //接下来开始判断右值的true和false
    landexp->true_label = this->true_label;
    landexp->false_label = this->false_label;
    landexp->codeIR();//做右值的中间代码的生成

    LLVMBlock B2 = llvmIR.GetBlock(function_now, curr_block_label);

    Type::ty temp = landexp->attribute.T.type;
    IRgenTypeConverse(B2, temp, Type::BOOL, max_reg);

}





void ConstExp::codeIR(){
    addexp->codeIR();//这是一个常量的中间代码生成，直接使用对应的addexp进行codeir即可

}
//lval ref:https://github.com/yuhuifishash/SysY/blob/master/ir_gen/IRgen.cc
void Lval::codeIR() { 

    LLVMBlock BL = llvmIR.GetBlock(function_now, curr_block_label);//获取当前的块
    Operand temp_lval_operand;
    VarAttribute lval_Vattr;
    std::vector<Operand> lval_array_indexs;
    
    if (dims != nullptr) {//如果为数组
        for (auto d : *dims) {//遍历

            d->codeIR();
            Type::ty d_type = d->attribute.T.type;
            IRgenTypeConverse(BL, d_type, Type::INT, max_reg);
            lval_array_indexs.push_back(GetNewRegOperand(max_reg));//索引值寄存器存储到lval_array_indexs中

        }
    }
    int finding_reg = irgen_table.symbol_reg_table.lookup(name);//然后去找变量的位置
    bool formal_array_tag = false;
    if(finding_reg==-1)//如果是-1的话代表为全局变量
    {
        //没找到，说明在全局变量
        temp_lval_operand = GetNewGlobalOperand(name->get_string());//获取全局的操作数
        
        lval_Vattr = semant_table.GlobalTable[name];//查找全局变量的属性
        
    }
    else//为局部变量
    {
        temp_lval_operand = GetNewRegOperand(finding_reg);//获取寄存器的操作数


        lval_Vattr = irgen_table.Reg_VarA_Table[finding_reg];//去找到局部变量的属性

        formal_array_tag = irgen_table.Formal_Array_Table[finding_reg];//检查是不是函数参数数组，如果是的话就为true
    }

    BasicInstruction::LLVMType lval_llvm_type=type2llvm(lval_Vattr.type);

    if (lval_array_indexs.empty() == false ||
        attribute.T.type == Type::PTR) { //是数组或者指针
        if (formal_array_tag) {   //如果是函数参数数组的话，通过索引列表生成
            IRgenGetElementptr(BL, lval_llvm_type, ++max_reg, temp_lval_operand, lval_Vattr.dims, lval_array_indexs);
        } else {    // 普通的数组，前面需要插入0
            lval_array_indexs.insert(lval_array_indexs.begin(), new ImmI32Operand(0));
            IRgenGetElementptr(BL, lval_llvm_type, ++max_reg, temp_lval_operand, lval_Vattr.dims, lval_array_indexs);
        }
        temp_lval_operand = GetNewRegOperand(max_reg);    // ptr数组的最终地址
    }


    ptr = temp_lval_operand;
    if (is_left == false) {                     
        if (attribute.T.type != Type::PTR) {    //不处理数组
            IRgenLoad(BL, lval_llvm_type, ++max_reg, temp_lval_operand);
        }
    }

 }//左值的中间代码生成

void FuncRParams::codeIR(){

    //实际不会调用到这里来，因为实参都是表达式类型
}

//func_call ref:https://github.com/yuhuifishash/SysY/blob/master/ir_gen/semant.cc
void Func_call::codeIR() { //函数调用的中间代码生成

    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);

    Type::ty return_type = semant_table.FunctionTable[name]->return_type;//首先，从函数表中查询其返回值类型
    BasicInstruction::LLVMType ret_type = type2llvm(return_type);//然后转成llvm类型，进行返回
    if(funcr_params!=NULL)//处理含参的函数调用
    {
        //首先，完成实参和形参的匹配
        std::vector<std::pair<BasicInstruction::LLVMType, Operand>> args;
        std::vector<Expression>* realParams = ((FuncRParams *)funcr_params)->params;//实际传入的参数列表
        auto folmalParams = semant_table.FunctionTable[name]->formals;//我们在函数定义中赋值的形参列表
        assert(realParams->size() == folmalParams->size());//必须相等，不然就直接错误了
        for (int i = 0; i < (*realParams).size(); ++i) {
            auto realparam = (*realParams)[i];
            auto folmalparam = (*folmalParams)[i];
            realparam->codeIR();//如果是表达式，如add(1+2,3)等需要计算值后传入
            IRgenTypeConverse(B, realparam->attribute.T.type, folmalparam->attribute.T.type, max_reg);
            args.push_back({type2llvm(folmalparam->attribute.T.type), GetNewRegOperand(max_reg)});
        }
        if (return_type == Type::VOID) {//返回空的就调用IRgenCallVoid，不为空就调用IRgenCall
            IRgenCallVoid(B, ret_type, args, name->get_string());
        } 
        else {
            IRgenCall(B, ret_type, ++max_reg, args, name->get_string());
        }
    }
    else{//不含参的，如果返回值为空，就调用IRgenCallVoidNoArgs，不为空就调用IRgenCallNoArgs
        if (return_type == Type::VOID) {
            IRgenCallVoidNoArgs(B, ret_type, name->get_string());
        } 
        else {
            IRgenCallNoArgs(B, ret_type, ++max_reg, name->get_string());
        }
    }

}


//下面三个处理的是单目运算的中间代码生成
void UnaryExp_plus::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);
    //这个比前面的简单一些，单目的运算符只需要判断三种类型int float和bool就行了
    if(unary_exp->attribute.T.type == Type::INT){//int的话就直接调用int计算
        unary_exp->codeIR();
        //add的话不需要做什么处理
    }

    else if(unary_exp->attribute.T.type == Type::FLOAT){//float的话就直接调用float计算
        unary_exp->codeIR();

    }

    else if(unary_exp->attribute.T.type == Type::BOOL){//bool的话就是转成int，然后调用int运算
        unary_exp->codeIR();
        int reg1 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);

    }

    else{
        assert(false);//停止
    }

}

void UnaryExp_neg::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);
    if(unary_exp->attribute.T.type == Type::INT){
        unary_exp->codeIR();
        int reg1 = max_reg;
        IRgenArithmeticI32ImmLeft(B, BasicInstruction::LLVMIROpcode::SUB, 0, reg1, ++max_reg);

    }

    else if(unary_exp->attribute.T.type == Type::FLOAT){
        unary_exp->codeIR();
        int reg1 = max_reg;
        IRgenArithmeticF32ImmLeft(B, BasicInstruction::LLVMIROpcode::FSUB, 0, reg1, ++max_reg);

    }

    else if(unary_exp->attribute.T.type == Type::BOOL){
        unary_exp->codeIR();
        int reg1 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);
        reg1 = max_reg;
        IRgenArithmeticI32ImmLeft(B, BasicInstruction::LLVMIROpcode::SUB, 0, reg1, ++max_reg);

    }

    else{
        assert(false);//停止
    }

}

void UnaryExp_not::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);
    if(unary_exp->attribute.T.type == Type::INT){
        unary_exp->codeIR();
        int reg1 = max_reg;
        IRgenIcmpImmRight(B, BasicInstruction::IcmpCond::eq, reg1, 0, ++max_reg);

    }

    else if(unary_exp->attribute.T.type == Type::FLOAT){
        unary_exp->codeIR();
        int reg1 = max_reg;
        IRgenFcmpImmRight(B, BasicInstruction::FcmpCond::OEQ, reg1, 0, ++max_reg);

    }

    else if(unary_exp->attribute.T.type == Type::BOOL){
        unary_exp->codeIR();
        int reg1 = max_reg;
        IRgenZextI1toI32(B, reg1, ++max_reg);
        reg1 = max_reg;
        IRgenIcmpImmRight(B, BasicInstruction::IcmpCond::eq, reg1, 0, ++max_reg);
        
    }

    else{
        assert(false);//停止
    }

}




void IntConst::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);//获取当前的块
    int now_reg = ++max_reg;//更新寄存器的数量
    IRgenArithmeticI32ImmAll(B, BasicInstruction::LLVMIROpcode::ADD, val, 0, now_reg);
    //意思就是调用32位的整数add函数，然后使val和0相加，存到now_reg寄存器中，相当于将intconst的val存到了我们需要存储的寄存器中
}

void FloatConst::codeIR(){
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);
    int now_reg = ++max_reg;
    IRgenArithmeticF32ImmAll(B, BasicInstruction::LLVMIROpcode::FADD, val, 0, now_reg);//同理，将函数换成浮点数的运算即可
}

//string ref:https://github.com/yuhuifishash/SysY/blob/master/ir_gen/IRgen.cc
void StringConst::codeIR() { 
    int id = semant_table.GlobalStrTable[str];//id是该字符串常量在全局范围内的唯一标识
    //根据字符串ID生成一个全局操作数，将生成的全局操作数存储到 irgen_table.current_putin_ptr
    irgen_table.current_putin_ptr = GetNewGlobalOperand(".str" + std::to_string(id));
    
}

void PrimaryExp_branch::codeIR(){
    exp->codeIR();

}

void assign_stmt::codeIR(){//赋值语句的中间代码生成，使用前面定义的类型转换函数
    LLVMBlock B = llvmIR.GetBlock(function_now, curr_block_label);
    lval->codeIR();
    exp->codeIR();
    Type::ty a = lval->attribute.T.type;
    Type::ty b = exp->attribute.T.type;
    //IRgenTypeConverse(B, a, b, max_reg);//先通过类型的转换，保证我们的源和目标的类型相同
    IRgenTypeConverse(B, b,a, max_reg);//先通过类型的转换，保证我们的源和目标的类型相同
    BasicInstruction::LLVMType lval_type = type2llvm(lval->attribute.T.type);

    //生成存储操作的 LLVM IR 代码。它的作用是将一个已经计算好的右值存储到指定的左值位置（通常是内存中的某个变量）
    IRgenStore(B, lval_type, GetNewRegOperand(max_reg), ((Lval *)lval)->ptr);//使用store指令进行存储

}

void expr_stmt::codeIR(){
    exp->codeIR();

}

void block_stmt::codeIR(){//代码块的中间代码生成
    irgen_table.symbol_reg_table.enter_scope();//首先，新建一个中间代码生成的作用域
    b->codeIR();//对该block的块进行中间代码生成

    irgen_table.symbol_reg_table.exit_scope();//完成中间代码生成后，退出当前的作用域
}

void ifelse_stmt::codeIR(){
    //LLVMBlock B_basic = llvmIR.NewBlock(function_now, max_block_label);
    //LLVMBlock B_basic = llvmIR.GetBlock(function_now, max_block_label);
    LLVMBlock B_if = llvmIR.NewBlock(function_now, max_block_label);
    LLVMBlock B_else = llvmIR.NewBlock(function_now, max_block_label);
    LLVMBlock B_end = llvmIR.NewBlock(function_now, max_block_label);
    //初始话三个label，分别表示if语句 else语句和结束语句
    int if_label = B_if->block_id;
    int else_label = B_else->block_id;
    int end_label = B_end->block_id;

    Cond->true_label = if_label;
    Cond->false_label = else_label;
    Cond->codeIR();

    //curr_block_label = B_basic->block_id;
    LLVMBlock B_c = llvmIR.GetBlock(function_now, curr_block_label);
    Type::ty Cond_type = Cond->attribute.T.type;
    IRgenTypeConverse(B_c, Cond_type, Type::BOOL, max_reg);
    IRgenBrCond(B_c, max_reg, if_label, else_label);

    //进入if语句的中间代码生成
    curr_block_label = if_label;
    ifstmt->codeIR();
    LLVMBlock B_t = llvmIR.GetBlock(function_now, curr_block_label);
    IRgenBRUnCond(B_t, end_label);

    curr_block_label = else_label;
    elsestmt->codeIR();
    LLVMBlock B_f = llvmIR.GetBlock(function_now, curr_block_label);
    IRgenBRUnCond(B_f, end_label);

    curr_block_label = end_label;//最后到了结束，将当前的label赋值为结束时的label

}

void if_stmt::codeIR(){
    //if_stmt相比于上面的ifelse_stmt语句，少了一种状态，就更好写了
    //LLVMBlock B_basic = llvmIR.NewBlock(function_now, max_block_label);
    //LLVMBlock B_basic = llvmIR.GetBlock(function_now, max_block_label);
    LLVMBlock B_if = llvmIR.NewBlock(function_now, max_block_label);
    LLVMBlock B_then = llvmIR.NewBlock(function_now, max_block_label);
    int if_label = B_if->block_id;
    int end_label = B_then->block_id;

    Cond->true_label = if_label;
    Cond->false_label = end_label;
    Cond->codeIR();

    //curr_block_label = B_basic->block_id;
    LLVMBlock B_basic = llvmIR.GetBlock(function_now, curr_block_label);
    Type::ty Cond_type = Cond->attribute.T.type;
    IRgenTypeConverse(B_basic, Cond_type, Type::BOOL, max_reg);
    IRgenBrCond(B_basic, max_reg, if_label, end_label);

    //进入if语句的中间代码生成
    curr_block_label = if_label;
    ifstmt->codeIR();
    LLVMBlock B_t = llvmIR.GetBlock(function_now, curr_block_label);
    IRgenBRUnCond(B_t, end_label);

    //如果不是if分支的话，相当于直接进入end分支，将当前的label设置为end_label即可
    curr_block_label = end_label;

}


void while_stmt::codeIR(){
    //处理while语句，这个比较难一些，要在全局申明一个变量loop的label，因为不知道要循环几次
    LLVMBlock B_basic = llvmIR.NewBlock(function_now, max_block_label);
    //LLVMBlock B_basic = llvmIR.GetBlock(function_now, max_block_label);
    LLVMBlock B_true = llvmIR.NewBlock(function_now, max_block_label);
    LLVMBlock B_false = llvmIR.NewBlock(function_now, max_block_label);
    
    int basic_lable = B_basic->block_id;
    int t_label = B_true->block_id;
    int f_label = B_false->block_id;

    int tmp_start = loop_start_label;
    int tmp_end = loop_end_label;
    //嵌套的enter与恢复
    
    loop_start_label = B_basic->block_id;
    loop_end_label = f_label;

    LLVMBlock B_before = llvmIR.GetBlock(function_now, curr_block_label);
    IRgenBRUnCond(B_before, basic_lable);
    //现在已经生成
    /*
    L0:
        i=0;
        br L1
    L1:
        flag = i<n
    L2:
    L3:

    */


    curr_block_label=B_basic->block_id;

    Cond->true_label = t_label;
    Cond->false_label = f_label;
    Cond->codeIR();
    
    
    
    
    LLVMBlock B_c = llvmIR.GetBlock(function_now, curr_block_label);    
    Type::ty Cond_type = Cond->attribute.T.type;
    IRgenTypeConverse(B_c, Cond_type, Type::BOOL, max_reg);
    IRgenBrCond(B_c, max_reg, t_label, f_label);
    //现在已经生成
    /*
    L0:
        i=0;
        br L1
    L1:
        flag = i<n
        br flag l2 l3
    L2:
    L3:

    */
    curr_block_label = t_label;
    body->codeIR();
    LLVMBlock B_t = llvmIR.GetBlock(function_now, curr_block_label); 
    IRgenBRUnCond(B_t, B_basic->block_id);
    
    //现在已经生成
    /*
    L0:
        i=0;
        br L1
    L1:
        flag = i<n
        br flag l2 l3
    L2:
        i++;
        br l1
    L3:

    */
    curr_block_label = f_label;
    loop_start_label = tmp_start;
    loop_end_label = tmp_end;
}

void continue_stmt::codeIR(){
    //处理continue语句
    //用于跳过当前循环的剩余部分，直接进入下一次循环的判断或开始部分
    LLVMBlock B = llvmIR.function_block_map[function_now][curr_block_label];//获取当前函数的基本块
    IRgenBRUnCond(B, loop_start_label);//生成一条无条件跳转指令，将控制流转移到循环的开始标签
    //curr_block_label = loop_start_label;
    curr_block_label = llvmIR.NewBlock(function_now, max_block_label)->block_id;

}

void break_stmt::codeIR() { 
    LLVMBlock B = llvmIR.function_block_map[function_now][curr_block_label];//获取当前函数的基本块
    IRgenBRUnCond(B, loop_end_label);//生成一条无条件跳转指令，将控制流转移到循环的结束标签
    //curr_block_label = loop_end_label;
    curr_block_label = llvmIR.NewBlock(function_now, max_block_label)->block_id;
}

void return_stmt::codeIR() {
    return_exp->codeIR();
    LLVMBlock BL = llvmIR.GetBlock(function_now, curr_block_label);
    IRgenTypeConverse(BL, return_exp->attribute.T.type, llvm2type(function_now->GetReturnType()), max_reg);
    IRgenRetReg(BL, function_now->GetReturnType(), max_reg);
}

void return_stmt_void::codeIR() { 
    LLVMBlock BL = llvmIR.GetBlock(function_now, curr_block_label);
    IRgenRetVoid(BL);
}

void ConstInitVal::codeIR(){
    //后续完成对应的中间代码生成
}

void ConstInitVal_exp::codeIR(){
    exp->codeIR();//直接对其表达式进行中间代码生成即可

}

void VarInitVal::codeIR(){

}

void VarInitVal_exp::codeIR(){
    exp->codeIR();

}

void VarDef_no_init::codeIR(){

}

void VarDef::codeIR(){

}

void ConstDef::codeIR(){

}

//数组的初始化
/*
B:当前基本块
ary_type:数组元素类型
reg_ary:数组的基址寄存器操作数
dims:各维度大小的列表
init:初始化列表
cur_dim:当前维度，从 0 开始
index:当前初始化的元素索引，数组从 0 开始计数
last_required_num:上一维度需要初始化的总数量
total_size:数组的总大小，用于多维索引计算
*/
void Init_Array(LLVMBlock B, Type::ty ary_type, 
int reg_ary, std::vector<int> dims, InitVal init, 
int cur_dim, int index, int last_required_num, int total_size)
{   
    //a[5][4][3]={{1} , {{5,4} , 3+5}, 2 , {7,8,9} };
    //5个[4][3] 12
    int dims_number = dims[cur_dim];//当前维度的大小
    //当前维度需要初始化的元素数量，由上一维度的数量 last_required_num 除以当前维度大小计算得出
    int required_num = last_required_num / dims_number;
    int cur_num = 0;
    //{{1},{{5,4},3},2}
    for(auto init_item : *init->GetList())//初始化列表的每一个元素
    {
        if(init_item->IsExp())//是数(没有{})的话
        {
            init_item->GetExp()->codeIR();
            IRgenTypeConverse(B, init_item->GetExp()->attribute.T.type, ary_type, max_reg);

            int reg_val = max_reg;//存储了常量的ir
            int reg_adr = ++max_reg;

            std::vector<Operand> arrayindexs;
            int temp_size = total_size;//60
            int index_left = index;//13
            int cnt = 0;

            while(index_left != 0)//遍历，存储当前元素的多维索引
            {
                arrayindexs.push_back(new ImmI32Operand(index_left / temp_size));//0*60+ 1*12+ 0*3+ 1
                index_left %= temp_size;//13%60 13%12 1%3
                if(index_left == 0)
                    break;
                temp_size /= dims[cnt++];// 60/5 12/4 3/3
            }

            IRgenGetElementptrIndexI32(B, type2llvm(ary_type), reg_adr, GetNewRegOperand(reg_ary), dims, arrayindexs);
            IRgenStore(B, type2llvm(ary_type), GetNewRegOperand(reg_val), GetNewRegOperand(reg_adr));

            cur_num = (cur_num + 1) % required_num;
            index = index + 1;
        }
        else
        {   
            Assert(cur_num == 0);
            Assert(cur_dim < dims.size() - 1);
            //递归查找:每遇到一个{}就跳过一个维度
            Init_Array(B, ary_type, reg_ary, dims, init_item, cur_dim + 1, index, required_num, total_size);
            index = index + required_num;
        }
    }
}

//vardecl and constdecl ref:https://github.com/yuhuifishash/SysY/blob/master/ir_gen/IRgen.cc
void VarDecl::codeIR(){//变量声明
    LLVMBlock B_start = llvmIR.GetBlock(function_now, 0);//获取该变量所在的函数的基本初始块
    LLVMBlock B_now = llvmIR.GetBlock(function_now, curr_block_label);//获取该变量所在的当前块，用于处理变量初始化
    std::vector<Def> def_vector = *var_def_list;//获取到变量参数的列表
    for(auto def : def_vector){
        VarAttribute val;
        val.type = type_decl;//初始化
        //max_reg++;//将要读取新的一个寄存器
        irgen_table.symbol_reg_table.add_Symbol(def->GetName(), ++max_reg);//写到irgen表中去
        int alloca_reg = max_reg;
        if(def->GetDims()!=nullptr){//是一个数组
            auto dim_vector = *def->GetDims();
            for (auto d : dim_vector) {    // init val.dims
                val.dims.push_back(d->attribute.V.val.IntVal);
            }
            IRgenAllocaArray(B_start, type2llvm(type_decl), alloca_reg, val.dims);
            irgen_table.Reg_VarA_Table[alloca_reg] = val;

            InitVal init = def->GetInit();
            if (init != nullptr) {
                int array_sz = 1;
                for (auto d : val.dims) {
                    array_sz *= d;
                }

                CallInstruction *memsetCall = new CallInstruction(BasicInstruction::VOID, nullptr, std::string("llvm.memset.p0.i32"));
                memsetCall->push_back_Parameter(BasicInstruction::PTR, GetNewRegOperand(alloca_reg));    // array address
                memsetCall->push_back_Parameter(BasicInstruction::I8, new ImmI32Operand(0));
                memsetCall->push_back_Parameter(BasicInstruction::I32, new ImmI32Operand(array_sz * sizeof(int)));
                memsetCall->push_back_Parameter(BasicInstruction::I1, new ImmI32Operand(0));
                llvmIR.function_block_map[function_now][curr_block_label]->InsertInstruction(1, memsetCall);
                Init_Array(B_now, type_decl, max_reg, val.dims, init, 0, 0, array_sz, array_sz);
            }
        }
        else{//不是一个数组
            IRgenAlloca(B_start, type2llvm(type_decl), alloca_reg);
            irgen_table.Reg_VarA_Table[alloca_reg] = val;
            Operand val_operand;

            InitVal none_array_init = def->GetInit();
            if(none_array_init != nullptr){//如果已经完成了初始化的话
                Expression initExp = none_array_init->GetExp();
                initExp->codeIR();
                Type::ty initExp_Type = initExp->attribute.T.type;
                IRgenTypeConverse(B_now, initExp_Type, type_decl, max_reg);
                val_operand = GetNewRegOperand(max_reg);

            }
            else{//就是没有初始化的情况，直接根据其类型给他分配就可以
                switch(type_decl){
                    case Type::INT:
                        IRgenArithmeticI32ImmAll(B_now, BasicInstruction::LLVMIROpcode::ADD, 0, 0, ++max_reg);
                        val_operand = GetNewRegOperand(max_reg);
                        break;
                    case Type::FLOAT:
                        IRgenArithmeticF32ImmAll(B_now, BasicInstruction::LLVMIROpcode::FADD, 0, 0, ++max_reg);
                        val_operand = GetNewRegOperand(max_reg);
                        break;
                    default:
                        break;
                }
            }
            IRgenStore(B_now, type2llvm(type_decl), val_operand, GetNewRegOperand(alloca_reg));
        }


    }



}




void ConstDecl::codeIR(){//常量声明
    LLVMBlock B_start = llvmIR.GetBlock(function_now, 0);//获取该变量所在的函数的基本初始块
    LLVMBlock B_now = llvmIR.GetBlock(function_now, curr_block_label);//获取该变量所在的当前块，用于处理变量初始化
    std::vector<Def> def_vector = *var_def_list;//获取到变量参数的列表
    for(auto def : def_vector){
        VarAttribute val;
        val.type = type_decl;//初始化
        //max_reg++;//将要读取新的一个寄存器
        irgen_table.symbol_reg_table.add_Symbol(def->GetName(), ++max_reg);//写到irgen表中去
        int alloca_reg = max_reg;
        if(def->GetDims()!=nullptr){//是一个数组
            auto dim_vector = *def->GetDims();
            for (auto d : dim_vector) {    // init val.dims
                val.dims.push_back(d->attribute.V.val.IntVal);
            }
            IRgenAllocaArray(B_start, type2llvm(type_decl), alloca_reg, val.dims);
            irgen_table.Reg_VarA_Table[alloca_reg] = val;

            InitVal init = def->GetInit();

            if (init != nullptr) {
                int array_sz = 1;
                for (auto d : val.dims) {
                    array_sz *= d;
                }

                CallInstruction *memsetCall = new CallInstruction(BasicInstruction::VOID, nullptr, std::string("llvm.memset.p0.i32"));
                memsetCall->push_back_Parameter(BasicInstruction::PTR, GetNewRegOperand(alloca_reg));    // array address
                memsetCall->push_back_Parameter(BasicInstruction::I8, new ImmI32Operand(0));
                memsetCall->push_back_Parameter(BasicInstruction::I32, new ImmI32Operand(array_sz * sizeof(int)));
                memsetCall->push_back_Parameter(BasicInstruction::I1, new ImmI32Operand(0));
                llvmIR.function_block_map[function_now][curr_block_label]->InsertInstruction(1, memsetCall);
                Init_Array(B_now, type_decl, max_reg, val.dims, init, 0, 0, array_sz, array_sz);
            }
        }
        else{//不是一个数组
            IRgenAlloca(B_start, type2llvm(type_decl), alloca_reg);
            irgen_table.Reg_VarA_Table[alloca_reg] = val;
            Operand val_operand;

            InitVal none_array_init = def->GetInit();

            assert(none_array_init != nullptr);//判断是否为初始化，如果没有就错了
            Expression initExp = none_array_init->GetExp();
            initExp->codeIR();
            Type::ty initExp_Type = initExp->attribute.T.type;
            IRgenTypeConverse(B_now, initExp_Type, type_decl, max_reg);
            val_operand = GetNewRegOperand(max_reg);


            IRgenStore(B_now, type2llvm(type_decl), val_operand, GetNewRegOperand(alloca_reg));
        }


    }
}

void BlockItem_Decl::codeIR(){
    decl->codeIR();

}

void BlockItem_Stmt::codeIR(){
    stmt->codeIR();

}

void __Block::codeIR() {
    irgen_table.symbol_reg_table.enter_scope();//进入对应的作用域
    std::vector<BlockItem> item_vector = *item_list;
    for(auto item:item_vector)
    {
        item->codeIR();
    }
    irgen_table.symbol_reg_table.exit_scope();//完成中间代码的递归生成之后，出作用域

}

void __FuncFParam::codeIR(){
    VarAttribute val;
    val.type = type_decl;

    BasicInstruction::LLVMType lltype = type2llvm(type_decl);
    FuncDefInstruction FuncDefIns=function_now;
    LLVMBlock BL= llvmIR.GetBlock(function_now, curr_block_label);
    if (dims != nullptr) {    // formal is array
        // in SysY, we can assume that we can not modify the array address, so we do not need alloca
        FuncDefIns->InsertFormal(BasicInstruction::LLVMType::PTR);

        for (int i = 1; i < dims->size(); ++i) {    // in IRgen, we ignore the first dim of the
                                                            // formal
            auto d = dims->at(i);
            val.dims.push_back(d->attribute.V.val.IntVal);
        }

        irgen_table.Formal_Array_Table[temp_fol] = 1;
        irgen_table.symbol_reg_table.add_Symbol(name, temp_fol);
        irgen_table.Reg_VarA_Table[temp_fol] = val;
    } else {    // formal is not array
        FuncDefIns->InsertFormal(lltype);
        IRgenAlloca(BL, lltype, ++max_reg);
        IRgenStore(BL, lltype, GetNewRegOperand(temp_fol), GetNewRegOperand(max_reg));
        irgen_table.symbol_reg_table.add_Symbol(name, max_reg);
        irgen_table.Reg_VarA_Table[max_reg] = val;
    }
}
void __FuncDef::codeIR() {
    // add FuncDef llvm Instructions
    irgen_table.symbol_reg_table.enter_scope();

    BasicInstruction::LLVMType FuncDeclRetType = type2llvm(return_type);
    FuncDefInstruction FuncDefIns = new FunctionDefineInstruction(FuncDeclRetType, name->get_string());

    max_reg = -1;
    irgen_table.Reg_VarA_Table.clear();
    irgen_table.Formal_Array_Table.clear();

    curr_block_label = 0;
    max_block_label = -1;
    function_now = FuncDefIns;
    static Type::ty function_returntype = Type::VOID;
    function_returntype = return_type;

    llvmIR.NewFunction(function_now);
    LLVMBlock B = llvmIR.NewBlock(function_now, max_block_label);
    std::vector<FuncFParam> formal_vector = *formals;
    max_reg = formal_vector.size() - 1;
    for (int i = 0; i < formal_vector.size(); ++i) {
        FuncFParam formal = formal_vector[i];
        temp_fol = i;
        formal->codeIR();
        
    }
    //IRgenBRUnCond(B, max_block_label);
    //  ref:https://github.com/yuhuifishash/SysY/blob/master/ir_gen/IRgen.cc
    //IRgenBRUnCond(B, 1);//在一个函数里重新计数，生成局部变量后进入新块

    //B = llvmIR.NewBlock(function_now, max_block_label);
    curr_block_label = max_block_label;
    block->codeIR();

    for (auto block : llvmIR.function_block_map[function_now]) {
        LLVMBlock B = block.second;
        if (B->Instruction_list.empty() ||
            (!(B->Instruction_list.back()->GetOpcode() == BasicInstruction::LLVMIROpcode::RET) &&
            !(B->Instruction_list.back()->GetOpcode() == BasicInstruction::LLVMIROpcode::BR_COND ||
              B->Instruction_list.back()->GetOpcode() == BasicInstruction::LLVMIROpcode::BR_UNCOND))) {
            if (function_returntype == Type::VOID) {
                IRgenRetVoid(B);
            } else if (function_returntype == Type::INT) {
                IRgenRetImmInt(B, BasicInstruction::LLVMType::I32, 0);
            } else if (function_returntype == Type::FLOAT) {
                IRgenRetImmFloat(B, BasicInstruction::LLVMType::FLOAT32, 0);
            }
        }
    }
    max_reg_map[FuncDefIns] = max_reg;
    max_label_map[FuncDefIns] = max_block_label;

    irgen_table.symbol_reg_table.exit_scope();
}




void CompUnit_Decl::codeIR(){
    //不用做中间代码的生成
}

void CompUnit_FuncDef::codeIR(){
    func_def->codeIR();

}

void AddLibFunctionDeclare() {
    FunctionDeclareInstruction *getint = new FunctionDeclareInstruction(BasicInstruction::I32, "getint");
    llvmIR.function_declare.push_back(getint);

    FunctionDeclareInstruction *getchar = new FunctionDeclareInstruction(BasicInstruction::I32, "getch");
    llvmIR.function_declare.push_back(getchar);

    FunctionDeclareInstruction *getfloat = new FunctionDeclareInstruction(BasicInstruction::FLOAT32, "getfloat");
    llvmIR.function_declare.push_back(getfloat);

    FunctionDeclareInstruction *getarray = new FunctionDeclareInstruction(BasicInstruction::I32, "getarray");
    getarray->InsertFormal(BasicInstruction::PTR);
    llvmIR.function_declare.push_back(getarray);

    FunctionDeclareInstruction *getfloatarray = new FunctionDeclareInstruction(BasicInstruction::I32, "getfarray");
    getfloatarray->InsertFormal(BasicInstruction::PTR);
    llvmIR.function_declare.push_back(getfloatarray);

    FunctionDeclareInstruction *putint = new FunctionDeclareInstruction(BasicInstruction::VOID, "putint");
    putint->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(putint);

    FunctionDeclareInstruction *putch = new FunctionDeclareInstruction(BasicInstruction::VOID, "putch");
    putch->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(putch);

    FunctionDeclareInstruction *putfloat = new FunctionDeclareInstruction(BasicInstruction::VOID, "putfloat");
    putfloat->InsertFormal(BasicInstruction::FLOAT32);
    llvmIR.function_declare.push_back(putfloat);

    FunctionDeclareInstruction *putarray = new FunctionDeclareInstruction(BasicInstruction::VOID, "putarray");
    putarray->InsertFormal(BasicInstruction::I32);
    putarray->InsertFormal(BasicInstruction::PTR);
    llvmIR.function_declare.push_back(putarray);

    FunctionDeclareInstruction *putfarray = new FunctionDeclareInstruction(BasicInstruction::VOID, "putfarray");
    putfarray->InsertFormal(BasicInstruction::I32);
    putfarray->InsertFormal(BasicInstruction::PTR);
    llvmIR.function_declare.push_back(putfarray);

    FunctionDeclareInstruction *starttime = new FunctionDeclareInstruction(BasicInstruction::VOID, "_sysy_starttime");
    starttime->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(starttime);

    FunctionDeclareInstruction *stoptime = new FunctionDeclareInstruction(BasicInstruction::VOID, "_sysy_stoptime");
    stoptime->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(stoptime);

    // 一些llvm自带的函数，也许会为你的优化提供帮助
    FunctionDeclareInstruction *llvm_memset =
    new FunctionDeclareInstruction(BasicInstruction::VOID, "llvm.memset.p0.i32");
    llvm_memset->InsertFormal(BasicInstruction::PTR);
    llvm_memset->InsertFormal(BasicInstruction::I8);
    llvm_memset->InsertFormal(BasicInstruction::I32);
    llvm_memset->InsertFormal(BasicInstruction::I1);
    llvmIR.function_declare.push_back(llvm_memset);

    FunctionDeclareInstruction *llvm_umax = new FunctionDeclareInstruction(BasicInstruction::I32, "llvm.umax.i32");
    llvm_umax->InsertFormal(BasicInstruction::I32);
    llvm_umax->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(llvm_umax);

    FunctionDeclareInstruction *llvm_umin = new FunctionDeclareInstruction(BasicInstruction::I32, "llvm.umin.i32");
    llvm_umin->InsertFormal(BasicInstruction::I32);
    llvm_umin->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(llvm_umin);

    FunctionDeclareInstruction *llvm_smax = new FunctionDeclareInstruction(BasicInstruction::I32, "llvm.smax.i32");
    llvm_smax->InsertFormal(BasicInstruction::I32);
    llvm_smax->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(llvm_smax);

    FunctionDeclareInstruction *llvm_smin = new FunctionDeclareInstruction(BasicInstruction::I32, "llvm.smin.i32");
    llvm_smin->InsertFormal(BasicInstruction::I32);
    llvm_smin->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(llvm_smin);
}
