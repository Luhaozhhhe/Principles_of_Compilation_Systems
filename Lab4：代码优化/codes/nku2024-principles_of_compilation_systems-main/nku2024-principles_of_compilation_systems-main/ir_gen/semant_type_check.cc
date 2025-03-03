#include "../include/type.h"
#include "../include/Instruction.h"
#include "../include/basic_block.h"
#include "../include/tree.h"

extern std::vector<std::string> error_msgs;//错误抛出


/*
-------------------------------------------------------------
|                         2211044：int的双目运算处理                             |
-------------------------------------------------------------
*/

//处理双目的加法运算
NodeAttribute Binary_Add_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute add_result;
    add_result.T.type = Type::INT;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        add_result.V.ConstTag = true;
    }
    else{
        add_result.V.ConstTag = false;
    }

    if (add_result.V.ConstTag == true) {
        add_result.V.val.IntVal = a.V.val.IntVal + b.V.val.IntVal;
    }
    return add_result;
}

//处理双目的减法运算
NodeAttribute Binary_Sub_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute sub_result;
    sub_result.T.type = Type::INT;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        sub_result.V.ConstTag = true;
    }
    else{
        sub_result.V.ConstTag = false;
    }

    if (sub_result.V.ConstTag == true) {
        sub_result.V.val.IntVal = a.V.val.IntVal - b.V.val.IntVal;
    }
    return sub_result;
}

//处理双目的乘法运算
NodeAttribute Binary_Mul_Int(NodeAttribute a, NodeAttribute b){
    NodeAttribute mul_result;
    mul_result.T.type = Type::INT;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        mul_result.V.ConstTag = true;
    }
    else{
        mul_result.V.ConstTag = false;
    }

    if(mul_result.V.ConstTag == true){
        mul_result.V.val.IntVal = a.V.val.IntVal * b.V.val.IntVal;
    }
    return mul_result;
}

//处理双目的除法运算，考虑除以0的情况
NodeAttribute Binary_Div_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute div_result;
    div_result.T.type = Type::INT;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        div_result.V.ConstTag = true;
    }
    else{
        div_result.V.ConstTag = false;
    }
    //如果被除数是0的话，就抛出除0错误
    if((b.V.ConstTag==true && b.V.val.IntVal == 0)){
        div_result.T.type = Type::VOID;
        div_result.V.ConstTag = false;

        error_msgs.push_back("cannot div zero!!!Error in line "   + std::to_string(b.line_number) 
        +" a.Attr:"+ a.GetAttributeInfo() +" b.Attr:"+ b.GetAttributeInfo()+ "\n");
    }

    if(div_result.V.ConstTag == true){
        div_result.V.val.IntVal = a.V.val.IntVal / b.V.val.IntVal;
    }
    return div_result;
}

//处理双目的取模运算，考虑mod 0的情况
NodeAttribute Binary_Mod_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute mod_result;
    mod_result.T.type = Type::INT;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        mod_result.V.ConstTag = true;
    }
    else{
        mod_result.V.ConstTag = false;
    }

    //如果mod后面是0的话，就抛出模0错误
    if(mod_result.V.ConstTag == true && b.V.val.IntVal == 0){
        mod_result.T.type = Type::VOID;
        mod_result.V.ConstTag = false;
        error_msgs.push_back("cannot mod zero!!!Error in line " + std::to_string(a.line_number) + "\n");
    }

    if(mod_result.V.ConstTag == true){
        mod_result.V.val.IntVal = a.V.val.IntVal % b.V.val.IntVal;
    }
    return mod_result;
}

//处理双目的大于等于运算
NodeAttribute Binary_Geq_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute geq_result;
    geq_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        geq_result.V.ConstTag = true;
    }
    else{
        geq_result.V.ConstTag = false;
    }

    if(geq_result.V.ConstTag == true){
        if(a.V.val.IntVal >= b.V.val.IntVal){
            geq_result.V.val.BoolVal = true;
        }
        else if(a.V.val.IntVal < b.V.val.IntVal){
            geq_result.V.val.BoolVal = false;
        }

    }
    return geq_result;
}

//处理双目的大于运算
NodeAttribute Binary_Gt_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute gt_result;
    gt_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        gt_result.V.ConstTag = true;
    }
    else{
        gt_result.V.ConstTag = false;
    }

    if(gt_result.V.ConstTag == true){
        if(a.V.val.IntVal > b.V.val.IntVal){
            gt_result.V.val.BoolVal = true;
        }
        else if(a.V.val.IntVal <= b.V.val.IntVal){
            gt_result.V.val.BoolVal = false;
        }

    }
    return gt_result;
}

//处理双目的小于等于运算
NodeAttribute Binary_Leq_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute leq_result;
    leq_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        leq_result.V.ConstTag = true;
    }
    else{
        leq_result.V.ConstTag = false;
    }

    if(leq_result.V.ConstTag == true){
        if(a.V.val.IntVal <= b.V.val.IntVal){
            leq_result.V.val.BoolVal = true;
        }
        else if(a.V.val.IntVal > b.V.val.IntVal){
            leq_result.V.val.BoolVal = false;
        }

    }
    return leq_result;
}

//处理双目的小于运算
NodeAttribute Binary_Lt_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute lt_result;
    lt_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        lt_result.V.ConstTag = true;
    }
    else{
        lt_result.V.ConstTag = false;
    }

    if(lt_result.V.ConstTag == true){
        if(a.V.val.IntVal < b.V.val.IntVal){
            lt_result.V.val.BoolVal = true;
        }
        else if(a.V.val.IntVal >= b.V.val.IntVal){
            lt_result.V.val.BoolVal = false;
        }

    }
    return lt_result;
}

//处理双目的等于运算
NodeAttribute Binary_Eq_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute eq_result;
    eq_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        eq_result.V.ConstTag = true;
    }
    else{
        eq_result.V.ConstTag = false;
    }

    if(eq_result.V.ConstTag == true){
        if(a.V.val.IntVal == b.V.val.IntVal){
            eq_result.V.val.BoolVal = true;
        }
        else if(a.V.val.IntVal != b.V.val.IntVal){
            eq_result.V.val.BoolVal = false;
        }

    }
    return eq_result;
}

//处理双目的或运算
NodeAttribute Binary_Ne_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute ne_result;
    ne_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        ne_result.V.ConstTag = true;
    }
    else{
        ne_result.V.ConstTag = false;
    }

    if(ne_result.V.ConstTag != true){
        if(a.V.val.IntVal > b.V.val.IntVal){
            ne_result.V.val.BoolVal = true;
        }
        else if(a.V.val.IntVal == b.V.val.IntVal){
            ne_result.V.val.BoolVal = false;
        }

    }
    return ne_result;
}

//处理双目的或运算
NodeAttribute Binary_Or_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute or_result;
    or_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        or_result.V.ConstTag = true;
    }
    else{
        or_result.V.ConstTag = false;
    }

    if(or_result.V.ConstTag != true){
        if(a.V.val.IntVal > b.V.val.IntVal){
            or_result.V.val.BoolVal = true;
        }
        else if(a.V.val.IntVal == b.V.val.IntVal){
            or_result.V.val.BoolVal = false;
        }

    }
    return or_result;
}

//处理双目的与运算
NodeAttribute Binary_And_Int(NodeAttribute a, NodeAttribute b) {
    NodeAttribute and_result;
    and_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        and_result.V.ConstTag = true;
    }
    else{
        and_result.V.ConstTag = false;
    }

    if(and_result.V.ConstTag != true){
        if(a.V.val.IntVal > b.V.val.IntVal){
            and_result.V.val.BoolVal = true;
        }
        else if(a.V.val.IntVal == b.V.val.IntVal){
            and_result.V.val.BoolVal = false;
        }

    }
    return and_result;
}


/*
-------------------------------------------------------------
|                         2211044：float的双目运算处理                             |
-------------------------------------------------------------
*/

//处理双目的加法运算
NodeAttribute Binary_Add_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute add_result;
    add_result.T.type = Type::FLOAT;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        add_result.V.ConstTag = true;
    }
    else{
        add_result.V.ConstTag = false;
    }

    if (add_result.V.ConstTag == true) {
        add_result.V.val.FloatVal = a.V.val.FloatVal + b.V.val.FloatVal;
    }
    return add_result;
}

//处理双目的减法运算
NodeAttribute Binary_Sub_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute sub_result;
    sub_result.T.type = Type::FLOAT;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        sub_result.V.ConstTag = true;
    }
    else{
        sub_result.V.ConstTag = false;
    }

    if (sub_result.V.ConstTag == true) {
        sub_result.V.val.FloatVal = a.V.val.FloatVal - b.V.val.FloatVal;
    }
    return sub_result;
}

//处理双目的乘法运算
NodeAttribute Binary_Mul_Float(NodeAttribute a, NodeAttribute b){
    NodeAttribute mul_result;
    mul_result.T.type = Type::FLOAT;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        mul_result.V.ConstTag = true;
    }
    else{
        mul_result.V.ConstTag = false;
    }

    if(mul_result.V.ConstTag == true){
        mul_result.V.val.FloatVal = a.V.val.FloatVal * b.V.val.FloatVal;
    }
    return mul_result;
}

//处理双目的除法运算，考虑除以0的情况
NodeAttribute Binary_Div_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute div_result;
    div_result.T.type = Type::FLOAT;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        div_result.V.ConstTag = true;
    }
    else{
        div_result.V.ConstTag = false;
    }
    //如果被除数是0的话，就抛出除0错误
    if(b.V.ConstTag&&b.V.val.FloatVal == 0){
        div_result.T.type = Type::VOID;
        div_result.V.ConstTag = false;
        error_msgs.push_back("cannot div zero!!!Error in line " + std::to_string(a.line_number) + "\n");
    }

    if(div_result.V.ConstTag == true){
        div_result.V.val.FloatVal = a.V.val.FloatVal / b.V.val.FloatVal;
    }
    return div_result;
}

//处理双目的取模运算，考虑mod 0的情况
NodeAttribute Binary_Mod_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute mod_result;
    mod_result.T.type = Type::VOID;

    //mod的浮点运算不合法，直接抛出错误
    mod_result.V.ConstTag = false;
    error_msgs.push_back("cannot mod float!!!Error in line " + std::to_string(a.line_number) + "\n");

    return mod_result;
}

//处理双目的大于等于运算
NodeAttribute Binary_Geq_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute geq_result;
    geq_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        geq_result.V.ConstTag = true;
    }
    else{
        geq_result.V.ConstTag = false;
    }

    if(geq_result.V.ConstTag == true){
        if(a.V.val.FloatVal >= b.V.val.FloatVal){
            geq_result.V.val.BoolVal = true;
        }
        else if(a.V.val.FloatVal < b.V.val.FloatVal){
            geq_result.V.val.BoolVal = false;
        }

    }
    return geq_result;
}

//处理双目的大于运算
NodeAttribute Binary_Gt_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute gt_result;
    gt_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        gt_result.V.ConstTag = true;
    }
    else{
        gt_result.V.ConstTag = false;
    }

    if(gt_result.V.ConstTag == true){
        if(a.V.val.FloatVal > b.V.val.FloatVal){
            gt_result.V.val.BoolVal = true;
        }
        else if(a.V.val.FloatVal <= b.V.val.FloatVal){
            gt_result.V.val.BoolVal = false;
        }

    }
    return gt_result;
}

//处理双目的小于等于运算
NodeAttribute Binary_Leq_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute leq_result;
    leq_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        leq_result.V.ConstTag = true;
    }
    else{
        leq_result.V.ConstTag = false;
    }

    if(leq_result.V.ConstTag == true){
        if(a.V.val.FloatVal <= b.V.val.FloatVal){
            leq_result.V.val.BoolVal = true;
        }
        else if(a.V.val.FloatVal > b.V.val.FloatVal){
            leq_result.V.val.BoolVal = false;
        }

    }
    return leq_result;
}

//处理双目的小于运算
NodeAttribute Binary_Lt_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute lt_result;
    lt_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        lt_result.V.ConstTag = true;
    }
    else{
        lt_result.V.ConstTag = false;
    }

    if(lt_result.V.ConstTag == true){
        if(a.V.val.FloatVal < b.V.val.FloatVal){
            lt_result.V.val.BoolVal = true;
        }
        else if(a.V.val.FloatVal >= b.V.val.FloatVal){
            lt_result.V.val.BoolVal = false;
        }

    }
    return lt_result;
}

//处理双目的等于运算
NodeAttribute Binary_Eq_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute eq_result;
    eq_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        eq_result.V.ConstTag = true;
    }
    else{
        eq_result.V.ConstTag = false;
    }

    if(eq_result.V.ConstTag == true){
        if(a.V.val.FloatVal == b.V.val.FloatVal){
            eq_result.V.val.BoolVal = true;
        }
        else if(a.V.val.FloatVal != b.V.val.FloatVal){
            eq_result.V.val.BoolVal = false;
        }

    }
    return eq_result;
}

//处理双目的或运算
NodeAttribute Binary_Ne_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute ne_result;
    ne_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        ne_result.V.ConstTag = true;
    }
    else{
        ne_result.V.ConstTag = false;
    }

    if(ne_result.V.ConstTag != true){
        if(a.V.val.FloatVal > b.V.val.FloatVal){
            ne_result.V.val.BoolVal = true;
        }
        else if(a.V.val.FloatVal == b.V.val.FloatVal){
            ne_result.V.val.BoolVal = false;
        }

    }
    return ne_result;
}

//处理双目的或运算
NodeAttribute Binary_Or_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute or_result;
    or_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        or_result.V.ConstTag = true;
    }
    else{
        or_result.V.ConstTag = false;
    }

    if(or_result.V.ConstTag != true){
        if(a.V.val.FloatVal > b.V.val.FloatVal){
            or_result.V.val.BoolVal = true;
        }
        else if(a.V.val.FloatVal == b.V.val.FloatVal){
            or_result.V.val.BoolVal = false;
        }

    }
    return or_result;
}

//处理双目的与运算
NodeAttribute Binary_And_Float(NodeAttribute a, NodeAttribute b) {
    NodeAttribute and_result;
    and_result.T.type = Type::BOOL;

    //判断父节点的类型，如果两个子节点都是常量的话就给上面的父节点赋值为常量
    if(a.V.ConstTag == true && b.V.ConstTag == true){
        and_result.V.ConstTag = true;
    }
    else{
        and_result.V.ConstTag = false;
    }

    if(and_result.V.ConstTag != true){
        if(a.V.val.FloatVal > b.V.val.FloatVal){
            and_result.V.val.BoolVal = true;
        }
        else if(a.V.val.FloatVal == b.V.val.FloatVal){
            and_result.V.val.BoolVal = false;
        }

    }
    return and_result;
}


/*
-------------------------------------------------------------
|                        :2211044：编写调用函数,来完成对前面写的float和int的调用                             |
-------------------------------------------------------------
*/

//处理int之间的运算
NodeAttribute Calculate_Binary_Int(NodeAttribute::opcode op, NodeAttribute a, NodeAttribute b) {
    switch (op) {
        case NodeAttribute::ADD:
            return Binary_Add_Int(a, b);
        case NodeAttribute::SUB:
            return Binary_Sub_Int(a, b);
        case NodeAttribute::MUL:
            return Binary_Mul_Int(a, b);
        case NodeAttribute::DIV:
            return Binary_Div_Int(a, b);
        case NodeAttribute::MOD:
            return Binary_Mod_Int(a, b);
        case NodeAttribute::GEQ:
            return Binary_Geq_Int(a, b);
        case NodeAttribute::GT:
            return Binary_Gt_Int(a, b);
        case NodeAttribute::LEQ:
            return Binary_Leq_Int(a, b);
        case NodeAttribute::LT:
            return Binary_Lt_Int(a, b);
        case NodeAttribute::EQ:
            return Binary_Eq_Int(a, b);
        case NodeAttribute::NE:
            return Binary_Ne_Int(a, b);
        case NodeAttribute::OR:
            return Binary_Or_Int(a, b);
        case NodeAttribute::AND:
            return Binary_And_Int(a, b);
        default:
            NodeAttribute error_result;
            error_result.T.type = Type::VOID;
            error_result.V.ConstTag = false;
            error_msgs.push_back("the opcode in the Binary Int Calculate is wrong!!\n");
            return error_result;
    }
}

//处理float之间的运算
NodeAttribute Calculate_Binary_Float(NodeAttribute::opcode op, NodeAttribute a, NodeAttribute b) {
    switch (op) {
        case NodeAttribute::ADD:
            return Binary_Add_Float(a, b);
        case NodeAttribute::SUB:
            return Binary_Sub_Float(a, b);
        case NodeAttribute::MUL:
            return Binary_Mul_Float(a, b);
        case NodeAttribute::DIV:
            return Binary_Div_Float(a, b);
        case NodeAttribute::MOD:
            return Binary_Mod_Float(a, b);
        case NodeAttribute::GEQ:
            return Binary_Geq_Float(a, b);
        case NodeAttribute::GT:
            return Binary_Gt_Float(a, b);
        case NodeAttribute::LEQ:
            return Binary_Leq_Float(a, b);
        case NodeAttribute::LT:
            return Binary_Lt_Float(a, b);
        case NodeAttribute::EQ:
            return Binary_Eq_Float(a, b);
        case NodeAttribute::NE:
            return Binary_Ne_Float(a, b);
        case NodeAttribute::OR:
            return Binary_Or_Float(a, b);
        case NodeAttribute::AND:
            return Binary_And_Float(a, b);
        default:
            NodeAttribute error_result;
            error_result.T.type = Type::VOID;
            error_result.V.ConstTag = false;
            error_msgs.push_back("the opcode in the Binary Float Calculate is wrong!!!\n");
            return error_result;
    }
}



/*
-------------------------------------------------------------
|                        :2211044：处理双目属性之间的运算,完成隐式转换                             |
-------------------------------------------------------------
*/

NodeAttribute Semant_Int_Int(NodeAttribute a,NodeAttribute b,NodeAttribute::opcode opcode){
    return Calculate_Binary_Int(opcode , a , b);//如果是两个int型运算的话，就不需要进行转换

}

NodeAttribute Semant_Int_Float(NodeAttribute a,NodeAttribute b,NodeAttribute::opcode opcode){
    NodeAttribute temp = a;//int和float运算的话，将int转化为float即可
    a.T.type = Type::FLOAT;
    temp.V.val.FloatVal = (float)temp.V.val.IntVal;
    return Calculate_Binary_Float(opcode , temp , b);
}

NodeAttribute Semant_Int_Bool(NodeAttribute a,NodeAttribute b,NodeAttribute::opcode opcode){
    NodeAttribute temp = b;
    b.T.type = Type::INT;
    temp.V.val.IntVal = temp.V.val.BoolVal;
    return Calculate_Binary_Int(opcode , a , temp);
}

NodeAttribute Semant_Float_Int(NodeAttribute a,NodeAttribute b,NodeAttribute::opcode opcode){
    NodeAttribute temp = b;
    temp.T.type = Type::FLOAT;
    temp.V.val.FloatVal = (float)temp.V.val.IntVal;
    return Calculate_Binary_Float(opcode, a , temp);
}

NodeAttribute Semant_Float_Float(NodeAttribute a,NodeAttribute b,NodeAttribute::opcode opcode){
    return Calculate_Binary_Float(opcode , a , b);
}

NodeAttribute Semant_Float_Bool(NodeAttribute a,NodeAttribute b,NodeAttribute::opcode opcode){
    NodeAttribute temp = b;
    temp.T.type = Type::FLOAT;
    temp.V.val.FloatVal = temp.V.val.BoolVal;
    return Calculate_Binary_Float(opcode , a , temp);
}

NodeAttribute Semant_Bool_Int(NodeAttribute a,NodeAttribute b,NodeAttribute::opcode opcode){
    NodeAttribute temp = a;
    temp.T.type = Type::INT;
    temp.V.val.IntVal = temp.V.val.BoolVal;
    return Calculate_Binary_Int(opcode , a , temp);
}

NodeAttribute Semant_Bool_Float(NodeAttribute a,NodeAttribute b,NodeAttribute::opcode opcode){
    NodeAttribute temp = a;
    temp.T.type = Type::FLOAT;
    temp.V.val.FloatVal = temp.V.val.BoolVal;
    return Calculate_Binary_Float(opcode, a , temp);
}

NodeAttribute Semant_Bool_Bool(NodeAttribute a,NodeAttribute b,NodeAttribute::opcode opcode){
    NodeAttribute temp_a = a, temp_b = b;
    temp_a.T.type = Type::INT;
    temp_b.T.type = Type::INT;
    temp_a.V.val.IntVal = temp_a.V.val.BoolVal;
    temp_b.V.val.IntVal = temp_b.V.val.BoolVal;
    return Calculate_Binary_Int(opcode , temp_a, temp_b);
}

NodeAttribute Semant_ERROR(NodeAttribute a,NodeAttribute b,NodeAttribute::opcode opcode){
    error_msgs.push_back("invalid operators in line " + std::to_string(a.line_number) + "\n");
    NodeAttribute error_result;
    error_result.T.type = Type::VOID;
    error_result.V.ConstTag = false;
    return error_result;
}


/*
-------------------------------------------------------------
|                        2211044：处理单目的三个运算符：+，-，！                             |
-------------------------------------------------------------
*/

NodeAttribute Single_Add_Int(NodeAttribute a){
    NodeAttribute add_result;
    add_result.T.type = Type::INT;
    add_result.V.ConstTag = a.V.ConstTag;

    if(add_result.V.ConstTag == true){
        add_result.V.val.IntVal = a.V.val.IntVal;
    }
    return add_result;
    
}


NodeAttribute Single_Sub_Int(NodeAttribute a){
    NodeAttribute sub_result;
    sub_result.T.type = Type::INT;
    sub_result.V.ConstTag = a.V.ConstTag;

    if(sub_result.V.ConstTag == true){
        sub_result.V.val.IntVal = -a.V.val.IntVal;
    }
    return sub_result;
    
}

NodeAttribute Single_Not_Int(NodeAttribute a){
    NodeAttribute not_result;
    not_result.T.type = Type::BOOL;
    not_result.V.ConstTag = a.V.ConstTag;

    if(not_result.V.ConstTag == true){
        not_result.V.val.BoolVal = !a.V.val.IntVal;
    }
    return not_result;
    
}

NodeAttribute Single_Add_Float(NodeAttribute a){
    NodeAttribute add_result;
    add_result.T.type = Type::FLOAT;
    add_result.V.ConstTag = a.V.ConstTag;

    if(add_result.V.ConstTag == true){
        add_result.V.val.FloatVal = a.V.val.FloatVal;
    }
    return add_result;
    
}


NodeAttribute Single_Sub_Float(NodeAttribute a){
    NodeAttribute sub_result;
    sub_result.T.type = Type::FLOAT;
    sub_result.V.ConstTag = a.V.ConstTag;

    if(sub_result.V.ConstTag == true){
        sub_result.V.val.FloatVal = -a.V.val.FloatVal;
    }
    return sub_result;
    
}

NodeAttribute Single_Not_Float(NodeAttribute a){
    NodeAttribute not_result;
    not_result.T.type = Type::BOOL;
    not_result.V.ConstTag = a.V.ConstTag;

    if(not_result.V.ConstTag == true){
        not_result.V.val.BoolVal = !a.V.val.FloatVal;
    }
    return not_result;
    
}


/*
-------------------------------------------------------------
|                         2211044：单目的运算调用函数，处理类型转换                             |
-------------------------------------------------------------
*/

NodeAttribute Calculate_Single_Int(NodeAttribute::opcode op, NodeAttribute a) {
    switch (op) {
        case NodeAttribute::ADD:
            return Single_Add_Int(a);
        case NodeAttribute::SUB:
            return Single_Sub_Int(a);
        case NodeAttribute::NOT:
            return Single_Not_Int(a);
        default:
            NodeAttribute error_result;
            error_result.T.type = Type::VOID;
            error_result.V.ConstTag = 0;
            error_msgs.push_back("the opcode in the Single Int Calculate is wrong!!!\n");
            return error_result;
    }
}

NodeAttribute Calculate_Single_Float(NodeAttribute::opcode op, NodeAttribute a) {
    switch (op) {
        case NodeAttribute::ADD:
            return Single_Add_Float(a);
        case NodeAttribute::SUB:
            return Single_Sub_Float(a);
        case NodeAttribute::NOT:
            return Single_Not_Float(a);
        default:
            NodeAttribute result;
            result.T.type = Type::VOID;
            result.V.ConstTag = 0;
            error_msgs.push_back("the opcode in the Single Float Calculate is wrong!!!\n");
            return result;
    }
}




/*
-------------------------------------------------------------
|                         2211044：处理单目属性之间的运算,完成隐式转换                             |
-------------------------------------------------------------
*/

NodeAttribute Semant_Int(NodeAttribute a, NodeAttribute::opcode opcode){
    return Calculate_Single_Int(opcode , a);

}

NodeAttribute Semant_Float(NodeAttribute a, NodeAttribute::opcode opcode){
    return Calculate_Single_Float(opcode , a);

}

NodeAttribute Semant_Bool(NodeAttribute a, NodeAttribute::opcode opcode) {
    //将bool类型的单目值转化为int类型的就可以了
    NodeAttribute temp = a;
    //temp.T.type = Type::INT;
    temp.V.val.IntVal = a.V.val.BoolVal;
    return Calculate_Single_Int(opcode , temp);

}

NodeAttribute Semant_ERROR(NodeAttribute a, NodeAttribute::opcode opcode) {
    error_msgs.push_back("invalid operators in line " + std::to_string(a.line_number) + "\n");
    NodeAttribute error_result;
    error_result.T.type = Type::VOID;
    error_result.V.ConstTag = false;
    return error_result;

}


/*
-------------------------------------------------------------
|                         2211044：处理semant中的二元运算                             |
-------------------------------------------------------------
*/

NodeAttribute Perform_Binary_Operation(NodeAttribute a, NodeAttribute b, NodeAttribute::opcode opcode) {
    NodeAttribute result;

    if (a.T.type == Type::INT && b.T.type == Type::INT) {
        result = Semant_Int_Int(a, b, opcode);
    }

    else if (a.T.type == Type::INT && b.T.type == Type::FLOAT) {
        result = Semant_Int_Float(a, b, opcode);
    }

    else if (a.T.type == Type::INT && b.T.type == Type::BOOL) {
        result = Semant_Int_Bool(a, b, opcode);
    }

    else if (a.T.type == Type::FLOAT && b.T.type == Type::INT) {
        result = Semant_Float_Int(a, b, opcode);
    }

    else if (a.T.type == Type::FLOAT && b.T.type == Type::FLOAT) {
        result = Semant_Float_Float(a, b, opcode);
    }

    else if (a.T.type == Type::FLOAT && b.T.type == Type::BOOL) {
        result = Semant_Float_Bool(a, b, opcode);
    }

    else if (a.T.type == Type::BOOL && b.T.type == Type::INT) {
        result = Semant_Bool_Int(a, b, opcode);
    }

    else if (a.T.type == Type::BOOL && b.T.type == Type::FLOAT) {
        result = Semant_Bool_Float(a, b, opcode);
    }

    else if (a.T.type == Type::BOOL && b.T.type == Type::BOOL) {
        result = Semant_Bool_Bool(a, b, opcode);
    }

    else {
        result = Semant_ERROR(a, b, opcode);
    }

    return result;

}


NodeAttribute Perform_Single_Operation(NodeAttribute a, NodeAttribute::opcode opcode) {
    NodeAttribute result;

    if (a.T.type == Type::INT) {
        result = Semant_Int(a, opcode);
    }

    else if (a.T.type == Type::FLOAT) {
        result = Semant_Float(a, opcode);
    }
    
    else if (a.T.type == Type::BOOL) {
        result = Semant_Bool(a, opcode);
    }

    else {
        result = Semant_ERROR(a, opcode);
    }

    return result;
}
