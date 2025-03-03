# lab4:代码优化

基本要求(1.5分)

cfg：

- [x] 控制流图cfg构建
- [x] 在控制流图上进行dfs，完成不可达指令和基本块的删除

mem2reg：

- [x] 消除只有定义 没有被使用的变量
- [x] 完成定义和使用均在同一基本块的mem2reg

进阶要求(2.5分+4分可溢出)

- [x] 完整的mem2reg构建支配树 插入phi指令 变量重命名(2分)
- [x] 普通的死代码消除(0.5分)
- [x] ADCE(1分)
- [ ] TCO(0.7分)
- [ ] Fully Redundancy Elimination in BasicBlock(Memory+Call)(1分)
- [ ] 函数内联+基本调用展开(1分)
- [ ] Fully Redundancy Elimination(Scalar)(1.5分)
- [ ] Fully Redundancy Elimination(Memory)(1分)







**基本要求**需要阅读并编写的代码:

**include/cfg.h**:控制流图的定义

**optimize/cfg/cfg.cc**:基本的cfg优化函数及辅助函数定义

**optimize/ssa/mem2reg.h**:Mem2Reg类的定义

**optimize/ssa/mem2reg.cc**:Mem2Reg类的实现, 基本要求只需要实现部分函数即可









**进阶要求**需要阅读并编写的代码:

**optimize/cfg/dominator_tree.h**:支配树类的定义

**optimize/cfg/dominator_tree.cc**:支配树类的实现

**optimize/ssa/mem2reg.cc**:进阶要求中，你需要实现完整的mem2reg

同时，如果你想在本次实验获得满分，你还需要根据大作业要求自行选择一些优化pass编写, 你需要自己从头开始编写你的优化pass