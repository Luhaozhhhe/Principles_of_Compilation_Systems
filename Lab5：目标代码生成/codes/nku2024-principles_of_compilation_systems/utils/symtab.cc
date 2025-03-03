#include "../include/symtab.h"
#include <string>
Symbol IdTable::add_id(std::string s) {
    auto it = id_table.find(s);
    if (it == id_table.end()) {
        Symbol new_symbol = new Entry(s);
        id_table[s] = new_symbol;
        return new_symbol;
    } else {
        return id_table[s];
    }
}

void SymbolTable::add_Symbol(Symbol C, VarAttribute val) { symbol_table[current_scope][C] = val; }

Type::ty SymbolTable::lookup_type(Symbol C) {
    for (int i = current_scope; i >= 0; --i) {
        auto it = symbol_table[i].find(C);
        if (it != symbol_table[i].end()) {
            return it->second.type;
        }
    }
    return Type::VOID;
}

int SymbolTable::lookup_scope(Symbol C) {
    for (int i = current_scope; i >= 0; --i) {
        auto it = symbol_table[i].find(C);
        if (it != symbol_table[i].end()) {
            return i;
        }
    }
    return -1;
}

VarAttribute SymbolTable::lookup_val(Symbol C) {
    for (int i = current_scope; i >= 0; --i) {
        auto it = symbol_table[i].find(C);
        if (it != symbol_table[i].end()) {
            return it->second;
        }
    }
    return VarAttribute();
}

void SymbolTable::enter_scope() {
    ++current_scope;//作用域每进入一次就会++
    symbol_table.push_back(std::map<Symbol, VarAttribute>());
}

void SymbolTable::exit_scope() {//出作用域，--
    --current_scope;
    symbol_table.pop_back();
}

int SymbolRegTable::lookup(Symbol C) {
    for (int i = current_scope; i >= 0; --i) {
        auto it = symbol_table[i].find(C);
        if (it != symbol_table[i].end()) {
            return it->second;
        }
    }
    return -1;
}

void SymbolRegTable::add_Symbol(Symbol C, int val) { symbol_table[current_scope][C] = val; }

void SymbolRegTable::enter_scope() {
    ++current_scope;
    symbol_table.push_back(std::map<Symbol, int>());
}

void SymbolRegTable::exit_scope() {
    --current_scope;
    symbol_table.pop_back();
}
bool SymbolTable::lookup_current_scope_Symobol(Symbol C,int c_scope){
    const auto &current_map = symbol_table[c_scope];
    std::string goal_str=C->get_string();
    // 遍历当前作用域的 map 并对比
    for (const auto &it : current_map) {
        const Symbol &sym = it.first;
        VarAttribute val = it.second;
        
        std::string it_str=sym->get_string();
        if(it_str==goal_str)
        {
            return true;
        }
        
        
    }
    return false;
}
bool SymbolTable::lookup_current_scope_Symobol(std::string s,int c_scope){
    const auto &current_map = symbol_table[c_scope];
    
    // 遍历当前作用域的 map 并打印
    for (const auto &it : current_map) {
        const Symbol &sym = it.first;
        VarAttribute val = it.second;
        
        std::string it_str=sym->get_string();
        if(it_str==s)
        {
            return true;
        }
        
        
    }
    return false;
}