#include "semant.h"
#include "../include/SysY_tree.h"
#include "../include/ir.h"
#include "../include/type.h"
#include "../include/Instruction.h"

/*
    语义分析阶段需要对语法树节点上的类型和常量等信息进行标注, 即NodeAttribute类
    同时还需要标注每个变量的作用域信息，即部分语法树节点中的scope变量
    你可以在utils/ast_out.cc的输出函数中找到你需要关注哪些语法树节点中的NodeAttribute类及其他变量
    以及对语义错误的代码输出报错信息
*/

/*
    错误检查的基本要求:
    • 检查 main 函数是否存在 (根据SysY定义，如果不存在main函数应当报错)；————已完成，在program处进行总检查，然后在函数类中对main_flag进行赋值
    • 检查未声明变量，及在同一作用域下重复声明的变量；————这个应该是得利用符号表来实现，还没写
    • 条件判断和运算表达式：int 和 bool 隐式类型转换（例如int a=5，return a+!a）；————在类型检查type_check中已经完成
    • 数值运算表达式：运算数类型是否正确 (如返回值为 void 的函数调用结果是否参与了其他表达式的计算)；正在写
    • 检查未声明函数，及函数形参是否与实参类型及数目匹配；————还没写
    • 检查是否存在整型变量除以整型常量0的情况 (如对于表达式a/(5-4-1)，编译器应当给出警告或者直接报错)；————在几个除法类型检查中补充检查被除数是不是0就可以了

    错误检查的进阶要求:
    • 对数组维度进行相应的类型检查 (例如维度是否有浮点数，定义维度时是否为常量等)；————看起来比较麻烦
    • 对float进行隐式类型转换以及其他float相关的检查 (例如模运算中是否有浮点类型变量等)；————在type_check中已经完成了对float类型的检查
*/


/*
-------------------------------------------------------------
现在还没有写完的部分：
-符号表的实现
-Lval左值的typecheck
-FuncRParams
-Func_call
-StringConst
-assign_stmt
-while_stmt
-continue_stmt
-break_stmt
-return_stmt
-return_stmt_void
-ConstInitVal
-ConstInitVal_exp
-VarInitVal
-VarInitVal_exp
-VarDef_no_init
-VarDef
-ConstDef
-VarDecl
-ConstDecl
-CompUnit_Decl
-------------------------------------------------------------
*/



//全局变量llvmir，是中间代码生成的顶端
extern LLVMIR llvmIR;

//字符表
extern StringTable str_table;



/*
这个函数的作用是将抽象的类型枚举Type::ty

enum ty { VOID = 0, INT = 1, FLOAT = 2, BOOL = 3, PTR = 4, DOUBLE = 5 } type;

映射为 LLVM IR 的具体类型枚举 BasicInstruction::LLVMType

enum LLVMType { I32 = 1, FLOAT32 = 2, PTR = 3, VOID = 4, I8 = 5, I1 = 6, I64 = 7, DOUBLE = 8 };

用于编译器中类型转换和代码生成的过程。
*/
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




SemantTable semant_table;//是一个语义分析表的对象，用于存储程序中变量、函数、常量等的语义信息。
std::vector<std::string> error_msgs{}; // 将语义错误信息保存到该变量中

static bool main_flag = false;//定义main函数是否出现，如果bool值为true，就是出现了，反之就是没有出现

static int While_Number = 0;//将我们的while的嵌套次数进行初始化，为0即可

static int GlobalStrCnt = 0;//定义全局字符串的标签

static int GlobalVarCnt = 0;//用于为全局变量分配唯一的标识或索引

std::map<std::string, VarAttribute> ConstGlobalMap;//存储全局常量的映射表
std::map<std::string, VarAttribute> StaticGlobalMap;//存储静态全局变量的映射表


                    
/*
-------------------------------------------------------------
|                         2211044：声明我们的二元和单元的运算调用函数                             |
-------------------------------------------------------------
*/
NodeAttribute Perform_Binary_Operation(NodeAttribute a, NodeAttribute b, NodeAttribute::opcode opcode);
NodeAttribute Perform_Single_Operation(NodeAttribute a, NodeAttribute::opcode opcode);


//递归地初始化多维数组，根据提供的初始化值init和变量属性val，将数据存储到val对应的位置，并处理不同的数据类型或嵌套的初始化列表。
/*
参数说明：
init:表示当前的初始化值，可以是一个表达式（单一值）或嵌套的初始化列表。
val:变量的属性，在type.h中有定义
begpos:需要初始化的数组的范围，开始地址
endpos:需要初始化的数组的范围，结束地址
dimsIdx:表示当前正在处理的维度索引，用于多维数组递归初始化
*/
//ref: https://github.com/yuhuifishash/SysY/blob/master/ir_gen/semant.cc
void Semant_Array_Init(InitVal init, VarAttribute &val, int begPos, int endPos, int dimsIdx) {
    int pos = begPos;//首先，给pos赋值为开始地址，表示初始化开始

    if (init->IsExp()) {// 如果是表达式expression类型的初始化值
        if (init->attribute.T.type == Type::VOID) {//如果初始化表达式的类型为void的话，相当于不合法，抛出error
            error_msgs.push_back("Expression cannot be void in initval in line " + std::to_string(init->GetLineNumber()) + "\n");
        }
        if (pos <= endPos) {//如果不为空的话，就遍历我们的pos直到末尾，进行处理。
            switch (val.type) {//匹配val的类型，有int和float，其他都是error
                case Type::INT://如果是int类型的话，给val变量的pos位置的值赋值，如果为float类型就进行转换，如果是int类型就直接赋值
                    val.IntInitVals[pos] = init->attribute.T.type == Type::FLOAT
                                               ? static_cast<int>(init->attribute.V.val.FloatVal)
                                               : init->attribute.V.val.IntVal;
                    break;

                case Type::FLOAT://float也是一样的道理
                    val.FloatInitVals[pos] = init->attribute.T.type == Type::INT
                                                 ? static_cast<float>(init->attribute.V.val.IntVal)
                                                 : init->attribute.V.val.FloatVal;
                    break;

                default://如果不是这两种类型，就报错
                    error_msgs.push_back("Unsupported type in initval in line " + std::to_string(init->GetLineNumber()) + "\n");
            }
            pos++;//继续往后进行处理
        }
    } else {
        // 如果不是表达式，说明需要递归。进行处理嵌套的初始化
        auto initList = init->GetList();//首先获取到init的列表
        int blockSize = 1;//初始化当前块的size大小为1
        for (int i = dimsIdx + 1; i < val.dims.size(); ++i) {//这一块完成的是，根据我们给定的数组维度计算出我们应该需要的块大小
            blockSize *= val.dims[i];
        }

        for (auto &subInit : *initList) {
            if (pos > endPos) 
            {
                error_msgs.push_back("too many values that more than dims " + std::to_string(init->GetLineNumber()) + "\n");
                break;
            }
            Semant_Array_Init(subInit, val, pos, pos + blockSize - 1, dimsIdx + 1);//递归调用函数，来处理每一个块
            pos += blockSize;//处理完之后，移动到下一个块的起始位置进行处理
        }
    }
}

void Solve_Init_Val(InitVal init, VarAttribute &val, Type::ty type) {
    val.type = type;
    int arraySz = 1;
    for (auto d : val.dims) {
        arraySz *= d;
    }//首先需要确定下来数组的大小

    //根据类型的不同进行不同的初始化操作，int初始化为0，float初始化为0.0f
    if (type == Type::ty::INT) {
        val.IntInitVals.resize(arraySz, 0);
    } else if (type == Type::ty::FLOAT) {
        val.FloatInitVals.resize(arraySz, 0.0f);  // 使用浮点类型的默认值 0.0f
    }

    //如果不是数组的话
    if (val.dims.empty()) {
        if (init->GetExp() != nullptr) {//检查是否为表达式
            if (init->GetExp()->attribute.T.type == Type::ty::VOID) {//如果遇到void说明error
                error_msgs.push_back("Expression cannot be void in initval in line " +
                                     std::to_string(init->GetLineNumber()) + "\n");
            } else if (init->GetExp()->attribute.T.type == Type::ty::INT) {//int的情况，如果遇到int就直接初始化，float就转为float
                if (type == Type::ty::INT) {
                    val.IntInitVals[0] = init->GetExp()->attribute.V.val.IntVal;
                } else if (type == Type::ty::FLOAT) {
                    val.FloatInitVals[0] = static_cast<float>(init->GetExp()->attribute.V.val.IntVal);
                }
            } else if (init->GetExp()->attribute.T.type == Type::ty::FLOAT) {//float情况，如果遇到float就直接初始化，如果int就转为int
                if (type == Type::ty::INT) {
                    val.IntInitVals[0] = static_cast<int>(init->GetExp()->attribute.V.val.FloatVal);
                } else if (type == Type::ty::FLOAT) {
                    val.FloatInitVals[0] = init->GetExp()->attribute.V.val.FloatVal;
                }
            }
        }
        return;
    } else {//是数组的话
        if (init->IsExp()) {//如果初始化值是表达式（init->IsExp()），记录错误信息（数组不能用单一表达式初始化）
            error_msgs.push_back("InitVal cannot be expression in line " + std::to_string(init->GetLineNumber()) + "\n");
            return;
        } else {//如果初始化值是嵌套结构，则调用 Semant_Array_Init 递归处理数组的初始化。
            Semant_Array_Init(init, val, 0, arraySz - 1, 0);
        }
    }
}


//用于从 VarAttribute 对象中根据指定的多维索引 indexs 获取数组的具体值
template <typename T>
T GetArrayVal(VarAttribute &val, const std::vector<int> &indexs) {
    int idx = 0;
    for (int curIndex = 0; curIndex < indexs.size(); ++curIndex) {
        if (curIndex >= val.dims.size()) {
            throw std::out_of_range("Index out of range in GetArrayVal");
        }
        if (indexs[curIndex] >= val.dims[curIndex]) {
            throw std::out_of_range("Dimension index exceeds array size");
        }
        idx *= val.dims[curIndex];
        idx += indexs[curIndex];//计算出我们的数组多维索引位置的一维索引位
    }

    if constexpr (std::is_same<T, int>::value) {
        return val.IntInitVals.at(idx);
    } else if constexpr (std::is_same<T, float>::value) {
        return val.FloatInitVals.at(idx);
    } else {
        throw std::runtime_error("Unsupported type for GetArrayVal");
    }
}




//2211044:完成了对于main函数是否存在的检查
void __Program::TypeCheck() {
    semant_table.symbol_table.enter_scope();//进入作用域，开始该程序的检查
    auto comp_vector = *comp_list;
    for (auto comp : comp_vector) {
        comp->TypeCheck();
    }
    if(main_flag == false){//也就是说main函数在我们的func检查中均没有找到main，对应的bool值为false
        error_msgs.push_back("main function does not exist!!!\n");

    }

}

void Exp::TypeCheck() {//对于expression表达式来说，直接对其变量addexp做typecheck即可，然后将属性节点值赋值为addexp的属性
    addexp->TypeCheck();

    attribute = addexp->attribute;
}


//二元的类型检查和赋值。首先对两个子节点分别做typecheck，然后将属性赋值为对应的类型。
//调用逻辑：Perform_Binary_Operation->Semant_Int_Int->Calculate_Binary_Int->Binary_Add_Int，最后进行计算
void AddExp_plus::TypeCheck() {
    addexp->TypeCheck();
    mulexp->TypeCheck();

    attribute = Perform_Binary_Operation(addexp->attribute, mulexp ->attribute, NodeAttribute::ADD);

}

void AddExp_sub::TypeCheck() {
    addexp->TypeCheck();
    mulexp->TypeCheck();

    attribute = Perform_Binary_Operation(addexp->attribute, mulexp->attribute, NodeAttribute::SUB);
}

void MulExp_mul::TypeCheck() {
    mulexp->TypeCheck();
    unary_exp->TypeCheck();

    attribute = Perform_Binary_Operation(mulexp->attribute, unary_exp->attribute, NodeAttribute::MUL);
}

void MulExp_div::TypeCheck() {
    mulexp->TypeCheck();
    unary_exp->TypeCheck();

    attribute = Perform_Binary_Operation(mulexp->attribute, unary_exp->attribute, NodeAttribute::DIV);
}

void MulExp_mod::TypeCheck() {
    mulexp->TypeCheck();
    unary_exp->TypeCheck();

    attribute = Perform_Binary_Operation(mulexp->attribute, unary_exp->attribute, NodeAttribute::MOD);
}

void RelExp_leq::TypeCheck() {
    relexp->TypeCheck();
    addexp->TypeCheck();

    attribute = Perform_Binary_Operation(relexp->attribute, addexp->attribute, NodeAttribute::LEQ);
}

void RelExp_lt::TypeCheck() {
    relexp->TypeCheck();
    addexp->TypeCheck();

    attribute = Perform_Binary_Operation(relexp->attribute, addexp->attribute, NodeAttribute::LT);
}

void RelExp_geq::TypeCheck() {
    relexp->TypeCheck();
    addexp->TypeCheck();

    attribute = Perform_Binary_Operation(relexp->attribute, addexp->attribute, NodeAttribute::GEQ);
}

void RelExp_gt::TypeCheck() {
    relexp->TypeCheck();
    addexp->TypeCheck();

    attribute = Perform_Binary_Operation(relexp->attribute, addexp->attribute, NodeAttribute::GT);
}

void EqExp_eq::TypeCheck() {
    eqexp->TypeCheck();
    relexp->TypeCheck();

    attribute = Perform_Binary_Operation(eqexp->attribute, relexp->attribute, NodeAttribute::EQ);
}

void EqExp_neq::TypeCheck() {
    eqexp->TypeCheck();
    relexp->TypeCheck();

    attribute = Perform_Binary_Operation(eqexp->attribute, relexp->attribute, NodeAttribute::NE);
}

void LAndExp_and::TypeCheck() {
    landexp->TypeCheck();
    eqexp->TypeCheck();

    attribute = Perform_Binary_Operation(landexp->attribute, eqexp->attribute, NodeAttribute::AND);
}

void LOrExp_or::TypeCheck() {
    lorexp->TypeCheck();
    landexp->TypeCheck();

    attribute = Perform_Binary_Operation(lorexp->attribute, landexp->attribute,NodeAttribute::OR);
}

void ConstExp::TypeCheck() {
    addexp->TypeCheck();
    attribute = addexp->attribute;
    if (!attribute.V.ConstTag) {//判断属性是否为常量，如果不是的话，就报错
        error_msgs.push_back("Expression is not const " + std::to_string(line_number) + "\n");
    }
}
//ref: https://github.com/yuhuifishash/SysY/blob/master/ir_gen/semant.cc
void Lval::TypeCheck() {
    is_left = false;//初始化为false，表示当前语义分析中尚未确认该值可以作为左值使用
    std::vector<int> lval_array_indexs;//用于保存数组索引值，表示当前左值的维度索引
    bool array_index_ConstTag = true;//用于跟踪索引是否全为常量。如果所有维度的索引都是常量，则为true；否则为false
    if (dims != nullptr) {//左值的维度不为空，也就是说这是一个数组！
        for (auto d : *dims) {
            d->TypeCheck();
            if (d->attribute.T.type == Type::ty::VOID || d->attribute.T.type == Type::ty::FLOAT) {
                error_msgs.push_back("Array Dim cannot be void or float in line " + std::to_string(line_number) + "\n");
            }
            lval_array_indexs.push_back(d->attribute.V.val.IntVal);//将维度的具体索引值存储到lval_array_indexs中，以备后续使用
            //如果扫描到的属性有一个不是常量，那么数组就不是常量
            if(d->attribute.V.ConstTag == false){
                array_index_ConstTag = false;
            }
            else{
                array_index_ConstTag = true;
            }
        }
    }

    VarAttribute val = semant_table.symbol_table.lookup_val(name);
    //局部变量找不到，全局变量能找到
    if (val.type == Type::ty::VOID && semant_table.GlobalTable.find(name) != semant_table.GlobalTable.end()) {
        val = semant_table.GlobalTable[name];//给val赋值为全局变量表中存储的值，并且将作用域赋值为0
        scope = 0;
    } 
    else if (val.type != Type::ty::VOID) {//如果能在局部变量中找到，那么就查找其所在作用域的编号
        scope = semant_table.symbol_table.lookup_scope(name);
    } 
    else {//如果都不是，那就报error
        error_msgs.push_back("Undefined var in line " + std::to_string(line_number) + "\n");
        return;
    }

    if (lval_array_indexs.size() == val.dims.size()) {    // 左值是一个数组中的一个具体元素
        //必须得变量本身和索引均为常数，其属性才会为常数
        if(val.ConstTag == true && array_index_ConstTag == true){
            attribute.V.ConstTag = true;
        }
        else{
            attribute.V.ConstTag = false;
        }

        attribute.T.type = val.type;
        if (attribute.V.ConstTag == true) {//如果是常量，则根据类型（INT或FLOAT）获取对应的值
            if (attribute.T.type == Type::ty::INT) {
                attribute.V.val.IntVal = GetArrayVal<int>(val, lval_array_indexs);
            } 
            else if (attribute.T.type == Type::ty::FLOAT) {
                attribute.V.val.FloatVal = GetArrayVal<float>(val, lval_array_indexs);
            }
        }
    } 
    else if (lval_array_indexs.size() < val.dims.size()) {    //左值是一个数组的部分内容
        attribute.V.ConstTag = false;
        attribute.T.type = Type::ty::PTR;//就是指针类型的
    } 
    else {//如果都不是的话，就报错
        error_msgs.push_back("Array is unmatched in line " + std::to_string(line_number) + "\n");
    }
}



void FuncRParams::TypeCheck(){

}

void Func_call::TypeCheck() {
    int funcr_params_number = 0;//表示实参的数量
    if (funcr_params != nullptr) {//说明有参数，是实参
        auto params = static_cast<FuncRParams *>(funcr_params)->params;//首先，提取参数列表
        funcr_params_number = params->size();
        for (auto param : *params) {
            param->TypeCheck();//遍历检查
            if (param->attribute.T.type == Type::ty::VOID) {//实参的类型不能为空！
                error_msgs.push_back("FuncRParam is void in line " + std::to_string(line_number) + "\n");
            }
        }
    }
    //检查函数是否已经定义
    auto func_it = semant_table.FunctionTable.find(name);//首先在我们的语义分析函数表中查找该name
    if (func_it == semant_table.FunctionTable.end()) {//如果没有的话，说明未定义，error
        error_msgs.push_back("Function is undefined in line " + std::to_string(line_number) + "\n");
        return;
    }

    FuncDef funcdef = func_it->second;//形参
    if (funcdef->formals->size() != funcr_params_number) {//如果形参和实参的数量不对应的话，报错
        error_msgs.push_back("Function FuncFParams and FuncRParams are not matched in line " +
                             std::to_string(line_number) + "\n");
    }
    std::vector<std::pair<BasicInstruction::LLVMType, Operand>> args;
    // std::vector<Expression>* realParams = ((FuncRParams *)funcr_params)->params;//实际传入的参数列表
    // auto folmalParams = semant_table.FunctionTable[name]->formals;//我们在函数定义中赋值的形参列表
    // assert(realParams->size() == folmalParams->size());//必须相等，不然就直接错误了
    // for (int i = 0; i < (*realParams).size(); ++i) {
    //     auto realparam = (*realParams)[i];
    //     auto folmalparam = (*folmalParams)[i];
    //     if(realparam->attribute.T.type != folmalparam->attribute.T.type)
    //     {
    //         int flag_fol_can_trans = 0;
    //         int flag_re_can_trans = 0;
    //         if(realparam->attribute.T.type==Type::ty::INT ||realparam->attribute.T.type==Type::ty::FLOAT || realparam->attribute.T.type==Type::ty::BOOL )
    //         {
    //             flag_re_can_trans=1;
    //         }
    //         if(folmalparam->attribute.T.type==Type::ty::INT ||folmalparam->attribute.T.type==Type::ty::FLOAT || folmalparam->attribute.T.type==Type::ty::BOOL )
    //         {
    //             flag_fol_can_trans = 1;
    //         }
    //         if(!(flag_fol_can_trans && flag_re_can_trans))
    //         {
    //             error_msgs.push_back("paras not match in line " + std::to_string(line_number) + "\n");
    //         }
    //     }

    // }



    attribute.T.type = funcdef->return_type;//提取返回值类型
    attribute.V.ConstTag = false;//函数调用的结果基本不为常量
}

void UnaryExp_plus::TypeCheck(){
    unary_exp->TypeCheck();
    attribute = Perform_Single_Operation(unary_exp->attribute, NodeAttribute::ADD);
    
}

void UnaryExp_neg::TypeCheck(){
    unary_exp->TypeCheck();
    attribute = Perform_Single_Operation(unary_exp->attribute, NodeAttribute::SUB);

}

void UnaryExp_not::TypeCheck(){
    unary_exp->TypeCheck();
    attribute = Perform_Single_Operation(unary_exp->attribute, NodeAttribute::NOT);
    attribute.T.type = (Type::BOOL);
}

void IntConst::TypeCheck() {
    attribute.T.type = Type::INT;
    attribute.V.ConstTag = true;
    attribute.V.val.IntVal = val;
}

void FloatConst::TypeCheck() {
    attribute.T.type = Type::FLOAT;
    attribute.V.ConstTag = true;
    attribute.V.val.FloatVal = val;
}

void StringConst::TypeCheck() {
    attribute.T.type = Type::PTR;//字符串常量的类型设置为ptr，因为基本上是指针
    GlobalStrCnt += 1;//增加全局字符串计数器 GlobalStrCnt，确保为每个字符串常量分配唯一的标识符
    //根据全局字符串计数器生成一个唯一的标识符
    auto GlobalStrI = new GlobalStringConstInstruction(str->get_string(), ".str" + std::to_string(GlobalStrCnt));
    //将生成的全局字符串定义指令添加到全局 LLVM IR 定义列表 llvmIR.global_def 中
    llvmIR.global_def.push_back(GlobalStrI);
    //将当前字符串常量 str 的值与其全局索引 GlobalStrCnt 关联起来，并存储到语义分析表 GlobalStrTable 中
    semant_table.GlobalStrTable[str] = GlobalStrCnt;
}

void PrimaryExp_branch::TypeCheck() {//基本表达式的检查
    exp->TypeCheck();
    attribute = exp->attribute;
}

void assign_stmt::TypeCheck(){
    lval->TypeCheck();
    exp->TypeCheck();
    ((Lval *)lval)->is_left = true;//先强制将我们的stmt赋值为左值,表示可以被赋值
    if(exp->attribute.T.type == Type::VOID){//如果右值没有类型的话，抛出错误即可
        error_msgs.push_back("void type cannot be the assign_stmt's expression! " + std::to_string(line_number) + "\n");
    }

}

void expr_stmt::TypeCheck() {
    exp->TypeCheck();
    attribute = exp->attribute;
}

void block_stmt::TypeCheck(){
    b->TypeCheck();//递归调用即可

}

void ifelse_stmt::TypeCheck() {
    Cond->TypeCheck();
    if (Cond->attribute.T.type == Type::VOID) {
        error_msgs.push_back("if cond type is invalid " + std::to_string(line_number) + "\n");
    }
    ifstmt->TypeCheck();
    elsestmt->TypeCheck();

}

void if_stmt::TypeCheck() {
    Cond->TypeCheck();
    if (Cond->attribute.T.type == Type::VOID) {
        error_msgs.push_back("if cond type is invalid " + std::to_string(line_number) + "\n");
    }
    ifstmt->TypeCheck();

}

void while_stmt::TypeCheck(){
    Cond->TypeCheck();
    if(Cond->attribute.T.type == Type::VOID){
        error_msgs.push_back("while cond type is invalid " + std::to_string(line_number) + "\n");
    }
    /*
    while_number表示的是我们的循环嵌套的次数，我们每检查一个while循环，
    首先，我们将循环的次数加一，然后对该循环内部的body部分进行typecheck
    然后检查完成之后，我们再将其减去，表示我们已经完成了该部分的检查
    */
    While_Number = While_Number + 1;
    body->TypeCheck();
    While_Number = While_Number - 1;

}

void continue_stmt::TypeCheck(){
    if (While_Number == 0) {
        error_msgs.push_back("continue is not in while stmt in line " + std::to_string(line_number) + "\n");
    }
}

void break_stmt::TypeCheck(){
    if (While_Number == 0) {
        error_msgs.push_back("break is not in while stmt in line " + std::to_string(line_number) + "\n");
    }
}

void return_stmt::TypeCheck(){
    return_exp->TypeCheck();

    if (return_exp->attribute.T.type == Type::VOID) {//当return值为空的时候需要报错
        error_msgs.push_back("return type is invalid in line " + std::to_string(line_number) + "\n");
    }
}

void return_stmt_void::TypeCheck(){

}

void ConstInitVal::TypeCheck(){//遍历检查即可

    for(auto init_list : *initval){
        init_list->TypeCheck();

    }
}

void ConstInitVal_exp::TypeCheck(){//检查常量初始化的合理性
    if(exp == nullptr){//如果表达式为空的话，就直接返回，无需检查了
        return;
    }
    else{
        exp->TypeCheck();
        attribute = exp->attribute;

        if(attribute.T.type == Type::VOID){//初始化表达式不能为空，报错
            error_msgs.push_back("Initval expression can not be void in line " + std::to_string(line_number) + "\n");
        }
        if(attribute.V.ConstTag == false){//初始化表达式如果不为常量的话，报错
            error_msgs.push_back("Expression is not const! " + std::to_string(line_number) + "\n");
        }
    }
}

void VarInitVal::TypeCheck(){
    for(auto init_list : *initval){
        init_list->TypeCheck();

    }
}

void VarInitVal_exp::TypeCheck(){
    if(exp == nullptr){
        return;
    }
    else{
        exp->TypeCheck();
        attribute = exp->attribute;

        if(attribute.T.type == Type::VOID){
            error_msgs.push_back("Initval expression can not be void in line " + std::to_string(line_number) + "\n");
        }
    } 
}


void VarDef_no_init::TypeCheck(){
    
    
}

// bool dim_var_initval(std::vector<InitVal> *initval_list, std::vector<Expression>* dims)
// {
//     if(initval_list->size() > dims[0]->)

// }

void VarDef::TypeCheck(){
    // Symbol name;
    // std::vector<Expression> *dims;
    // InitVal init;
    //todo：判断维数有无超限
    // InitVal init_now = init;
    // std::vector<Expression>* dims_list = dims;
    // if(init->IsExp()==0)
    // {
    //     if(init->IsConst()==0)
    //     {
    //         //VarInitVal
    //         std::vector<InitVal> *initval_list = init->GetList();
    //         if(dim_var_initval(initval_list,  dims_list) == false)
    //         {
                
    //         }
    //         else
    //         {

    //         }

    //     }
    //     else{
    //         //ConstInitVal

    //     }
    // }
    // else
    // {
    //     if(init->IsConst()==0)
    //     {
    //         //VarInitVal_exp

    //     }
    //     else{
    //         //ConstInitVal_exp
    //     }
    // }
    //std::vector<InitVal> init_vec = init->attribu

}

void ConstDef::TypeCheck(){
    
}


/*
检查变量定义的合法性。
验证数组维度和初始化表达式的类型和常量属性。
处理变量作用域及符号表的更新。
记录多重定义、非法数组维度等语义错误信息。
*/
void VarDecl::TypeCheck() { 
    
    std::vector<Def> def_vector = *var_def_list;//获取所有变量定义的列表
    
    //std::vector<Def> * def_vector = var_def_list;
    for(auto def : def_vector)//遍历处理变量
    {
        Symbol s_name=def->GetName();
        int c_scope=semant_table.symbol_table.get_current_scope();//进入当前的作用域
        bool hadsymbol_flag = semant_table.symbol_table.lookup_current_scope_Symobol(s_name,c_scope);
        if(hadsymbol_flag==true)//如果重名的话，就报错
        {
            error_msgs.push_back("multiple definitions of " + def->GetName()->get_string() + std::to_string(line_number) + "\n");
        }
        
    }
    for(auto def : def_vector)//继续检查
    {
        Symbol s_name=def->GetName();
        int c_scope=semant_table.symbol_table.get_current_scope();
        VarAttribute varA;
        varA.ConstTag=0;
        varA.type=type_decl;
        def->scope=semant_table.symbol_table.get_current_scope();
        
        //包含4种可能：VarDef_list := IDENT Array_Dim_Number_list '=' VarInitVal |IDENT Array_Dim_Number_list |IDENT '=' VarInitVal|IDENT
        if(def->GetDims()!=NULL)//数组维度不为空，需要遍历检查
        {
            std::vector<Expression> dim_vector = *def->GetDims();
            for(auto c_dim : dim_vector)
            {
                c_dim->TypeCheck();//看看是不是个常量 比如int a[5][6]，中5和6都是常量
                if(c_dim->attribute.V.ConstTag != true)//如果不为常量的话，就报错 不是常量的数组维度
                {
                    error_msgs.push_back("not const array_dim" + def->GetName()->get_string() + std::to_string(line_number) + "\n");
                }
                if(c_dim->attribute.T.type != Type::INT)//如果不为int的话，就报不是int类型的数组维度
                {
                    error_msgs.push_back("not INT array_dim" + def->GetName()->get_string() + std::to_string(line_number) + "\n");
                }
                // if(c_dim->attribute.V.val < )
                // {

                // }
                varA.dims.push_back(c_dim->attribute.V.val.IntVal);
            }
            
            
        }
        
        if (def->GetInit() != nullptr) {//初始化检查，如果不为空的话，就也要检查
            def->GetInit()->TypeCheck();
        }
        semant_table.symbol_table.add_Symbol(s_name,varA);//添加到符号表中
    }


}

void ConstDecl::TypeCheck() { //和变量的声明基本是一样的，多了一个初始化的工作

    std::vector<Def> def_vector = *var_def_list;
    
    //std::vector<Def> * def_vector = var_def_list;
    for(auto def : def_vector)
    {
        Symbol s_name=def->GetName();
        int c_scope=semant_table.symbol_table.get_current_scope();
        bool hadsymbol_flag = semant_table.symbol_table.lookup_current_scope_Symobol(s_name,c_scope);
        if(hadsymbol_flag==true)
        {
            error_msgs.push_back("multiple definitions of " + def->GetName()->get_string() + std::to_string(line_number) + "\n");
        }
        
    }
    for(auto def : def_vector)
    {
        Symbol s_name=def->GetName();
        int c_scope=semant_table.symbol_table.get_current_scope();
        VarAttribute varA;
        varA.ConstTag=1;
        varA.type=type_decl;
        def->scope=semant_table.symbol_table.get_current_scope();
        //包含4种可能：ConstDef_list
        if(def->GetDims()!=NULL)
        {   //是数组：处理维数
            std::vector<Expression> dim_vector = *def->GetDims();
            for(auto c_dim : dim_vector)
            {
                c_dim->TypeCheck();//看看是不是个常量 比如int a[5][6]，中5和6都是常量
                if(c_dim->attribute.V.ConstTag != true)
                {
                    error_msgs.push_back("not const array_dim" + def->GetName()->get_string() + std::to_string(line_number) + "\n");
                }
                if(c_dim->attribute.T.type != Type::INT)
                {
                    error_msgs.push_back("not INT array_dim" + def->GetName()->get_string() + std::to_string(line_number) + "\n");
                }
                varA.dims.push_back(c_dim->attribute.V.val.IntVal);
            }
            if (def->GetInit() != nullptr){
                
                InitVal ini=def->GetInit();
                ini->TypeCheck();
                if (type_decl == Type::INT) 
                {
                    //TODO("初始化const数组的INT");
                    Solve_Init_Val(ini,varA,Type::INT);
                } 
                else if (type_decl == Type::FLOAT) 
                {
                    //TODO("初始化const数组的FLOAT");
                    Solve_Init_Val(ini,varA,Type::FLOAT);
                }
            }
            semant_table.symbol_table.add_Symbol(s_name,varA);
            
        }
        
        else{//不为数组时,初始化符号
            if (def->GetInit() != nullptr) {
                InitVal ini=def->GetInit();
                ini->TypeCheck();
                if (type_decl == Type::INT) 
                {
                    //varA.IntInitVals[0] = ini->GetExp()->attribute.V.val.IntVal;
                    Solve_Init_Val(ini,varA,Type::INT);
                } 
                else if (type_decl == Type::FLOAT) 
                {
                    //varA.FloatInitVals[0] = ini->GetExp()->attribute.V.val.FloatVal;
                    Solve_Init_Val(ini,varA,Type::FLOAT);
                }
            }
            semant_table.symbol_table.add_Symbol(s_name,varA);
        }
    }
}



void BlockItem_Decl::TypeCheck(){
    decl->TypeCheck(); 
}

void BlockItem_Stmt::TypeCheck(){
    stmt->TypeCheck(); 
}


void __Block::TypeCheck() {
    semant_table.symbol_table.enter_scope();//进入一个新的作用域
    auto item_vector = *item_list;
    for (auto item : item_vector) {//遍历检查
        item->TypeCheck();
    }
    semant_table.symbol_table.exit_scope();//检查完之后，就退出对应的作用域
}


void __FuncFParam::TypeCheck() {//函数形式的检查
    VarAttribute val;
    val.ConstTag = false;
    val.type = type_decl;
    scope = 1;

    //如果不为空的话，说明表示的为数组类型
    if (dims != nullptr) {    
        auto dim_vector = *dims;

        //如果第一个维度为空的话
        val.dims.push_back(-1);    // 这里用-1表示empty，你也可以使用其他方式
        for (int i = 1; i < dim_vector.size(); ++i) {
            auto d = dim_vector[i];
            d->TypeCheck();
            if (d->attribute.V.ConstTag == false) {
                error_msgs.push_back("Array Dim must be const expression in line " + std::to_string(line_number) +
                                     "\n");
            }
            if (d->attribute.T.type == Type::FLOAT) {
                error_msgs.push_back("Array Dim can not be float in line " + std::to_string(line_number) + "\n");
            }
            val.dims.push_back(d->attribute.V.val.IntVal);
        }
        attribute.T.type = Type::PTR;
    } 
    else {
        attribute.T.type = type_decl;
    }

    //检查名称合法性和多重定义
    if (name != nullptr) {
        if (semant_table.symbol_table.lookup_scope(name) != -1) {//不等于-1说明已经有定义了
            error_msgs.push_back("multiple definitions of formals in function " + name->get_string() + " in line " +
                                 std::to_string(line_number) + "\n");
        }
        semant_table.symbol_table.add_Symbol(name, val);//然后把这个函数加到对应的表中
    }
}

//2211044：在这个地方加上对main函数的检查，给main_flag赋值
void __FuncDef::TypeCheck() {
    semant_table.symbol_table.enter_scope();

    //检查main函数状态
    if(name->get_string() == "main"){
        main_flag = true;
    }

    semant_table.FunctionTable[name] = this;

    auto formal_vector = *formals;
    for (auto formal : formal_vector) {
        formal->TypeCheck();
    }

    // block TypeCheck
    if (block != nullptr) {
        auto item_vector = *(block->item_list);
        for (auto item : item_vector) {
            item->TypeCheck();
        }
    }

    semant_table.symbol_table.exit_scope();
}

void CompUnit_Decl::TypeCheck() {
    Type::ty type_decl = decl->GetTypedecl();
    auto def_vector = *decl->GetDefs();
    for (auto def : def_vector) {
        if (semant_table.GlobalTable.find(def->GetName()) != semant_table.GlobalTable.end()) {//检查重复定义的问题
            error_msgs.push_back("Multiple definitions of vars in line " + std::to_string(line_number) + "\n");
            continue;
        }

        VarAttribute val;
        val.ConstTag = def->IsConst();
        val.type = type_decl;
        def->scope = 0;

        auto dim_vector = def->GetDims();//检查数组维度，必须是常量表达式且类型不能是float
        if (dim_vector != nullptr) {
            for (auto d : *dim_vector) {
                d->TypeCheck();
                if (!d->attribute.V.ConstTag || d->attribute.T.type == Type::ty::FLOAT) {
                    error_msgs.push_back("Array Dim must be const expression and cannot be float in line " + std::to_string(line_number) + "\n");
                }
                val.dims.push_back(d->attribute.V.val.IntVal);
            }
        }
        //检查初始化表达式
        InitVal init = def->GetInit();
        if (init != nullptr) {
            init->TypeCheck();
            Solve_Init_Val(init, val, type_decl);//将初始化的结果存储到val中
        }

        if (def->IsConst()) {//如果变量是常量的话，记录到全局常量表ConstGlobalMap中
            ConstGlobalMap[def->GetName()->get_string()] = val;
        }
        //所有的全局变量都记录到StaticGlobalMap和GlobalTable中
        StaticGlobalMap[def->GetName()->get_string()] = val;
        semant_table.GlobalTable[def->GetName()] = val;

        //对于全局变量的中间代码生成，我们直接在此处完成即可
        BasicInstruction::LLVMType lltype = type2llvm(type_decl);
        Instruction globalDecl = nullptr;  // 添加默认初始化

        if (def->GetDims() != nullptr) {
            globalDecl = new GlobalVarDefineInstruction(def->GetName()->get_string(), lltype, val);
        } 
        else if (init == nullptr) {
            globalDecl = new GlobalVarDefineInstruction(def->GetName()->get_string(), lltype, nullptr);
        } 
        else {
            if (lltype == BasicInstruction::LLVMType::I32) {
                globalDecl = new GlobalVarDefineInstruction(def->GetName()->get_string(), lltype, new ImmI32Operand(val.IntInitVals[0]));
            } 
            else if (lltype == BasicInstruction::LLVMType::FLOAT32) {
                globalDecl = new GlobalVarDefineInstruction(def->GetName()->get_string(), lltype, new ImmF32Operand(val.FloatInitVals[0]));
            }
        }
        //将全局变量指令加到列表global_def中
        if (globalDecl != nullptr) {
            llvmIR.global_def.push_back(globalDecl);
        }
    }
}




void CompUnit_FuncDef::TypeCheck(){
    func_def->TypeCheck();

}