# Lab5:目标代码生成

## 指令选择

### ==target/common/MachineBaseInstruction.h==

主要定义了基本的机器指令，RISC-V汇编指令继承了该文件中定义的类

```c++
struct MachineDataType {
    enum { INT, FLOAT };
    enum { B32, B64, B128 };
    unsigned data_type;
    unsigned data_length;
    MachineDataType() {}
    MachineDataType(const MachineDataType &other) {
        this->data_type = other.data_type;
        this->data_length = other.data_length;
    }
    MachineDataType operator=(const MachineDataType &other) {
        this->data_type = other.data_type;
        this->data_length = other.data_length;
        return *this;
    }
    MachineDataType(unsigned data_type, unsigned data_length) : data_type(data_type), data_length(data_length) {}
    bool operator==(const MachineDataType &other) const {
        return this->data_type == other.data_type && this->data_length == other.data_length;
    }
    int getDataWidth() {
        switch (data_length) {
        case B32:
            return 4;
        case B64:
            return 8;
        case B128:
            return 16;
        }
        return 0;
    }
    std::string toString() {
        std::string ret;
        if (data_type == INT)
            ret += 'i';
        if (data_type == FLOAT)
            ret += 'f';
        if (data_length == B32)
            ret += "32";
        if (data_length == B64)
            ret += "64";
        if (data_length == B128)
            ret += "128";
        return ret;
    }
};
```

定义了数据类型和数据长度的枚举值，包括整数（INT）和浮点数（FLOAT）以及不同的长度（B32、B64、B128）。



```c++
struct Register {
public:
    int reg_no;         // 寄存器编号
    bool is_virtual;    // 是否为虚拟寄存器
    bool save_across_call;
    MachineDataType type;
    Register() {}
    Register(bool is_virtual, int reg_no, MachineDataType type, bool save = false)
        : is_virtual(is_virtual), reg_no(reg_no), type(type) {}
    int getDataWidth() { return type.getDataWidth(); }
    Register(const Register &other) {
        this->is_virtual = other.is_virtual;
        this->reg_no = other.reg_no;
        this->type = other.type;
    }
    Register operator=(const Register &other) {
        this->is_virtual = other.is_virtual;
        this->reg_no = other.reg_no;
        this->type = other.type;
        return *this;
    }
    bool operator<(Register other) const {
        if (is_virtual != other.is_virtual)
            return is_virtual < other.is_virtual;
        if (reg_no != other.reg_no)
            return reg_no < other.reg_no;
        if (type.data_type != other.type.data_type)
            return type.data_type < other.type.data_type;
        if (type.data_length != other.type.data_length)
            return type.data_length < other.type.data_length;
        return false;
    }
    bool operator==(Register other) const {
        return reg_no == other.reg_no && is_virtual == other.is_virtual && type.data_type == other.type.data_type &&
               type.data_length == other.type.data_length;
    }
};
```



### ==target/common/machine_instruction_structures/==

定义了与汇编代码相关的数据结构，例如控制流图，汇编代码上的函数，基本块等



### ==target/common/machine_passes/machine_pass.h==

MachineIR的Pass基类



### ==target/common/machine_passes/machine_selector.h==

指令选择基类



### ==target/common/machine_passes/machine_printer.h==

指令打印基类



### ==target/riscv64gc/instruction_print/==

RISC-V指令打印类和函数



### ==target/riscv64gc/riscv64.*==

定义了RISC-V指令类和辅助函数



### target/riscv64gc/instruction_select/riscv64_instSelect.*

指令选择关键函数



### ==target/riscv64gc/instruction_select/riscv64_lowerframe.*==

指令选择阶段只需要完成RiscV64LowerFrame类的实现，该类的作用为在函数入口地址处插入获取参数的指令



## 寄存器分配

### ==target/common/machine_passes/register_alloc/basic_register_allocation.*==

寄存器分配基类



### target/common/machine_passes/register_alloc/fast_linear_scan/fast_linear_scan.h

定义了线性扫描算法的相关函数



### ==target/common/machine_passes/register_alloc/machine_liveness.cc==

活跃变量分析



### ==target/common/machine_passes/register_alloc/live_interval.h==

活跃区间计算，你只需要完成判断两个活跃区间是否存在重叠的代码即可。



### ==target/common/machine_passes/register_alloc/physical_register.*==

维护物理寄存器以及溢出寄存器对内存的占用情况，这里定义了一些辅助函数，你可以根据需要实现它们或者根据自己的想法编写新的辅助函数



### target/common/machine_passes/register_alloc/fast_linear_scan/fast_linear_scan.cc

你需要实现线性扫描算法的关键函数



### target/common/machine_passes/register_alloc/vreg_rewrite.h

你需要实现根据寄存器分配情况生成溢出代码的函数，以及根据寄存器分配情况将虚拟寄存器重写为物理寄存器的函数



### ==target/riscv64gc/riscv64.cc==

实现插入Spill指令的函数，这些函数用于在寄存器分配阶段生成溢出代码，放在该文件的原因是溢出指令的生成是体系结构相关的



### ==target/riscv64gc/instruction_select/riscv64_lowerframe.*==

完成寄存器分配后需要完成RiscV64LowerStack类的实现，在函数开始和结尾处插入保存和恢复 函数被调者 需要保存寄存器的代码