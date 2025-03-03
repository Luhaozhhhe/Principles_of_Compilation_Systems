%{
#include <fstream>
#include "SysY_tree.h"
#include "type.h"
Program ast_root;

void yyerror(char *s, ...);
int yylex();
int error_num = 0;
extern int line_number;
extern std::ofstream fout;
extern IdTable id_table;
%}
%union{
    char* error_msg;
    Symbol symbol_token;
    double float_token; // 对于SysY的浮点常量，我们需要先以double类型计算，再在语法树节点创建的时候转为float
    int int_token;
    Program program;  
    CompUnit comp_unit;  std::vector<CompUnit>* comps; 
    Decl decl;
    Def def;  std::vector<Def>* defs;
    FuncDef func_def;
    Expression expression;  std::vector<Expression>* expressions;
    Stmt stmt;
    Block block;
    InitVal initval;  std::vector<InitVal>* initvals;
    FuncFParam formal;   std::vector<FuncFParam>* formals;
    BlockItem block_item;   std::vector<BlockItem>* block_items;
}
//declare the terminals
%token <symbol_token> STR_CONST IDENT
%token <float_token> FLOAT_CONST
%token <int_token> INT_CONST
%token LEQ GEQ EQ NE // <=   >=   ==   != 
%token AND OR // &&    ||
%token ADDASSIGN MULASSIGN SUBASSIGN DIVASSIGN MODASSIGN  //added
%token CONST IF ELSE WHILE NONE_TYPE INT FLOAT FOR
%token RETURN BREAK CONTINUE ERROR

//give the type of nonterminals
%type <program> Program
%type <comp_unit> CompUnit 
%type <comps> Comp_list
%type <decl> Decl VarDecl ConstDecl
%type <def> ConstDef VarDef
%type <defs> ConstDef_list VarDef_list 
%type <func_def> FuncDef 
%type <expression> Exp LOrExp AddExp MulExp RelExp EqExp LAndExp UnaryExp PrimaryExp
%type <expression> ConstExp Lval FuncRParams Cond
%type <expression> IntConst FloatConst StringConst 
%type <expression> Array_Dim_Number ConstArray_Dim_Number
%type <expressions> Array_Dim_Number_list ConstArray_Dim_Number_list Exp_list;
%type <stmt> Stmt 
%type <block> Block
%type <block_item> BlockItem
%type <block_items> BlockItem_list
%type <initval> ConstInitVal VarInitVal  
%type <initvals> VarInitVal_list ConstInitVal_list
%type <formal> FuncFParam 
%type <formals> FuncFParams

// THEN和ELSE用于处理if和else的移进-规约冲突
%precedence THEN
%precedence ELSE
%%


Program 
:Comp_list
{
    @$ = @1;
    ast_root = new __Program($1);
    ast_root->SetLineNumber(line_number);
};

Comp_list
:CompUnit 
{
    ($$) = new std::vector<CompUnit>;
    ($$)->push_back($1);
}
|Comp_list CompUnit
{
    ($1)->push_back($2);
    ($$) = ($1);
};

CompUnit
:Decl
{
    ($$) = new CompUnit_Decl($1); 
    ($$)->SetLineNumber(line_number);
}
|FuncDef
{
    ($$) = new CompUnit_FuncDef($1); 
    ($$)->SetLineNumber(line_number);
}
;

Decl
:VarDecl
{
    ($$) = ($1); 
    ($$)->SetLineNumber(line_number);
}
|ConstDecl
{
    ($$) = ($1); 
    ($$)->SetLineNumber(line_number);
}
;

VarDecl
:INT VarDef_list ';'
{
    ($$) = new VarDecl(Type::INT,$2); 
    ($$)->SetLineNumber(line_number);
}
// 2211044: 考虑变量定义更多情况  
|FLOAT VarDef_list ';'
{
    ($$) = new VarDecl(Type::FLOAT,$2);
    ($$)->SetLineNumber(line_number);
} 
;

// 2211044: 考虑变量定义更多情况 
ConstDecl
:CONST INT ConstDef_list ';'{
    ($$) = new ConstDecl(Type::INT,$3); 
    ($$)->SetLineNumber(line_number);
}
|CONST FLOAT ConstDef_list ';'{
    ($$) = new ConstDecl(Type::FLOAT,$3);
    ($$)->SetLineNumber(line_number);
}
;

  
//2212514
VarDef_list
:VarDef
{
    ($$)= new std::vector<Def>;
    ($$)->push_back($1);
}
|VarDef_list ',' VarDef
{
    ($1)->push_back($3);
    ($$) = ($1);
}
;

ConstDef_list
:ConstDef
{
    ($$)= new std::vector<Def>;
    ($$)->push_back($1);
}
|ConstDef_list ',' ConstDef
{
    ($1)->push_back($3);
    ($$) = ($1);
}
;

FuncDef
:INT IDENT '(' FuncFParams ')' Block
{
    ($$) = new __FuncDef(Type::INT,$2,$4,$6);
    ($$)->SetLineNumber(line_number);
}
|INT IDENT '(' ')' Block
{
    ($$) = new __FuncDef(Type::INT,$2,new std::vector<FuncFParam>(),$5); 
    ($$)->SetLineNumber(line_number);
}
// 2211044: 考虑函数定义更多情况  
|FLOAT IDENT '(' FuncFParams ')' Block
{
    ($$) = new __FuncDef(Type::FLOAT,$2,$4,$6);
    ($$)->SetLineNumber(line_number);
}
|FLOAT IDENT '(' ')' Block
{
    ($$) = new __FuncDef(Type::FLOAT,$2,new std::vector<FuncFParam>(),$5);
    ($$)->SetLineNumber(line_number);
}
|NONE_TYPE IDENT '(' FuncFParams ')' Block
{
    ($$) = new __FuncDef(Type::VOID,$2,$4,$6); 
    ($$)->SetLineNumber(line_number);
}
|NONE_TYPE IDENT '(' ')' Block
{
    ($$) = new __FuncDef(Type::VOID,$2,new std::vector<FuncFParam>(),$5);
    ($$)->SetLineNumber(line_number);
}
;
  

VarDef
:IDENT Array_Dim_Number_list '=' VarInitVal
{
    ($$) = new VarDef($1,$2,$4); 
    ($$)->SetLineNumber(line_number);
}
|IDENT Array_Dim_Number_list
{
    ($$) = new VarDef_no_init($1,$2); 
    ($$)->SetLineNumber(line_number);
}
|IDENT '=' VarInitVal
{
    ($$) = new VarDef($1,nullptr,$3); 
    ($$)->SetLineNumber(line_number);
}
|IDENT
{
    ($$) = new VarDef_no_init($1,nullptr); 
    ($$)->SetLineNumber(line_number);
}
;

// 2212514: 考虑变量定义更多情况(数组)
//int a[5]= {6,7,8}中a[5]= {6,7,8}部分，a为IDENT, [5]为Array_Number_list, {6,7,8}为VarInitVal

ConstDef
:IDENT ConstArray_Dim_Number_list '=' ConstInitVal
{
    ($$) = new ConstDef($1,$2,$4); 
    ($$)->SetLineNumber(line_number);
}
|IDENT '=' ConstInitVal
{
    ($$) = new ConstDef($1,nullptr,$3); 
    ($$)->SetLineNumber(line_number);
}

;
//2212514：常量定义，没有不初始化的情况


ConstInitVal
:ConstExp
{
    ($$) = new ConstInitVal_exp($1); 
    ($$)->SetLineNumber(line_number);
}
|'{' ConstInitVal_list '}'
{
    ($$) = new ConstInitVal($2); 
    ($$)->SetLineNumber(line_number);
}
|'{' '}'
{
    ($$) = new ConstInitVal(new std::vector<InitVal>());
    ($$)->SetLineNumber(line_number);
}
;

VarInitVal
:Exp
{
    ($$) = new VarInitVal_exp($1); 
    ($$)->SetLineNumber(line_number);
}
|'{' VarInitVal_list '}'
{
    ($$) = new VarInitVal($2); 
    ($$)->SetLineNumber(line_number);
}
|'{' '}'
{
    ($$) = new VarInitVal(new std::vector<InitVal>()); 
    ($$)->SetLineNumber(line_number);
}
;

ConstInitVal_list
:ConstInitVal
{
    ($$)= new std::vector<InitVal>;
    ($$)->push_back($1);
}
|ConstInitVal_list ',' ConstInitVal
{
    ($1)->push_back($3);
    ($$) = ($1);
}
;

VarInitVal_list
:VarInitVal
{
    ($$)= new std::vector<InitVal>;
    ($$)->push_back($1);
}
|VarInitVal_list ',' VarInitVal
{
    ($1)->push_back($3);
    ($$) = ($1);
}
;


FuncFParams
:FuncFParam{
    ($$) = new std::vector<FuncFParam>;
    ($$)->push_back($1);
}
|FuncFParams ',' FuncFParam{
    ($1)->push_back($3);
    ($$) = ($1);
}
;

//2211044:补充函数的一些文法情况
FuncFParam
:INT IDENT
{
    ($$) = new __FuncFParam(Type::INT,$2,nullptr);
    ($$)->SetLineNumber(line_number);
}
|FLOAT IDENT
{
    ($$) = new __FuncFParam(Type::FLOAT,$2,nullptr);
    ($$)->SetLineNumber(line_number);
}
|INT IDENT '[' ']' Array_Dim_Number_list
{
    ($5)->insert(($5)->begin(),nullptr);
    ($$) = new __FuncFParam(Type::INT,($2),($5));
    ($$)->SetLineNumber(line_number);
}
|FLOAT IDENT '[' ']' Array_Dim_Number_list
{
    ($5)->insert(($5)->begin(),nullptr);
    ($$) = new __FuncFParam(Type::FLOAT,($2),($5));
    ($$)->SetLineNumber(line_number);
}
|INT IDENT '['  ']' 
{
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(nullptr);
    ($$) = new __FuncFParam(Type::INT,($2),temp);
    ($$)->SetLineNumber(line_number);
}
|FLOAT IDENT '['  ']' 
{
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(nullptr);
    ($$) = new __FuncFParam(Type::FLOAT,($2),temp);
    ($$)->SetLineNumber(line_number);
}
;


Block
:'{' BlockItem_list '}'{
    ($$) = new __Block($2);
    ($$)->SetLineNumber(line_number);
}
|'{' '}'{
    ($$) = new __Block(new std::vector<BlockItem>);
    ($$)->SetLineNumber(line_number);
}
;

BlockItem_list
:BlockItem
{
    ($$) = new std::vector<BlockItem>;
    ($$)->push_back($1);
}
|BlockItem_list BlockItem
{
    ($1)->push_back($2);
    ($$) = ($1);
}
;

BlockItem
:Decl
{
    ($$) = new BlockItem_Decl($1);
    ($$)->SetLineNumber(line_number);
}
|Stmt
{
    ($$) = new BlockItem_Stmt($1);
    ($$)->SetLineNumber(line_number);
}
;


//stmt的还没有完善 2211044 TODO!!
Stmt
:Lval '=' Exp ';'
{
    $$ = new assign_stmt($1,$3);
    $$ ->SetLineNumber(line_number);
}
|Lval ADDASSIGN Exp ';' 
{
    Lval* l;
    auto pd = ((Lval*)$1)->dims;
    if(pd != nullptr){
        auto vd = new std::vector<Expression>(*pd);
        l = new Lval(((Lval*)$1)->name,vd);
    }else{
        l = new Lval(((Lval*)$1)->name,nullptr);
    }
    auto n = new AddExp_plus(l,$3);
    $$ = new assign_stmt($1,n);
    $$->SetLineNumber(line_number);
}
|Lval SUBASSIGN Exp ';' 
{
    Lval* l;
    auto pd = ((Lval*)$1)->dims;
    if(pd != nullptr){
        auto vd = new std::vector<Expression>(*pd);
        l = new Lval(((Lval*)$1)->name,vd);
    }else{
        l = new Lval(((Lval*)$1)->name,nullptr);
    }
    auto n = new AddExp_sub(l,$3);
    $$ = new assign_stmt($1,n);
    $$->SetLineNumber(line_number);
}
|Lval MULASSIGN Exp ';' 
{
    Lval* l;
    auto pd = ((Lval*)$1)->dims;
    if(pd != nullptr){
        auto vd = new std::vector<Expression>(*pd);
        l = new Lval(((Lval*)$1)->name,vd);
    }else{
        l = new Lval(((Lval*)$1)->name,nullptr);
    }
    auto n = new MulExp_mul(l,$3);
    $$ = new assign_stmt($1,n);
    $$->SetLineNumber(line_number);
}
|Lval DIVASSIGN Exp ';' 
{
    Lval* l;
    auto pd = ((Lval*)$1)->dims;
    if(pd != nullptr){
        auto vd = new std::vector<Expression>(*pd);
        l = new Lval(((Lval*)$1)->name,vd);
    }else{
        l = new Lval(((Lval*)$1)->name,nullptr);
    }
    auto n = new MulExp_div(l,$3);
    $$ = new assign_stmt($1,n);
    $$->SetLineNumber(line_number);
}
|Lval MODASSIGN Exp ';' 
{
    Lval* l;
    auto pd = ((Lval*)$1)->dims;
    if(pd != nullptr){
        auto vd = new std::vector<Expression>(*pd);
        l = new Lval(((Lval*)$1)->name,vd);
    }else{
        l = new Lval(((Lval*)$1)->name,nullptr);
    }
    auto n = new MulExp_mod(l,$3);
    $$ = new assign_stmt($1,n);
    $$->SetLineNumber(line_number);
}
|Exp ';'
{
    $$ = new expr_stmt($1);
    $$->SetLineNumber(line_number);
}
|';'
{
    $$ = new null_stmt();
    $$->SetLineNumber(line_number);
}
|Block
{
    $$ = new block_stmt($1);
    $$->SetLineNumber(line_number);
}
|IF '(' Cond ')' Stmt %prec THEN
{
    $$ = new if_stmt($3,$5);
    $$->SetLineNumber(line_number);
}
|IF '(' Cond ')' Stmt ELSE Stmt
{
    $$ = new ifelse_stmt($3,$5,$7);
    $$->SetLineNumber(line_number);
}
|WHILE '(' Cond ')' Stmt
{
    $$ = new while_stmt($3,$5);
    $$->SetLineNumber(line_number);
}
|BREAK ';'
{
    $$ = new break_stmt();
    $$->SetLineNumber(line_number);
}
|CONTINUE ';'
{
    $$ = new continue_stmt();
    $$->SetLineNumber(line_number);
}
|RETURN Exp ';'
{
    $$ = new return_stmt($2);
    $$->SetLineNumber(line_number);
}
|RETURN ';'
{
    $$ = new return_stmt_void();
    $$->SetLineNumber(line_number);
}
;


Exp
:AddExp
{
    $$ = $1; 
    $$->SetLineNumber(line_number);
}
;

Cond
:LOrExp
{
    $$ = $1; 
    $$->SetLineNumber(line_number);
}
;

Lval
:IDENT
{
    $$ = new Lval($1,nullptr);
    $$->SetLineNumber(line_number);
}
|IDENT Array_Dim_Number_list
{
    $$ = new Lval($1,$2);
    $$->SetLineNumber(line_number);
}
;

PrimaryExp
:'(' Exp ')'
{
    ($$) = new PrimaryExp_branch($2);
    ($$)->SetLineNumber(line_number);
}
|Lval
{
    ($$) = ($1); 
    ($$)->SetLineNumber(line_number);
}
|IntConst
{
    ($$) = ($1); 
    ($$)->SetLineNumber(line_number);
}
|FloatConst
{
    ($$) = ($1); 
    ($$)->SetLineNumber(line_number);
}
|StringConst
{
    $$ = $1; 
    $$->SetLineNumber(line_number);
}
;

IntConst
:INT_CONST
{
    $$ = new IntConst($1);
    $$->SetLineNumber(line_number);
}
;

FloatConst
:FLOAT_CONST
{
    $$ = new FloatConst($1);
    $$->SetLineNumber(line_number);
}
;

StringConst
:STR_CONST
{
    $$ = new StringConst($1);
    $$->SetLineNumber(line_number);
}
;

UnaryExp
:PrimaryExp{$$ = $1;}
|IDENT '(' FuncRParams ')'
{
    $$ = new Func_call($1,$3);
    $$->SetLineNumber(line_number);
}
|IDENT '(' ')'
{
    // 在sylib.h这个文件中,starttime()是一个宏定义
    // #define starttime() _sysy_starttime(__LINE__)
    // 我们在语法分析中将其替换为_sysy_starttime(line_number)
    // stoptime同理
    if($1->get_string() == "starttime")
    {
        auto params = new std::vector<Expression>;
        params->push_back(new IntConst(line_number));
        Expression temp = new FuncRParams(params);
        $$ = new Func_call(id_table.add_id("_sysy_starttime"),temp);
        $$->SetLineNumber(line_number);
    }
    else if($1->get_string() == "stoptime")
    {
        auto params = new std::vector<Expression>;
        params->push_back(new IntConst(line_number));
        Expression temp = new FuncRParams(params);
        $$ = new Func_call(id_table.add_id("_sysy_stoptime"),temp);
        $$->SetLineNumber(line_number);
    }
    else
    {
        $$ = new Func_call($1,nullptr);
        $$->SetLineNumber(line_number);
    }
}
|'+' UnaryExp
{
    $$ = new UnaryExp_plus($2);
    $$->SetLineNumber(line_number);
}
|'-' UnaryExp
{
    $$ = new UnaryExp_neg($2);
    $$->SetLineNumber(line_number);
}
|'!' UnaryExp
{
    $$ = new UnaryExp_not($2);
    $$->SetLineNumber(line_number);
}
;

//2212514：函数实参列表其实就是表达式列表
FuncRParams
:Exp_list
{
    $$ = new FuncRParams($1);
    $$->SetLineNumber(line_number);
}
;

//2212514:表达式列表
Exp_list
:Exp
{ 
    $$ = new std::vector<Expression>;
    ($$)->push_back($1);
}
|Exp_list ',' Exp
{
    ($1)->push_back($3);
    ($$) = ($1);
}
;

//乘表达式-一元运算或同级运算，右递归
MulExp
: UnaryExp 
{
    ($$) = ($1);
    ($$)->SetLineNumber(line_number);
}
| MulExp '*' UnaryExp 
{
    $$ = new MulExp_mul($1, $3);
    $$->SetLineNumber(line_number);
}
| MulExp '/' UnaryExp 
{
    $$ = new MulExp_div($1, $3);
    $$->SetLineNumber(line_number);
}
//补充mod的情况 2211044
| MulExp '%' UnaryExp 
{
    $$ = new MulExp_mod($1, $3);
    $$->SetLineNumber(line_number);
}
;

AddExp
:MulExp
{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
|AddExp '+' MulExp
{
    $$ = new AddExp_plus($1,$3); 
    $$->SetLineNumber(line_number);
}
|AddExp '-' MulExp
{
    $$ = new AddExp_sub($1,$3); 
    $$->SetLineNumber(line_number);
}
;

//真值表达式
RelExp
:AddExp
{
    $$ = $1;
    $$->SetLineNumber(line_number);  
}
|RelExp '<' AddExp
{
    $$ = new RelExp_lt($1,$3);
    $$->SetLineNumber(line_number);
}
|RelExp '>' AddExp
{
    $$ = new RelExp_gt($1,$3);
    $$->SetLineNumber(line_number);
}
|RelExp LEQ AddExp
{
    $$ = new RelExp_leq($1,$3);
    $$->SetLineNumber(line_number);
}
|RelExp GEQ AddExp
{
    $$ = new RelExp_geq($1,$3);
    $$->SetLineNumber(line_number);
}
;

EqExp
:RelExp 
{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
|EqExp EQ RelExp 
{
    $$ = new EqExp_eq($1, $3);
    $$->SetLineNumber(line_number);
}
|EqExp NE RelExp 
{
    $$ = new EqExp_neq($1, $3);
    $$->SetLineNumber(line_number);
}
;

//与
LAndExp
:EqExp 
{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
|LAndExp AND LOrExp 
{
    $$ = new LAndExp_and($1, $3);
    $$->SetLineNumber(line_number);
}
;

//或
LOrExp
:LAndExp 
{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
|LOrExp OR LAndExp 
{
    $$ = new LOrExp_or($1, $3);
    $$->SetLineNumber(line_number);
}
;


ConstExp
:AddExp
{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
;

Array_Dim_Number
:'[' Exp ']'
{
    $$ = $2;
    $$->SetLineNumber(line_number);
}
;

Array_Dim_Number_list
:Array_Dim_Number
{
    $$ = new std::vector<Expression>;
    ($$)->push_back($1);
}
|Array_Dim_Number_list Array_Dim_Number
{
    ($1)->push_back($2);
    $$ = $1;
}
;

ConstArray_Dim_Number
:'[' ConstExp ']'
{
    
    $$ = $2;
    $$->SetLineNumber(line_number);
}
;

ConstArray_Dim_Number_list
:ConstArray_Dim_Number
{
    $$ = new std::vector<Expression>;
    ($$)->push_back($1);
}
|ConstArray_Dim_Number_list ConstArray_Dim_Number
{
    ($1)->push_back($2);
    $$ = $1;
}
;


// TODO: 也许你需要添加更多的非终结符
//+=那些？  ->这个在stmt里实现了
%% 

void yyerror(char* s, ...)
{
    ++error_num;
    fout<<"parser error in line "<<line_number<<"\n";
}
