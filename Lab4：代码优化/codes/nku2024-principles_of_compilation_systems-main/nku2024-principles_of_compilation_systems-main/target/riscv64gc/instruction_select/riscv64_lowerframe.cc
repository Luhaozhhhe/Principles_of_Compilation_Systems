#include "riscv64_lowerframe.h"

/*
    假设IR中的函数定义为f(i32 %r0, i32 %r1)
    则parameters应该存放两个虚拟寄存器%0,%1

    在LowerFrame后应当为
    add %0,a0,x0  (%0 <- a0)
    add %1,a1,x0  (%1 <- a1)

    对于浮点寄存器按照类似的方法处理即可
*/
void RiscV64LowerFrame::Execute() {
    // 在每个函数的开头处插入获取参数的指令
    for (auto func : unit->functions) {
        current_func = func;
        for (auto &b : func->blocks) {
            cur_block = b;
            if (b->getLabelId() == 0) 
            {    
                // 函数入口，需要插入获取参数的指令
                Register para_basereg = current_func->GetNewReg(INT64);
                int count_of_int32 = 0;
                int count_of_float32 = 0;
                int sum_off = 0;
                for (auto para : func->GetParameters()) 
                {    // 你需要在指令选择阶段正确设置parameters的值

                    if (para.type.data_type == INT64.data_type) 
                    {
                        if (count_of_int32 < 8) 
                        {    // 插入使用寄存器传参的指令
                            b->push_front(rvconstructor->ConstructR(RISCV_ADD, para, GetPhysicalReg(RISCV_a0 + count_of_int32), GetPhysicalReg(RISCV_x0)));
                        }
                        if (count_of_int32 >= 8) 
                        {    // 插入使用内存传参的指令
                            //TODO("Implement this if you need");
                            int offset = (sum_off) * 8; // 假设栈顶是存储超出的参数
                            b->push_front(rvconstructor->ConstructIImm(RISCV_LD, para, GetPhysicalReg(RISCV_fp), offset));
                            sum_off++;                   
                        }
                        count_of_int32++;
                        //sum_off++;
                    } 
                    else if (para.type.data_type == FLOAT64.data_type) 
                    {    // 处理浮点数
                        //TODO("Implement this if you need");
                        if (count_of_float32 < 8) 
                        {
                            b->push_front(rvconstructor->ConstructR2(RISCV_FMV_D, para, GetPhysicalReg(RISCV_fa0 + count_of_float32)));
                        } 
                        else 
                        {
                            // 超过8个浮点参数，从栈中加载
                            int offset = (sum_off) * 8;
                            b->push_front(rvconstructor->ConstructIImm(RISCV_FLD, para, GetPhysicalReg(RISCV_fp), offset));
                            sum_off++;

                        }
                        count_of_float32++;
                        //sum_off++;
                    } 
                    else 
                    {
                        ERROR("Unknown type");
                    }
                }
                if (sum_off != 0) 
                {    
                    //总的偏移量
                    func->spilled=true;
                    cur_block->push_front
                    (
                        rvconstructor->ConstructR(RISCV_ADD, para_basereg, GetPhysicalReg(RISCV_fp), 
                        GetPhysicalReg(RISCV_x0))
                    );
                //fp是frame pointer，记录了保存的寄存器的位置，将para_basereg的值设置为基址，放到最前面

                }
            }
        }
    }
}

void RiscV64LowerStack::Execute() {
    // 在函数在寄存器分配后执行
    // TODO: 在函数开头保存 函数被调者需要保存的寄存器，并开辟栈空间
    // TODO: 在函数结尾恢复 函数被调者需要保存的寄存器，并收回栈空间
    // TODO: 开辟和回收栈空间
    // 具体需要保存/恢复哪些可以查阅RISC-V函数调用约定
    //Log("RiscV64LowerStack");
    for(auto func : unit->functions)
    {
        current_func = func;
        int stack_offset = 0; // 栈空间偏移
        //std::vector<int> callee_saved = {}; // 被调用者保存的寄存器列表
        std::vector<int> callee_saved = 
        {          
            RISCV_sp,
            
            RISCV_s0, RISCV_s1, 

            RISCV_s2, RISCV_s3, RISCV_s4, RISCV_s5, 
            RISCV_s6, RISCV_s7, RISCV_s8, RISCV_s9, 
            RISCV_s10, RISCV_s11,
            
            RISCV_fs0, RISCV_fs1, 

            RISCV_fs2, RISCV_fs3, RISCV_fs4, RISCV_fs5, 
            RISCV_fs6, RISCV_fs7, RISCV_fs8, RISCV_fs9, 
            RISCV_fs10, RISCV_fs11, 
            
            RISCV_ra,
        };
        int fp_restore_offset;
       
        func->AddStackSize((callee_saved.size()) * 8);
        // 分配栈空间
        auto alloc_instr = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_sp), GetPhysicalReg(RISCV_sp), -func->GetStackSize());
        func->blocks.front()->push_front(alloc_instr);
        if(func->spilled){
            func->blocks.front()->push_front(rvconstructor->ConstructR(RISCV_ADD, GetPhysicalReg(RISCV_fp),
                                                            GetPhysicalReg(RISCV_sp), GetPhysicalReg(RISCV_x0)));
            
        }
        for (auto reg : callee_saved) {
            stack_offset += 8; // 每个寄存器占用8字节
            RiscV64Instruction * store_instr;

            if(reg == RISCV_s0)
            {
                //FP
                store_instr = rvconstructor->ConstructSImm(RISCV_SD, GetPhysicalReg(reg), GetPhysicalReg(RISCV_sp), -stack_offset);
                fp_restore_offset = stack_offset;
                func->blocks.front()->push_front(store_instr); // 函数开头保存寄存器
            }
            else if(reg <= RISCV_s11 && reg >= RISCV_s1 || reg == RISCV_ra || reg == RISCV_sp)
            {
                store_instr = rvconstructor->ConstructSImm(RISCV_SD, GetPhysicalReg(reg), GetPhysicalReg(RISCV_sp), -stack_offset);
                func->blocks.front()->push_front(store_instr); // 函数开头保存寄存器
    
            }
            else if(reg <= RISCV_fs11 && reg >= RISCV_fs0)
            {
                store_instr = rvconstructor->ConstructSImm(RISCV_FSD, GetPhysicalReg(reg), GetPhysicalReg(RISCV_sp), -stack_offset);
                func->blocks.front()->push_front(store_instr); // 函数开头保存寄存器

            }
            
        }

        for(auto &b : func->blocks)
        {
            cur_block = b;
            auto last_ins = *(b->ReverseBegin());   //看是否为返回指令
            Assert(last_ins->arch == MachineBaseInstruction::RiscV);
            auto riscv_last_ins = (RiscV64Instruction *)last_ins;
            if (riscv_last_ins->getOpcode() == RISCV_JALR) {
                if (riscv_last_ins->getRd() == GetPhysicalReg(RISCV_x0) && riscv_last_ins->getRs1() == GetPhysicalReg(RISCV_ra)) 
                {
                    Assert(riscv_last_ins->getImm() == 0);
                    b->pop_back();
                    int temp_stack_offset = 0;
                    auto dealloc_instr = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_sp), GetPhysicalReg(RISCV_sp), func->GetStackSize());
                    b->push_back(dealloc_instr);

                    for (auto reg : callee_saved) {
                        RiscV64Instruction *load_instr;
                        temp_stack_offset += 8 ;
                        if(reg <= RISCV_x31)
                        {
                            load_instr = rvconstructor->ConstructIImm(RISCV_LD, GetPhysicalReg(reg), GetPhysicalReg(RISCV_sp), -temp_stack_offset);
                        }
                        else    
                        {
                            load_instr = rvconstructor->ConstructIImm(RISCV_FLD, GetPhysicalReg(reg), GetPhysicalReg(RISCV_sp), -temp_stack_offset);
                        }
                        
                        b->push_back(load_instr); // 函数结尾恢复寄存器
                        
                    }
                    
                    b->push_back(last_ins);
                }
            }
        }
    }


    // 到此我们就完成目标代码生成的所有工作了
}
