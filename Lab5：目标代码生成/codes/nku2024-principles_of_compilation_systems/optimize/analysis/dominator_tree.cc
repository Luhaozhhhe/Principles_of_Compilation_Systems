#include "dominator_tree.h"
#include "../../include/ir.h"
#include "bitset.h"
#include <chrono>
void DomAnalysis::Execute() {
    auto end1 = std::chrono::high_resolution_clock::now();

    for (auto [defI, cfg] : llvmIR->llvm_cfg) {

        //std::cout<<"cfg max label: ";
        //std::cout<<cfg->max_label<<std::endl;
        DominatorTree* dt =new DominatorTree;
        dt->C=cfg;
        dt->BuildDominatorTree();
        DomInfo[cfg] = *dt;
        
        
        //std::cout<<"in domana exe df_size: "<<dt->df.size()<<std::endl;
        
    }

    for(auto [defI, cfg] : llvmIR->llvm_cfg){
        DominatorTree* dt_reverse = new DominatorTree;
        dt_reverse->C=cfg;
        dt_reverse->BuildPostDominatorTree();
        DomInfo_reverse[cfg] = *dt_reverse;
        
        //std::cout<<"in domana exe df_size: "<<dt_reverse->df.size()<<std::endl;
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    // std::cout << "domAnalysis_time: "<< std::chrono::duration_cast<std::chrono::milliseconds>(end2 - end1).count()
    //            << " ms" << std::endl;
    return;

}

//对每个块做深度优先搜索（DFS）的后续遍历
void reverse_dfs(int current_id, const std::vector<std::vector<LLVMBlock>> *G, std::vector<int> &result, 
                std::vector<int> &visited){
    //std::cout<<"current id:"<< current_id<<std::endl;
    
    visited[current_id] = 1;
    for(auto next_block : (*G)[current_id]){
        //std::cout<<"current id:"<< current_id<<std::endl;
        if(!visited[next_block->block_id]){
            //std::cout<<"current id:"<< current_id<<", call next_id: "<<next_block->block_id<<std::endl;
            reverse_dfs(next_block->block_id, G, result, visited);
        }
    }
    result.push_back(current_id);
}

//Reference: https://github.com/yuhuifishash/SysY
void DominatorTree::BuildDominatorTree(){
    const std::vector<std::vector<LLVMBlock>> *G=&(C->G);
    const std::vector<std::vector<LLVMBlock>> *invG= &(C->invG);
    int begin_id=0;

    //std::cout<<"before max_label"<<std::endl;
    int block_num = C->max_label + 1;
    //std::cout<<"after max_label"<<std::endl;
    
    std::vector<int> reverse_id;
    std::vector<int> visited;

    dom_tree.clear();
    dom_tree.resize(block_num);
    idom.clear();//存储每个节点的立即支配节点
    atdom.clear();//一个动态位集，用于表示每个节点的支配关系集合

    for(int i = 0; i<=C->max_label; i++){
        visited.push_back(0);//初始化所有的节点为未访问状态
    }
    //std::cout<<"after init visited"<<std::endl;

    //开始深度后续遍历
    reverse_dfs(begin_id, (G),reverse_id,visited);
    //std::cout<<"after dfs"<<std::endl;
    //初始化支配集合atdom
    atdom.resize(block_num);
    for(int i = 0; i < block_num; i++){
        atdom[i].remake(block_num);
    }
    //std::cout<<"after init atdom"<<std::endl;

    atdom[begin_id].setbit(begin_id,1);// 起始节点自支配自己
    for(int i = 0; i <= C->max_label; i++){
        for(int j = 0;j <= C->max_label; j++){
            if(i != begin_id){
                atdom[i].setbit(j,1);// 非起始节点支配所有节点
            }
        }
    }
    //std::cout<<"after dom def"<<std::endl;


    bool changed = 1;
    while (changed) {
        changed = 0;
        for (std::vector<int>::reverse_iterator it = reverse_id.rbegin(); it != reverse_id.rend(); ++it) {
            auto u = *it;

            DynamicBitset new_dom_u(block_num);
            if (!(*invG)[u].empty()) {
                new_dom_u = atdom[(*((*invG)[u].begin()))->block_id];
                for (auto v : (*invG)[u]) {
                    // new_dom_u &= atdom[v->block_id];
                    new_dom_u = new_dom_u & atdom[v->block_id];
                }
            }
            // Second:
            // dom(u) |= {u}
            new_dom_u.setbit(u, 1);
            if (new_dom_u != atdom[u]) {
                atdom[u] = new_dom_u;
                changed = 1;
            }
        }
    }
    //std::cout<<"after while"<<std::endl;

    idom.clear();
    idom.resize(block_num);

    for (auto [u, bb] : *C->block_map) {

        if (u == begin_id) {
            continue;
        }
        for (int v = 0; v <= C->max_label; v++) {

            if (atdom[u].getbit(v)) {
                //v dom u
                auto tmp = (atdom[u] & atdom[v]) ^ atdom[u];

                if (tmp.count_the_number_of_one() == 1 && tmp.getbit(u)) {
                    //std::cout<<"5"<<std::endl;

                    idom[u] = (*C->block_map)[v];  // 找到 u 的立即且唯一支配节点 v
                    dom_tree[v].push_back((*C->block_map)[u]);
                }

            }

        }

    }

    df.clear();//是一个动态位集，表示从一个节点到另一个节点的支配关系
    //std::cout<<"df block_num: "<<block_num<<std::endl;
    
    df.resize(block_num);
    //std::cout<<"df_size: "<<df.size()<<std::endl;

    for (int i = 0; i < block_num; i++) {

        df[i].remake(block_num);  // 初始化支配前沿
    }

    for (int i = 0; i < (*G).size(); i++) {
        for (auto edg_end : (*G)[i]) {//遍历节点 i 的邻接节点集合
            int a = i;//当前节点
            int b = edg_end->block_id;//通过a到达的邻接节点
            int x = a;//追溯到支配节点的辅助变量
            while (x == b || IsDominate(x, b) == 0) {//循环，找到b的先驱
                //std::cout<<"In for for df_size: "<<df.size()<<std::endl;
            
                df[x].setbit(b, 1);  // x 是 b 的支配前沿
                if (idom[x] != NULL) {
                    x = idom[x]->block_id;
                } else {
                    break;
                }
            }
        }
    }

}

void DominatorTree::BuildPostDominatorTree(){
    const std::vector<std::vector<LLVMBlock>> *G=&(C->invG);
    const std::vector<std::vector<LLVMBlock>> *invG= &(C->G);
    Assert(C->ret_block != nullptr);
    int begin_id = C->ret_block->block_id;

    //std::cout<<"before max_label"<<std::endl;
    int block_num = C->max_label + 1;
    //std::cout<<"after max_label"<<std::endl;
    
    std::vector<int> reverse_id;
    std::vector<int> visited;

    dom_tree.clear();
    dom_tree.resize(block_num);
    idom.clear();//存储每个节点的立即支配节点
    atdom.clear();//一个动态位集，用于表示每个节点的支配关系集合

    for(int i = 0; i<=C->max_label; i++){
        visited.push_back(0);//初始化所有的节点为未访问状态
    }
    //std::cout<<"after init visited"<<std::endl;

    //开始深度后续遍历
    reverse_dfs(begin_id, (G),reverse_id,visited);
    //std::cout<<"after dfs"<<std::endl;
    //初始化支配集合atdom
    atdom.resize(block_num);
    for(int i = 0; i < block_num; i++){
        atdom[i].remake(block_num);
    }
    //std::cout<<"after init atdom"<<std::endl;

    atdom[begin_id].setbit(begin_id,1);// 起始节点自支配自己
    for(int i = 0; i <= C->max_label; i++){
        for(int j = 0;j <= C->max_label; j++){
            if(i != begin_id){
                atdom[i].setbit(j,1);// 非起始节点支配所有节点
            }
        }
    }
    //std::cout<<"after dom def"<<std::endl;


    bool changed = 1;
    while (changed) {
        changed = 0;
        for (std::vector<int>::reverse_iterator it = reverse_id.rbegin(); it != reverse_id.rend(); ++it) {
            auto u = *it;

            DynamicBitset new_dom_u(block_num);
            if (!(*invG)[u].empty()) {
                new_dom_u = atdom[(*((*invG)[u].begin()))->block_id];
                for (auto v : (*invG)[u]) {
                    // new_dom_u &= atdom[v->block_id];
                    new_dom_u = new_dom_u & atdom[v->block_id];
                }
            }
            // Second:
            // dom(u) |= {u}
            new_dom_u.setbit(u, 1);
            if (new_dom_u != atdom[u]) {
                atdom[u] = new_dom_u;
                changed = 1;
            }
        }
    }
    //std::cout<<"after while"<<std::endl;

    idom.clear();
    idom.resize(block_num);

    for (auto [u, bb] : *C->block_map) {

        if (u == begin_id) {
            continue;
        }
        for (int v = 0; v <= C->max_label; v++) {

            if (atdom[u].getbit(v)) {

                auto tmp = (atdom[u] & atdom[v]) ^ atdom[u];

                if (tmp.count_the_number_of_one() == 1 && tmp.getbit(u)) {
                    //std::cout<<"5"<<std::endl;

                    idom[u] = (*C->block_map)[v];  // 找到 u 的立即支配节点 v
                    dom_tree[v].push_back((*C->block_map)[u]);
                }

            }

        }

    }

    df.clear();//是一个动态位集，表示从一个节点到另一个节点的支配关系
    //std::cout<<"df block_num: "<<block_num<<std::endl;
    
    df.resize(block_num);
    //std::cout<<"df_size: "<<df.size()<<std::endl;

    for (int i = 0; i < block_num; i++) {

        df[i].remake(block_num);  // 初始化支配前沿
    }

    for (int i = 0; i < (*G).size(); i++) {
        for (auto edg_end : (*G)[i]) {//遍历节点 i 的邻接节点集合
            int a = i;//当前节点
            int b = edg_end->block_id;//通过a到达的邻接节点
            int x = a;//追溯到支配节点的辅助变量
            while (x == b || IsDominate(x, b) == 0) {//循环，找到b的先驱
                //std::cout<<"In for for df_size: "<<df.size()<<std::endl;
            
                df[x].setbit(b, 1);  // x 是 b 的支配前沿
                if (idom[x] != NULL) {
                    x = idom[x]->block_id;
                } else {
                    break;
                }
            }
        }
    }
}


std::set<int> DominatorTree::GetDF(std::set<int> S){//计算节点集合 S 的联合支配前沿
    DynamicBitset result(C->max_label + 1);
    std::set<int> temp_set;
    for (auto node : S) 
    {
        result = result | df[node];
    }
    for (int i = 0; i <= C->max_label; ++i) 
    {
        if (result.getbit(i)) 
        {
            temp_set.insert(i);
        }
    }

    return temp_set;

}

std::set<int> DominatorTree::GetDF(int id){//计算单个节点 id 的支配前沿
    //std::cout<<"id"<<id<<std::endl;
    //std::cout<<"now df size: "<<df.size()<<std::endl;

    std::set<int> temp_set;
    //std::cout<<"ml: "<<C->max_label<<std::endl;
    for(int i = 0; i <= C->max_label; ++i){
        //std::cout<<"in GetDF for"<<std::endl;

        //std::cout<<"id: "<<id<<std::endl;
        //std::cout<<"df size: "<<df.size()<<std::endl;

        //std::cout<<"debug now:df has no num!"<<std::endl;

        if(df[id].getbit(i) == 1){
            //std::cout<<"temp_set insert "<<i<<std::endl;
            
            temp_set.insert(i);
        }

    }

    return temp_set;
    //std::cout<<"GetDF end!!!"<<std::endl;

}


//判断id1块是否为id2块的支配
bool DominatorTree::IsDominate(int id1, int id2){
    bool temp = atdom[id2].getbit(id1);
    return temp;
}