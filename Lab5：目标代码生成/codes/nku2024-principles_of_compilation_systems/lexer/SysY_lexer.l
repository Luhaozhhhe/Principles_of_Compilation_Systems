%option noyywrap
%{
#include <cmath>
#include <vector>
#include <cstring>

#include <string>

#include "../include/symtab.h"
#include "../parser/SysY_parser.tab.h"
using namespace std;
extern int line_number;   //表示行号
extern int col_number;    //表示token结束位置
extern int cur_col_number;  //表示token开始的位置
extern IdTable id_table;    //符号表，暂时不实现
void yyerror(char* s, ...);

int HexChar2Int(char x)
{
   if(x>='0'&& x<='9')
   {
   	return int(x-'0');
   }
   if(x>='A'&& x<='F')
   {
   	return int(x-'A'+10);
   }
   if(x>='a'&& x<='f')
   {
   	return int(x-'a'+10);
   }
   return 0;
}

double cul_value_HexFloat(const char *yytext)
{
    string str(yytext);
    int p_pos=str.find('p');
    if (p_pos==-1) {  
        p_pos=str.find('P');
    }
    string hex_part = str.substr(0, p_pos);
    int exponent = stoi(str.substr(p_pos + 1));//only DecDigit
    double value=0.0;
    bool found_dot=0;
    double factor=1.0;
    for(int i=2;i<hex_part.length();i++)
    {
    	if(hex_part[i] == '.')
    	{
    	    found_dot=1;
    	    continue;
    	}
    	int hex_value=HexChar2Int(hex_part[i]);
    	if (found_dot)
    	{
    	    factor/=16.0;
    	    value+=hex_value*factor;
    	}
    	else
    	{
            value=value*16+hex_value;
    	}
    }
    value = value * pow(2, exponent);
    return value;
}

%}

DecConst ([1-9][0-9]*)|0
OcConst "0"[0-7]*
HexConst "0"[xX][0-9a-fA-F]+
FloatConst [0-9]*\.[0-9]*([eE][+-]?[0-9]+)?|\.[0-9]+([eE][+-]?[0-9]+)?|[0-9]+(\.[0-9]*([eE][+-]?[0-9]+)?|[eE][+-]?[0-9]+)
HexFloatConst 0[xX]([0-9a-fA-F]*\.[0-9a-fA-F]*|[0-9a-fA-F]+\.)[pP][+-]?[0-9]+




%x SINGLE_EXEGESIS
%x MULTI_EXEGESIS



%%


  // Reference: https://github.com/yuhuifishash/SysY/blob/master/lexer/SysY_lexer.l line27-line50

"//".* {  
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    BEGIN(SINGLE_EXEGESIS);
    }

<SINGLE_EXEGESIS>.* {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
}

<SINGLE_EXEGESIS>\n {
    line_number=line_number+1;
    col_number=0;
    BEGIN(INITIAL);
}


"/*" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    BEGIN(MULTI_EXEGESIS); 
}

"*/" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    yylval.error_msg="unmatched the /*,wrong"; 
    return ERROR;  
}

<MULTI_EXEGESIS>"*/" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    BEGIN(INITIAL);   //如果在注释状态下遇到*/，相当于和前面一个匹配上了，就转变到初始状态
}

<MULTI_EXEGESIS>\n {
    line_number=line_number+1;
    col_number=0;   //遇到注释中的换行符，我们就将行数加一，然后直接清零字符串所在行的位置
}

<MULTI_EXEGESIS><<EOF>> {
    BEGIN(INITIAL);
    yylval.error_msg = "EOF in the exegesis,wrong";
    return ERROR;
}

<MULTI_EXEGESIS>. {
    cur_col_number = col_number;
    col_number=col_number+strlen(yytext);
    }





"<=" {      //将前者的结束位置赋值给后者的开始位置，然后结束位置就是开始位置加上token的长度，在lexer_out.cc中有对应的定义
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return LEQ;
    }         

">=" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return GEQ;
    }

"==" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return EQ;
    }     

"!=" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return NE;
    }

"&&" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return AND;
    }   

"||" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return OR;
    }        

"+=" {
    cur_col_number = col_number;
    col_number=col_number+strlen(yytext);
    return ADDASSIGN;
    }

"-=" {
    cur_col_number = col_number;
    col_number=col_number+strlen(yytext);
    return SUBASSIGN;
    }       

"*=" {
    cur_col_number = col_number;
    col_number=col_number+strlen(yytext);
    return MULASSIGN;
    }   

"/=" {
    cur_col_number = col_number;
    col_number=col_number+strlen(yytext);
    return DIVASSIGN;
    }   

"%=" {
    cur_col_number = col_number;
    col_number=col_number+strlen(yytext);
    return MODASSIGN;
    }   


"const" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return CONST;
    }     

"if" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return IF;
    }       

"else" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return ELSE;
    }  

"while" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return WHILE;
    }

"void" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return NONE_TYPE;
    }    

"for" {
    cur_col_number = col_number;
    col_number=col_number+strlen(yytext);
    return FOR;
    }  

"int" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return INT;
    }    

"float" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return FLOAT;
    }      

"return" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return RETURN;
    }   

"break" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return BREAK;
    }     

"continue" {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return CONTINUE;
    }


\n {   //换行就是，line_number加一，然后将token结束的位置清零，因为换行了
    line_number=line_number+1;
    col_number = 0;
    }   


[ \t\f\r\v] {
    col_number += strlen(yytext);
    }

[\.\+\-\*\/\=\<\!\%\>] {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return yytext[0];
    }


[\{\}\;\(\)\,\[\]] {
    cur_col_number=col_number;
    col_number=col_number+strlen(yytext);
    return yytext[0];
    }


[_a-zA-Z][_a-zA-Z0-9]* {
    cur_col_number = col_number;
    col_number += strlen(yytext);
    yylval.symbol_token = id_table.add_id(std::string(yytext));
    return IDENT;
}



{DecConst} {
    cur_col_number = col_number;
    col_number = col_number + strlen(yytext);
    yylval.int_token = 0;

    for(int i=0;yytext[i];i++){
        if(yytext[i]>='0' && yytext[i]<='9'){
            yylval.int_token = yylval.int_token * 10;
            yylval.int_token = yylval.int_token + yytext[i]-'0';
        }
        else{
            return ERROR; //遇到异常输入直接报error
        }
    }

    if(yylval.int_token>INT32_MAX || yylval.int_token<INT32_MIN){
        return ERROR;
    }

    else{
        return INT_CONST;
    }

}

	
{OcConst} {
    cur_col_number = col_number;
    col_number = col_number + strlen(yytext);
    yylval.int_token = 0;

    for(int i=0;yytext[i];i++){
        if(yytext[i]>='0' && yytext[i]<='7'){
            yylval.int_token = yylval.int_token * 8;
            yylval.int_token = yylval.int_token + yytext[i]-'0';
        }
        else{
            yylval.error_msg = yytext;
            return ERROR; //遇到异常输入直接报error
        }
    }

    if(yylval.int_token>INT32_MAX || yylval.int_token<INT32_MIN){
        return ERROR;
    }

    else{
        return INT_CONST;
    }
}


{HexConst} {
    cur_col_number = col_number;
    col_number = col_number + strlen(yytext);
    yylval.int_token = 0;

    for(int i=2;yytext[i];i++){   //循环从第三个开始，省略了0x
        if((yytext[i]>='0' && yytext[i]<='9') || (yytext[i]>='a' && yytext[i]<='f') || (yytext[i]>='A' && yytext[i]<='F')){
            yylval.int_token = yylval.int_token * 16;
            yylval.int_token = yylval.int_token + HexChar2Int( yytext[i] );
        }

        else{
            yylval.error_msg = yytext;
            return ERROR; //遇到异常输入直接报error
        }
    }

    if(yylval.int_token>INT32_MAX || yylval.int_token<INT32_MIN){
        return ERROR;
    }

    else{
        return INT_CONST;
    }
}


{FloatConst}  {
    cur_col_number=col_number;
    col_number += strlen(yytext);
    yylval.float_token = stof(yytext);
    return FLOAT_CONST;
}

{HexFloatConst} {
    cur_col_number=col_number;
    col_number += strlen(yytext);
    yylval.float_token = cul_value_HexFloat(yytext);
    return FLOAT_CONST;
}


    /*unknown tokens, return ERROR*/
. {
    cur_col_number = col_number;
    col_number += strlen(yytext);
    yylval.error_msg = yytext;
    return ERROR;
}
%%



