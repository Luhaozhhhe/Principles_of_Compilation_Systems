#include "riscv64_instSelect.h"
#include <sstream>

long long int fimm_string2int(std::string s)
{
    long long int sum = 0;
    for(int i =2; i<s.length() ;i++)
    {
        sum*=16;
        std::cout<<s[i]<<std::endl;
        if(s[i]<='9' && s[i]>='0')
        {
            sum += (s[i] - '0');
        }
        else if(s[i]<='f' && s[i]>='a')
        {
            sum += (s[i] - 'a' + 10);
        }
        
        
        //std::cout<<std::hex<<sum<<std::endl;
    }
    //std::cout<<std::hex<<sum<<std::endl;
    return sum;
}

//Reference: https://github.com/yuhuifishash/SysY
template <> void RiscV64Selector::ConvertAndAppend<LoadInstruction *>(LoadInstruction *ins) {//DONE
    switch (ins->GetPointer()->GetOperandType()) 
    {
        case BasicOperand::REG: 
        {
            auto ptr_op = (RegOperand *)ins->GetPointer();
            auto rd_op = (RegOperand *)ins->GetResult();

            switch (ins->GetDataType()) 
            {
                case BasicInstruction::I32: 
                {
                    Register rd = GetllvmReg(rd_op->GetRegNo(), INT64);
                    if (llvm_riscv_allocas.find(ptr_op->GetRegNo()) == llvm_riscv_allocas.end()) 
                    {
                        Register ptr = GetllvmReg(ptr_op->GetRegNo(), INT64);    // INT64 HERE
                        auto lw_instr = rvconstructor->ConstructIImm(RISCV_LW, rd, ptr, 0);
                        cur_block->push_back(lw_instr);
                    } 
                    else 
                    {
                        auto sp_offset = llvm_riscv_allocas[ptr_op->GetRegNo()];
                        auto lw_instr = rvconstructor->ConstructIImm(RISCV_LW, rd, GetPhysicalReg(RISCV_sp), sp_offset);
                        ((RiscV64Function *)cur_func)->AddAllocaIns(lw_instr);
                        cur_block->push_back(lw_instr);
                    }
                    break;
                }
                case BasicInstruction::FLOAT32: 
                {
                    Register rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
                    if (llvm_riscv_allocas.find(ptr_op->GetRegNo()) == llvm_riscv_allocas.end()) 
                    {
                        Register ptr = GetllvmReg(ptr_op->GetRegNo(), INT64);    // INT64 HERE
                        auto lw_instr = rvconstructor->ConstructIImm(RISCV_FLW, rd, ptr, 0);
                        cur_block->push_back(lw_instr);
                    } 
                    else 
                    {
                        auto sp_offset = llvm_riscv_allocas[ptr_op->GetRegNo()];
                        auto lw_instr = rvconstructor->ConstructIImm(RISCV_FLW, rd, GetPhysicalReg(RISCV_sp), sp_offset);
                        ((RiscV64Function *)cur_func)->AddAllocaIns(lw_instr);
                        cur_block->push_back(lw_instr);
                    }
                    break;
                }
                case BasicInstruction::PTR: 
                {
                    Register rd = GetllvmReg(rd_op->GetRegNo(), INT64);
                    if (llvm_riscv_allocas.find(ptr_op->GetRegNo()) == llvm_riscv_allocas.end()) 
                    {
                        Register ptr = GetllvmReg(ptr_op->GetRegNo(), INT64);    // INT64 HERE
                        auto lw_instr = rvconstructor->ConstructIImm(RISCV_LD, rd, ptr, 0);
                        cur_block->push_back(lw_instr);
                    } 
                    else 
                    {
                        auto sp_offset = llvm_riscv_allocas[ptr_op->GetRegNo()];
                        auto lw_instr = rvconstructor->ConstructIImm(RISCV_LD, rd, GetPhysicalReg(RISCV_sp), sp_offset);
                        ((RiscV64Function *)cur_func)->AddAllocaIns(lw_instr);
                        cur_block->push_back(lw_instr);
                    }
                    break;
                }
                default:
                    ERROR("Unexpected data type");
                    break;
            }
            break;
        }
        case BasicOperand::GLOBAL: 
        {
            // lui %r0, %hi(x)
            // lw  %rd, %lo(x)(%r0)
            // %hi(x) 为全局变量 x 地址的高 20 位。
            // 这是因为 lui 指令只能处理 20 位立即数，并且会将其左移 12 位
            // %lo(x) 即全局变量 x 地址的低 12 位。
            // 在后续的加载指令（如 lw、flw、ld）中，它作为偏移量使用，结合 lui 指令存储在寄存器中的高地址部分，共同构成完整的地址。
            auto global_op = (GlobalOperand *)ins->GetPointer();
            auto rd_op = (RegOperand *)ins->GetResult();

            Register addr_hi = GetNewReg(INT64);

            switch (ins->GetDataType()) 
            {
                case BasicInstruction::I32: 
                {
                    Register rd = GetllvmReg(rd_op->GetRegNo(), INT64);

                    auto lui_instr = rvconstructor->ConstructULabel(RISCV_LUI, addr_hi, RiscVLabel(global_op->GetName(), true));
                    auto lw_instr = rvconstructor->ConstructILabel(RISCV_LW, rd, addr_hi, RiscVLabel(global_op->GetName(), false));

                    cur_block->push_back(lui_instr);
                    cur_block->push_back(lw_instr);
                    break;
                }
                case BasicInstruction::FLOAT32: 
                {
                    Register rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);

                    auto lui_instr = rvconstructor->ConstructULabel(RISCV_LUI, addr_hi, RiscVLabel(global_op->GetName(), true));
                    auto lw_instr = rvconstructor->ConstructILabel(RISCV_FLW, rd, addr_hi, RiscVLabel(global_op->GetName(), false));

                    cur_block->push_back(lui_instr);
                    cur_block->push_back(lw_instr);
                    break;
                }
                case BasicInstruction::PTR: 
                {
                    Register rd = GetllvmReg(rd_op->GetRegNo(), INT64);

                    auto lui_instr = rvconstructor->ConstructULabel(RISCV_LUI, addr_hi, RiscVLabel(global_op->GetName(), true));
                    auto lw_instr = rvconstructor->ConstructILabel(RISCV_LD, rd, addr_hi, RiscVLabel(global_op->GetName(), false));

                    cur_block->push_back(lui_instr);
                    cur_block->push_back(lw_instr);
                    break;
                }
                default:
                    ERROR("Unexpected data type");
                    break;
            }
            break;
        }
    }
}

//Reference: https://github.com/yuhuifishash/SysY
template <> void RiscV64Selector::ConvertAndAppend<StoreInstruction *>(StoreInstruction *ins) {//DONE
    //TODO("Implement this if you need");
    Register value_reg;
    switch (ins->GetValue()->GetOperandType()) {
        case BasicOperand::IMMI32: {
            auto val_imm = (ImmI32Operand *)ins->GetValue();

            value_reg = GetNewReg(INT64);

            int imm_val = val_imm->GetIntImmVal();
            // int32_t high_imm = (imm_val + 0x800) >> 12;
            // int32_t low_imm = (imm_val<<20)>>20 ;
            // auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, value_reg, high_imm);
            // cur_block->push_back(lui_instr);
            // auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, value_reg, value_reg, low_imm);
            auto li_instr = rvconstructor->ConstructUImm(RISCV_LI, value_reg, imm_val);
            cur_block->push_back(li_instr);
            break;
        }
        case BasicOperand::REG: 
        {
            auto val_reg = (RegOperand *)ins->GetValue();
            switch (ins->GetDataType()) 
            {
                case BasicInstruction::I32: 
                {
                    value_reg = GetllvmReg(val_reg->GetRegNo(), INT64);
                    break;
                }
                case BasicInstruction::FLOAT32: 
                {
                    value_reg = GetllvmReg(val_reg->GetRegNo(), FLOAT64);
                    break;
                }
                default:
                    ERROR("Unexpected data type");
                    break;
            }
            break;
        }
        case BasicOperand::IMMF32: 
        {
            // Log("Deal with float immediate On store");
            auto val_imm = (ImmF32Operand *)ins->GetValue();

            value_reg = GetNewReg(FLOAT64);

            float val = val_imm->GetFloatVal();
            if (val == 0.0) 
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, value_reg, GetPhysicalReg(RISCV_x0)));
            } 
            else 
            {
                auto int_rep = *(int*)&val;
                int high_imm = (int_rep + 0x800) >> 12;  // 高 20 位（向上取整）
                int low_imm = (int_rep << 20) >> 20;  // 低 12 位
                Register temp_int_reg = GetNewReg(INT64);
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, temp_int_reg, high_imm);
                cur_block->push_back(lui_instr);
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, temp_int_reg, temp_int_reg, low_imm);
                cur_block->push_back(addi_instr);
                auto fmv_instr = rvconstructor->ConstructR2(RISCV_FMV_D_X, value_reg, temp_int_reg);
                cur_block->push_back(fmv_instr);
            }
            break;
        }
        default:
            ERROR("Unexpected or unimplemented operand type");
            break;
    }

    switch (ins->GetPointer()->GetOperandType()) 
    {
        case BasicOperand::REG: 
        {
            auto reg_ptr_op = (RegOperand *)ins->GetPointer();
            switch (ins->GetDataType()) 
            {
                case BasicInstruction::I32: 
                {
                    if (llvm_riscv_allocas.find(reg_ptr_op->GetRegNo()) == llvm_riscv_allocas.end()) 
                    {
                        auto ptr_reg = GetllvmReg(reg_ptr_op->GetRegNo(), INT64);

                        auto store_instruction = rvconstructor->ConstructSImm(RISCV_SW, value_reg, ptr_reg, 0);
                        cur_block->push_back(store_instruction);
                    } 
                    else 
                    {
                        auto sp_offset = llvm_riscv_allocas[reg_ptr_op->GetRegNo()];

                        auto store_instruction =
                        rvconstructor->ConstructSImm(RISCV_SW, value_reg, GetPhysicalReg(RISCV_sp), sp_offset);
                        ((RiscV64Function *)cur_func)->AddAllocaIns(store_instruction);
                        cur_block->push_back(store_instruction);
                    }
                    break;
                }
                case BasicInstruction::FLOAT32: 
                {
                    if (llvm_riscv_allocas.find(reg_ptr_op->GetRegNo()) == llvm_riscv_allocas.end()) 
                    {
                        auto ptr_reg = GetllvmReg(reg_ptr_op->GetRegNo(), INT64);

                        auto store_instruction = rvconstructor->ConstructSImm(RISCV_FSW, value_reg, ptr_reg, 0);
                        cur_block->push_back(store_instruction);
                    } 
                    else 
                    {
                        auto sp_offset = llvm_riscv_allocas[reg_ptr_op->GetRegNo()];

                        auto store_instruction =
                        rvconstructor->ConstructSImm(RISCV_FSW, value_reg, GetPhysicalReg(RISCV_sp), sp_offset);
                        ((RiscV64Function *)cur_func)->AddAllocaIns(store_instruction);
                        cur_block->push_back(store_instruction);
                    }
                    break;
                }
                default:
                    ERROR("Unexpected data type");
                    break;
            }
            break;
        }
        case BasicOperand::GLOBAL: 
        {
            auto global_op = (GlobalOperand *)ins->GetPointer();

            auto addr_hi = GetNewReg(INT64);

            auto lui_instruction =
            rvconstructor->ConstructULabel(RISCV_LUI, addr_hi, RiscVLabel(global_op->GetName(), true));
            cur_block->push_back(lui_instruction);
            switch (ins->GetDataType()) 
            {
                case BasicInstruction::I32: 
                {
                    auto store_instruction =
                    rvconstructor->ConstructSLabel(RISCV_SW, value_reg, addr_hi, RiscVLabel(global_op->GetName(), false));
                    cur_block->push_back(store_instruction);
                    break;
                }
                case BasicInstruction::FLOAT32: 
                {
                    auto store_instruction =
                    rvconstructor->ConstructSLabel(RISCV_FSW, value_reg, addr_hi, RiscVLabel(global_op->GetName(), false));
                    cur_block->push_back(store_instruction);
                    break;
                }
                default:
                    ERROR("Unexpected data type");
                    break;
            }
            break;
        }
    }
}

template <> void RiscV64Selector::ConvertAndAppend<ArithmeticInstruction *>(ArithmeticInstruction *ins) {//DONE
    //TODO("Implement this if you need");
    if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::ADD){
        if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
        {
            auto *imm1_op = (ImmI32Operand *)ins->GetOperand1();
            auto *imm2_op = (ImmI32Operand *)ins->GetOperand2();
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto imm1 = imm1_op->GetIntImmVal();
            auto imm2 = imm2_op->GetIntImmVal();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto load_imm_instr = rvconstructor->ConstructUImm(RISCV_LI, rd, imm1+imm2);
            cur_block->push_back(load_imm_instr);
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand2();
            auto *i_op = (ImmI32Operand *)ins->GetOperand1();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
            auto imm = i_op->GetIntImmVal();
            auto calc_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd, rs, imm);
            cur_block->push_back(calc_instr);
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *i_op = (ImmI32Operand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
            auto imm = i_op->GetIntImmVal();
            // Use ADDIW or SUBIW for Reg + Imm
            auto calc_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd, rs, imm);
            cur_block->push_back(calc_instr);
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *rt_op = (RegOperand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
            auto rt = GetllvmReg(rt_op->GetRegNo(), INT64);
            // Use ADDW or SUBW for Reg + Reg
            auto calc_instr = rvconstructor->ConstructR(RISCV_ADDW, rd, rs, rt);
            cur_block->push_back(calc_instr);
        }
        else{
            std::cout<<"wrong"<<std::endl;
        }
    }
    else if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::SUB){
        if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
        ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
    {
        auto *imm1_op = (ImmI32Operand *)ins->GetOperand1();
        auto *imm2_op = (ImmI32Operand *)ins->GetOperand2();
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto imm1 = imm1_op->GetIntImmVal();
        auto imm2 = imm2_op->GetIntImmVal();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto load_imm_instr = rvconstructor->ConstructUImm(RISCV_LI, rd, imm1 - imm2);
        cur_block->push_back(load_imm_instr);
    }
    else if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
             ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
    {
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto *rs_op = (RegOperand *)ins->GetOperand2();
        auto *i_op = (ImmI32Operand *)ins->GetOperand1();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);

        auto imm = i_op->GetIntImmVal();
        Register temp = cur_func->GetNewReg(INT64);
        auto load_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, temp, GetPhysicalReg(RISCV_x0), imm);
        auto calc_instr = rvconstructor->ConstructR(RISCV_SUBW, rd, temp, rs);
        cur_block->push_back(load_instr);
        cur_block->push_back(calc_instr);
    }
    else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
             ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
    {
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto *rs_op = (RegOperand *)ins->GetOperand1();
        auto *i_op = (ImmI32Operand *)ins->GetOperand2();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
        auto imm = i_op->GetIntImmVal();
        auto calc_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd, rs, -imm);
        cur_block->push_back(calc_instr);
    }
    else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
             ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
    {
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto *rs_op = (RegOperand *)ins->GetOperand1();
        auto *rt_op = (RegOperand *)ins->GetOperand2();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
        auto rt = GetllvmReg(rt_op->GetRegNo(), INT64);
        auto calc_instr = rvconstructor->ConstructR(RISCV_SUBW, rd, rs, rt);
        cur_block->push_back(calc_instr);
    }
    else{
        std::cout << "SUB: Invalid operands" << std::endl;
    }
    }
    else if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::MUL){
        if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
        ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
    {
        auto *imm1_op = (ImmI32Operand *)ins->GetOperand1();
        auto *imm2_op = (ImmI32Operand *)ins->GetOperand2();
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto imm1 = imm1_op->GetIntImmVal();
        auto imm2 = imm2_op->GetIntImmVal();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto load_imm_instr = rvconstructor->ConstructUImm(RISCV_LI, rd, imm1 * imm2);
        cur_block->push_back(load_imm_instr);
    }
    else if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
             ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
    {
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto *rs_op = (RegOperand *)ins->GetOperand2();
        auto *i_op = (ImmI32Operand *)ins->GetOperand1();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
        Register mul1 = cur_func->GetNewReg(INT64);
        auto imm = i_op->GetIntImmVal();
        auto calc_imm_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, mul1, GetPhysicalReg(RISCV_x0), imm);
        auto calc_instr = rvconstructor->ConstructR(RISCV_MULW, rd, mul1, rs);
        cur_block->push_back(calc_imm_instr);//added lhz
        cur_block->push_back(calc_instr);
    }
    else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
             ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
    {
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto *rs_op = (RegOperand *)ins->GetOperand1();
        auto *i_op = (ImmI32Operand *)ins->GetOperand2();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
        Register mul2 = cur_func->GetNewReg(INT64);
        auto imm = i_op->GetIntImmVal();
        auto calc_imm_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, mul2, GetPhysicalReg(RISCV_x0), imm);
        auto calc_instr = rvconstructor->ConstructR(RISCV_MULW, rd, rs, mul2);
        cur_block->push_back(calc_imm_instr);
        cur_block->push_back(calc_instr);
    }
    else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
             ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
    {
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto *rs_op = (RegOperand *)ins->GetOperand1();
        auto *rt_op = (RegOperand *)ins->GetOperand2();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
        auto rt = GetllvmReg(rt_op->GetRegNo(), INT64);
        auto calc_instr = rvconstructor->ConstructR(RISCV_MULW, rd, rs, rt);

        cur_block->push_back(calc_instr);
    }
    else{
        std::cout << "MUL: Invalid operands" << std::endl;
    }
    }
    else if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::DIV){
        if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
        ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
    {
        auto *imm1_op = (ImmI32Operand *)ins->GetOperand1();
        auto *imm2_op = (ImmI32Operand *)ins->GetOperand2();
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto imm1 = imm1_op->GetIntImmVal();
        auto imm2 = imm2_op->GetIntImmVal();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto load_imm_instr = rvconstructor->ConstructUImm(RISCV_LI, rd, imm1 / imm2);
        cur_block->push_back(load_imm_instr);
    }
    else if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
             ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
    {
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto *rs_op = (RegOperand *)ins->GetOperand2();
        auto *i_op = (ImmI32Operand *)ins->GetOperand1();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
        auto imm = i_op->GetIntImmVal();
        Register div1 = cur_func->GetNewReg(INT64);
        auto calc_imm_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, div1, GetPhysicalReg(RISCV_x0), imm);
        auto calc_instr = rvconstructor->ConstructR(RISCV_DIVW, rd, div1, rs);
        cur_block->push_back(calc_imm_instr);
        cur_block->push_back(calc_instr);
    }
    else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
             ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
    {
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto *rs_op = (RegOperand *)ins->GetOperand1();
        auto *i_op = (ImmI32Operand *)ins->GetOperand2();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
        auto imm = i_op->GetIntImmVal();
        Register div2 = cur_func->GetNewReg(INT64);
        auto calc_imm_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, div2, GetPhysicalReg(RISCV_x0), imm);
        auto calc_instr = rvconstructor->ConstructR(RISCV_DIVW, rd, rs, div2);
        cur_block->push_back(calc_imm_instr);
        cur_block->push_back(calc_instr);
    }
    else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
             ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
    {
        auto *rd_op = (RegOperand *)ins->GetResultOperand();
        auto *rs_op = (RegOperand *)ins->GetOperand1();
        auto *rt_op = (RegOperand *)ins->GetOperand2();
        auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
        auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
        auto rt = GetllvmReg(rt_op->GetRegNo(), INT64);
        auto calc_instr = rvconstructor->ConstructR(RISCV_DIVW, rd, rs, rt);
        cur_block->push_back(calc_instr);
    }
    else{
        std::cout << "DIV: Invalid operands" << std::endl;
    }
    }
    else if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::FADD){
        if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMF32 &&
                ins->GetOperand2()->GetOperandType() == BasicOperand::IMMF32)
        {
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand1();
            auto *imm2_op = (ImmF32Operand *)ins->GetOperand2();
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            
            // std::cout<<"imm1 float: "<<imm1<<std::endl;
            // std::string str_imm1 = ((MachineImmediateFloat*)imm1_op)->toString();
            // std::cout<<"imm1 string: "<<str_imm1<<std::endl;
            // int imm1_b = fimm_string2int(str_imm1);

            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }
            auto imm2 = imm2_op->GetFloatVal();
            Register fadd_reg2 = GetNewReg(FLOAT64);
            if(imm2 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg2, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm2_val = *(int *)&imm2;  // 将浮动类型转换为 int64_t
                Register midreg_imm2 = GetNewReg(INT64);
                int high_imm2 = (imm2_val + 0x800) >> 12;  // 高 20 位
                int low_imm2 = (imm2_val<<20)>>20;           // 低 12 位
                auto lui_instr2 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm2, high_imm2);
                cur_block->push_back(lui_instr2);
                auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm2, midreg_imm2, low_imm2);
                cur_block->push_back(addi_instr2);
                auto fmvwx_instr2=rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg2, midreg_imm2);
                cur_block->push_back(fmvwx_instr2);
            }
            



            // std::cout<<"imm2 float: "<<imm2<<std::endl;
            // std::string str_imm2 = ((MachineImmediateFloat*)imm2_op)->toString();
            // std::cout<<"imm2 string: "<<str_imm2<<std::endl;
            // int imm2_b = fimm_string2int(str_imm2);


            
            // Register temp_reg1 = cur_func->GetNewReg(INT64);
            // Register temp_reg2 = cur_func->GetNewReg(INT64);
            // Register temp_freg1 = cur_func->GetNewReg(FLOAT64);
            // Register temp_freg2 = cur_func->GetNewReg(FLOAT64);
            //auto load_imm_instr = rvconstructor->ConstructUImm(RISCV_FADD_S, rd, imm1+imm2);
            //cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LI, temp_reg1, imm1_b));


            // cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, temp_freg1, temp_reg1));

            // //cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LI, temp_reg2, imm2_b));
            // cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, temp_freg2, temp_reg2));
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FADD_S, rd, fadd_reg1, fadd_reg2));
            //COMMENT 可能需要改成FMADD.S R4
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMF32 &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand2();
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand1();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);

            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }

            cur_block->push_back(rvconstructor->ConstructR(RISCV_FADD_S, rd, fadd_reg1, rs));
            //cur_block->push_back(calc_instr);
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::IMMF32)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);
            
            //auto calc_instr = rvconstructor->ConstructIImm(RISCV_ADDI, rd, rs, imm);
            //cur_block->push_back(calc_instr);   

            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }

            cur_block->push_back(rvconstructor->ConstructR(RISCV_FADD_S, rd, fadd_reg1, rs));
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *rt_op = (RegOperand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);
            auto rt = GetllvmReg(rt_op->GetRegNo(), FLOAT64);
            auto calc_instr = rvconstructor->ConstructR(RISCV_FADD_S, rd, rs, rt);
            cur_block->push_back(calc_instr);   
        }
        else{
            std::cout<<"wrong"<<std::endl;
        }
        
    }
    else if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::FSUB){
        if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMF32 &&
                ins->GetOperand2()->GetOperandType() == BasicOperand::IMMF32)
        {
            
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand1();
            auto *imm2_op = (ImmF32Operand *)ins->GetOperand2();
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }


            auto imm2 = imm2_op->GetFloatVal();
            Register fadd_reg2 = GetNewReg(FLOAT64);
            if(imm2 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg2, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm2_val = *(int *)&imm2;  // 将浮动类型转换为 int64_t
                Register midreg_imm2 = GetNewReg(INT64);
                int high_imm2 = (imm2_val + 0x800) >> 12;  // 高 20 位
                int low_imm2 = (imm2_val<<20)>>20;           // 低 12 位
                auto lui_instr2 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm2, high_imm2);
                cur_block->push_back(lui_instr2);
                auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm2, midreg_imm2, low_imm2);
                cur_block->push_back(addi_instr2);
                auto fmvwx_instr2=rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg2, midreg_imm2);
                cur_block->push_back(fmvwx_instr2);
            }
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FSUB_S, rd, fadd_reg1, fadd_reg2));
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMF32 &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand2();
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand1();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);
            

            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }
            
            
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FSUB_S, rd, fadd_reg1, rs));

        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::IMMF32)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);

            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }

            cur_block->push_back(rvconstructor->ConstructR(RISCV_FSUB_S, rd, rs, fadd_reg1));
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *rt_op = (RegOperand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);
            auto rt = GetllvmReg(rt_op->GetRegNo(), FLOAT64);
            auto calc_instr = rvconstructor->ConstructR(RISCV_FSUB_S, rd, rs, rt);
            cur_block->push_back(calc_instr);   
        }
        else{
            std::cout<<"wrong"<<std::endl;
        }
    }
    else if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::FMUL){
        if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMF32 &&
                ins->GetOperand2()->GetOperandType() == BasicOperand::IMMF32)
        {
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand1();
            auto *imm2_op = (ImmF32Operand *)ins->GetOperand2();
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            
            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }



            auto imm2 = imm2_op->GetFloatVal();
            Register fadd_reg2 = GetNewReg(FLOAT64);
            if(imm2 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg2, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm2_val = *(int *)&imm2;  // 将浮动类型转换为 int64_t
                Register midreg_imm2 = GetNewReg(INT64);
                int high_imm2 = (imm2_val + 0x800) >> 12;  // 高 20 位
                int low_imm2 = (imm2_val<<20)>>20;           // 低 12 位
                auto lui_instr2 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm2, high_imm2);
                cur_block->push_back(lui_instr2);
                auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm2, midreg_imm2, low_imm2);
                cur_block->push_back(addi_instr2);
                auto fmvwx_instr2=rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg2, midreg_imm2);
                cur_block->push_back(fmvwx_instr2);
            }
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FMUL_S, rd, fadd_reg1, fadd_reg2));
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMF32 &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand2();
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand1();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);
            
            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }
            
            
            
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FMUL_S, rd, fadd_reg1, rs));

        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::IMMF32)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);

            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }

            cur_block->push_back(rvconstructor->ConstructR(RISCV_FMUL_S, rd, rs, fadd_reg1));
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *rt_op = (RegOperand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);
            auto rt = GetllvmReg(rt_op->GetRegNo(), FLOAT64);
            auto calc_instr = rvconstructor->ConstructR(RISCV_FMUL_S, rd, rs, rt);
            cur_block->push_back(calc_instr);   
        }
        else{
            std::cout<<"wrong"<<std::endl;
        }
    }
    else if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::FDIV){
        if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMF32 &&
                ins->GetOperand2()->GetOperandType() == BasicOperand::IMMF32)
        {
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand1();
            auto *imm2_op = (ImmF32Operand *)ins->GetOperand2();
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }


            auto imm2 = imm2_op->GetFloatVal();
            Register fadd_reg2 = GetNewReg(FLOAT64);
            if(imm2 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg2, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm2_val = *(int *)&imm2;  // 将浮动类型转换为 int64_t
                Register midreg_imm2 = GetNewReg(INT64);
                int high_imm2 = (imm2_val + 0x800) >> 12;  // 高 20 位
                int low_imm2 = (imm2_val<<20)>>20;           // 低 12 位
                auto lui_instr2 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm2, high_imm2);
                cur_block->push_back(lui_instr2);
                auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm2, midreg_imm2, low_imm2);
                cur_block->push_back(addi_instr2);
                auto fmvwx_instr2=rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg2, midreg_imm2);
                cur_block->push_back(fmvwx_instr2);
            }
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FDIV_S, rd, fadd_reg1, fadd_reg2));
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMF32 &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand2();
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand1();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);
            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }
            
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FDIV_S, rd, fadd_reg1, rs));

        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::IMMF32)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *imm1_op = (ImmF32Operand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);
            auto imm1 = imm1_op->GetFloatVal();
            Register fadd_reg1 = GetNewReg(FLOAT64);
            if(imm1 == 0.0)
            {
                cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, GetPhysicalReg(RISCV_x0)));
            }
            else{
                int imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
                Register midreg_imm1 = GetNewReg(INT64);
                int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
                auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                cur_block->push_back(lui_instr1);
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                cur_block->push_back(addi_instr1);
                auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, fadd_reg1, midreg_imm1);
                cur_block->push_back(fmvwx_instr1);
            }
            
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FDIV_S, rd, rs, fadd_reg1));
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
            ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *rt_op = (RegOperand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), FLOAT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), FLOAT64);
            auto rt = GetllvmReg(rt_op->GetRegNo(), FLOAT64);
            auto calc_instr = rvconstructor->ConstructR(RISCV_FDIV_S, rd, rs, rt);
            cur_block->push_back(calc_instr);   
        }
        else{
            std::cout<<"wrong"<<std::endl;
        }
    }
    else if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::MOD){
        if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
        ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
        {
            auto *imm1_op = (ImmI32Operand *)ins->GetOperand1();
            auto *imm2_op = (ImmI32Operand *)ins->GetOperand2();
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto imm1 = imm1_op->GetIntImmVal();
            auto imm2 = imm2_op->GetIntImmVal();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto load_imm_instr = rvconstructor->ConstructUImm(RISCV_LI, rd, imm1 % imm2);
            cur_block->push_back(load_imm_instr);
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
                ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand2();
            auto *i_op = (ImmI32Operand *)ins->GetOperand1();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);

            auto imm = i_op->GetIntImmVal();
            Register temp_reg = cur_func->GetNewReg(INT64);
            cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LI, temp_reg, imm)); // 加载立即数
            cur_block->push_back(rvconstructor->ConstructR(RISCV_REMW, rd, temp_reg, rs)); // 模运算
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
                ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *i_op = (ImmI32Operand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
            auto imm = i_op->GetIntImmVal();
            Register temp_reg = cur_func->GetNewReg(INT64);
            cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LI, temp_reg, imm)); // 加载立即数
            cur_block->push_back(rvconstructor->ConstructR(RISCV_REMW, rd, rs, temp_reg)); // 模运算
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
                ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *rt_op = (RegOperand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
            auto rt = GetllvmReg(rt_op->GetRegNo(), INT64);
            auto calc_instr = rvconstructor->ConstructR(RISCV_REMW, rd, rs, rt);
            cur_block->push_back(calc_instr);
        }
        else{
            std::cout << "mod: Invalid operands" << std::endl;
        }
    }
    else if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::SHL){
        //sysy中貌似没代码用
    }
    else if(ins->GetOpcode() == BasicInstruction::LLVMIROpcode::BITXOR){
        if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
        ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
        {
            auto *imm1_op = (ImmI32Operand *)ins->GetOperand1();
            auto *imm2_op = (ImmI32Operand *)ins->GetOperand2();
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto imm1 = imm1_op->GetIntImmVal();
            auto imm2 = imm2_op->GetIntImmVal();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto load_imm_instr = rvconstructor->ConstructUImm(RISCV_LI, rd, imm1 ^ imm2);
            cur_block->push_back(load_imm_instr);
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::IMMI32 &&
                ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand2();
            auto *i_op = (ImmI32Operand *)ins->GetOperand1();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);

            auto imm = i_op->GetIntImmVal();
            Register temp_reg = cur_func->GetNewReg(INT64);
            cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LI, temp_reg, imm)); // 加载立即数
            cur_block->push_back(rvconstructor->ConstructR(RISCV_XOR, rd, temp_reg, rs)); // 模运算
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
                ins->GetOperand2()->GetOperandType() == BasicOperand::IMMI32)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *i_op = (ImmI32Operand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
            auto imm = i_op->GetIntImmVal();
            Register temp_reg = cur_func->GetNewReg(INT64);
            cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LI, temp_reg, imm)); // 加载立即数
            cur_block->push_back(rvconstructor->ConstructR(RISCV_XOR, rd, rs, temp_reg)); // 模运算
        }
        else if (ins->GetOperand1()->GetOperandType() == BasicOperand::REG &&
                ins->GetOperand2()->GetOperandType() == BasicOperand::REG)
        {
            auto *rd_op = (RegOperand *)ins->GetResultOperand();
            auto *rs_op = (RegOperand *)ins->GetOperand1();
            auto *rt_op = (RegOperand *)ins->GetOperand2();
            auto rd = GetllvmReg(rd_op->GetRegNo(), INT64);
            auto rs = GetllvmReg(rs_op->GetRegNo(), INT64);
            auto rt = GetllvmReg(rt_op->GetRegNo(), INT64);
            auto calc_instr = rvconstructor->ConstructR(RISCV_XOR, rd, rs, rt);
            cur_block->push_back(calc_instr);
        }
        else{
            std::cout << "mod: Invalid operands" << std::endl;
        }
    }
    else{
        //comment: bitadd等等，不是所有的在llvm都有，这里只写了总的里面有的case
        std::cout<<"wrong opreation"<<std::endl;
    }
    

}

template <> void RiscV64Selector::ConvertAndAppend<IcmpInstruction *>(IcmpInstruction *ins) {//DONE
    //TODO("Implement this if you need");
    switch (ins->GetResult()->GetOperandType()) 
    {
        case BasicOperand::REG: 
        {
            auto res_op = (RegOperand *)ins->GetResult();
            auto res_reg = GetllvmReg(res_op->GetRegNo(), INT64);
            cmp_context[res_reg] = ins;
            //存储某个寄存器的结果是哪条指令比较的结果
            break;
        }
        default:
            Assert(false); 
            break;
    }
}

template <> void RiscV64Selector::ConvertAndAppend<FcmpInstruction *>(FcmpInstruction *ins) {//DONE
    //TODO("Implement this if you need");
    switch (ins->GetResult()->GetOperandType()) 
    {
        case BasicOperand::REG: 
        {
            auto res_op = (RegOperand *)ins->GetResult();
            auto res_reg = GetllvmReg(res_op->GetRegNo(), INT64);
            cmp_context[res_reg] = ins;
            //存储某个寄存器的结果是哪条指令比较的结果
            break;
        }
        default:
            Assert(false);
            break;
    }
}

template <> void RiscV64Selector::ConvertAndAppend<AllocaInstruction *>(AllocaInstruction *ins) {//DONE
    //TODO("Implement this if you need");
    switch (ins->GetResult()->GetOperandType()) 
    {
        case BasicOperand::REG: 
        {
            auto reg_operation = (RegOperand *)ins->GetResult();
            int byte_size = ins->GetAllocaSize() << 2;  // 4字节对齐
            llvm_riscv_allocas[reg_operation->GetRegNo()] = cur_offset;
            cur_offset = cur_offset + byte_size;
            break;
        }
        default:
            Assert(false);
            break;
    }
}

//Reference: https://github.com/yuhuifishash/SysY
template <> void RiscV64Selector::ConvertAndAppend<BrCondInstruction *>(BrCondInstruction *ins) {//DONE
    //TODO("Implement this if you need");
    //条件跳转
    int temp_result = ins->GetCond()->GetOperandType();
    Assert(temp_result == BasicOperand::REG);
    auto cond_reg = (RegOperand *)ins->GetCond();
    auto br_reg = GetllvmReg(cond_reg->GetRegNo(), INT64);
    auto cmp_ins = cmp_context[br_reg];
    int opcode;
    // 按两条语句算的,一条是比较一条是跳转
    // 最后会合成一条指令
    // %r12 = icmp slt i32 %r10,%r11
    // br i1 %r12, label %L7, label %L8
    // 前一条在icmp里完成，记录r12是result，指令%r12 = icmp slt i32 %r10,%r11在cmp_context里，即cmp_ins
    // 后一条是Brcond
    Register cmp_op1;
    Register cmp_op2;
    Register cmp_rd;
    //如果是icmp
    if(cmp_ins->GetOpcode() == BasicInstruction::ICMP)
    {
        auto icmp_ins = (IcmpInstruction *)cmp_ins;
        //lw			t0,4(sp)
	    //lw			t1,0(sp)
        //直接两条就够了，如果是fcmp需要三条
	    //bgt			t0,t1,.main_1
	    //jal			x0,.main_2
        //先处理第一个操作数op1
        if(icmp_ins->GetOp1()->GetOperandType() == BasicOperand::REG)   // 寄存器的情况，直接设置成寄存器
        {
            cmp_op1 = GetllvmReg(((RegOperand *)icmp_ins->GetOp1())->GetRegNo(), INT64);
        }
        else if(icmp_ins->GetOp1()->GetOperandType() == BasicOperand::IMMI32)   // 立即数的情况，加载进来
        {
            if(((ImmI32Operand *)icmp_ins->GetOp1())->GetIntImmVal() != 0)
            {
                cmp_op1 = GetNewReg(INT64);
                // //li指令 后续如果出问题了就改一下
                int imm = ((ImmI32Operand *)icmp_ins->GetOp1())->GetIntImmVal();

                auto li_instr = rvconstructor->ConstructUImm(RISCV_LI, cmp_op1, imm);
                cur_block->push_back(li_instr);
            }
            else
            {
                cmp_op1 = GetPhysicalReg(RISCV_x0);
            }
        }
        else
        {
            ERROR("Unexpected ICMP op1 type!!!");
        }
        
        //处理第二个操作数op2
        if (icmp_ins->GetOp2()->GetOperandType() == BasicOperand::REG)  // 寄存器的情况，直接设置成寄存器
        {
            cmp_op2 = GetllvmReg(((RegOperand *)icmp_ins->GetOp2())->GetRegNo(), INT64);
        } 
        else if (icmp_ins->GetOp2()->GetOperandType() == BasicOperand::IMMI32) // 立即数的情况，加载进来
        {
            if (((ImmI32Operand *)icmp_ins->GetOp2())->GetIntImmVal() != 0) 
            {
                cmp_op2 = GetNewReg(INT64);
                
                int imm = ((ImmI32Operand *)icmp_ins->GetOp2())->GetIntImmVal();

                auto li_instr = rvconstructor->ConstructUImm(RISCV_LI, cmp_op2, imm);
                cur_block->push_back(li_instr);

            } 
            else 
            {
                cmp_op2 = GetPhysicalReg(RISCV_x0);
            }
        } 
        else 
        {
            ERROR("Unexpected ICMP op2 type");
        }
        
        //更新操作码
        switch (icmp_ins->GetCompareCondition()) 
        {
        case BasicInstruction::eq:    // beq
            opcode = RISCV_BEQ;
            break;
        case BasicInstruction::ne:    // bne
            opcode = RISCV_BNE;
            break;
        case BasicInstruction::ugt:    // bgtu --p
            opcode = RISCV_BGTU;
            break;
        case BasicInstruction::uge:    // bgeu
            opcode = RISCV_BGEU;
            break;
        case BasicInstruction::ult:    // bltu
            opcode = RISCV_BLTU;
            break;
        case BasicInstruction::ule:    // bleu --p
            opcode = RISCV_BLEU;
            break;
        case BasicInstruction::sgt:    // bgt --p
            opcode = RISCV_BGT;
            break;
        case BasicInstruction::sge:    // bge
            opcode = RISCV_BGE;
            break;
        case BasicInstruction::slt:    // blt
            opcode = RISCV_BLT;
            break;
        case BasicInstruction::sle:    // ble --p
            opcode = RISCV_BLE;
            break;
        default:
            ERROR("Unexpected ICMP cond");
        }
    } 
    
    
    //如果是fcmp
    else if (cmp_ins->GetOpcode() == BasicInstruction::FCMP) 
    {
        auto fcmp_ins = (FcmpInstruction *)cmp_ins;
        //先处理第一个操作数op1
        if (fcmp_ins->GetOp1()->GetOperandType() == BasicOperand::REG) // 寄存器的情况，直接设置成寄存器
        {
            cmp_op1 = GetllvmReg(((RegOperand *)fcmp_ins->GetOp1())->GetRegNo(), FLOAT64);
        } 
        else if (fcmp_ins->GetOp1()->GetOperandType() == BasicOperand::IMMF32) // 立即数的情况，加载进来
        {
            cmp_op1 = GetNewReg(FLOAT64);
            auto cmp_oppre = GetNewReg(INT64);
            
            auto float_val = ((ImmF32Operand *)fcmp_ins->GetOp1())->GetFloatVal();

            auto imm1 = float_val;
            auto imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
            Register midreg_imm1 = GetNewReg(INT64);
            Register fadd_reg1 = GetNewReg(FLOAT64);
            int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
            int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位
            auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
            cur_block->push_back(lui_instr1);
            auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
            cur_block->push_back(addi_instr1);
            auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, cmp_op1, midreg_imm1);
            cur_block->push_back(fmvwx_instr1);
        } 
        else 
        {
            ERROR("Unexpected FCMP op1 type");
        }
        
        //处理第二个操作数op2
        if (fcmp_ins->GetOp2()->GetOperandType() == BasicOperand::REG) // 寄存器的情况，直接设置成寄存器
        {
            cmp_op2 = GetllvmReg(((RegOperand *)fcmp_ins->GetOp2())->GetRegNo(), FLOAT64);
        } 
        else if (fcmp_ins->GetOp2()->GetOperandType() == BasicOperand::IMMF32) // 立即数的情况，加载进来
        {
            cmp_op2 = GetNewReg(FLOAT64);
            auto cmp_oppre = GetNewReg(INT64);
            
            double float_val = ((ImmF32Operand *)fcmp_ins->GetOp2())->GetFloatVal();

            auto imm1 = float_val;
            auto imm1_val = *(int *)&imm1;  // 将浮动类型转换为 int64_t
            Register midreg_imm1 = GetNewReg(INT64);
            Register fadd_reg1 = GetNewReg(FLOAT64);
            int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
            int low_imm1 = (imm1_val<<20)>>20;           // 低 12 位

            auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
            cur_block->push_back(lui_instr1);

            auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
            cur_block->push_back(addi_instr1);
            
            auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, cmp_op2, midreg_imm1);
            cur_block->push_back(fmvwx_instr1);

        } 
        else 
        {
            ERROR("Unexpected FCMP op2 type");
        }
        
        // 还需要增加一个目标寄存器，并更新操作码
        // 先更新这个目标寄存器的结果为比较结果(就成为一个i类型)，为两个指令的操作
        // 操作码对应的逻辑应该让t0和0比为真就是原来的是真就可以
        // ...ft1
        // ...ft0
        // flt.s		t0,ft1,ft0
        // 最后使用这个数和x0比较，如果为真就跳转第一个label
        // bne			t0,x0,.main_1
        // jal			x0,.main_2
        cmp_rd = GetNewReg(INT64);
        switch (fcmp_ins->GetCompareCondition()) 
        {
        case BasicInstruction::OEQ:
        case BasicInstruction::UEQ:
            // FEQ.S
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FEQ_S, cmp_rd, cmp_op1, cmp_op2));
            opcode = RISCV_BNE;
            break;
        case BasicInstruction::OGT:
        case BasicInstruction::UGT:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FLT_S, cmp_rd, cmp_op2, cmp_op1));
            opcode = RISCV_BNE;
            break;
        case BasicInstruction::OGE:
        case BasicInstruction::UGE:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FLE_S, cmp_rd, cmp_op2, cmp_op1));
            opcode = RISCV_BNE;
            break;
        case BasicInstruction::OLT:
        case BasicInstruction::ULT:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FLT_S, cmp_rd, cmp_op1, cmp_op2));
            opcode = RISCV_BNE;
            break;
        case BasicInstruction::OLE:
        case BasicInstruction::ULE:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FLE_S, cmp_rd, cmp_op1, cmp_op2));
            opcode = RISCV_BNE;
            break;
        case BasicInstruction::ONE:
        case BasicInstruction::UNE:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_FEQ_S, cmp_rd, cmp_op1, cmp_op2));
            opcode = RISCV_BEQ;
            break;
        case BasicInstruction::ORD:
        case BasicInstruction::UNO:
        case BasicInstruction::TRUE:
        case BasicInstruction::FALSE:
        default:
            ERROR("Unexpected FCMP cond");
        }
        
        cmp_op1 = cmp_rd;
        cmp_op2 = GetPhysicalReg(RISCV_x0);
    } 
    else 
    {
        ERROR("No Cmp Before Br!!!");
    }
    Assert(ins->GetTrueLabel()->GetOperandType() == BasicOperand::LABEL);
    Assert(ins->GetFalseLabel()->GetOperandType() == BasicOperand::LABEL);
    auto true_label = (LabelOperand *)ins->GetTrueLabel();
    auto false_label = (LabelOperand *)ins->GetFalseLabel();
    auto br_true_ins = rvconstructor->ConstructBLabel(opcode, cmp_op1, cmp_op2, RiscVLabel(true_label->GetLabelNo()));
    cur_block->push_back(br_true_ins);
    auto br_else_ins = rvconstructor->ConstructJLabel(RISCV_JAL, GetPhysicalReg(RISCV_x0), RiscVLabel(false_label->GetLabelNo()));
    cur_block->push_back(br_else_ins);

}


template <> void RiscV64Selector::ConvertAndAppend<BrUncondInstruction *>(BrUncondInstruction *ins) {//DONE
    //TODO("Implement this if you need");
    //无条件跳转
    
    auto jmp_target = ((LabelOperand *)ins->GetDestLabel())->GetLabelNo();
    auto target_label = RiscVLabel(jmp_target);//无条件跳转，直接赋值跳转label即可
    auto jal_instr = rvconstructor->ConstructJLabel(RISCV_JAL, GetPhysicalReg(RISCV_x0), target_label);
    cur_block->push_back(jal_instr);

}

//Reference: https://github.com/yuhuifishash/SysY
template <> void RiscV64Selector::ConvertAndAppend<CallInstruction *>(CallInstruction *ins) {
    Assert(ins->GetRetType() == BasicInstruction::VOID || ins->GetResult()->GetOperandType() == BasicOperand::REG);

    int ireg_cnt = 0;
    int freg_cnt = 0;
    int stkpara_cnt = 0;
    //Reference: https://github.com/yuhuifishash/SysY
    if (ins->GetFunctionName() == std::string("llvm.memset.p0.i32")) 
    {
        // std::cout << "generating call instruction for llvm.memset.p0.i32" << std::endl;
        // 其实就是一个普通的call，把每一个参数都给放到对应的a寄存器中，但是存放参数的位置不一样，所以直接用一个特殊的判断完成
        // parameter 0
        int ptrreg_no = ((RegOperand *)ins->GetParameterList()[0].second)->GetRegNo();
        if (llvm_riscv_allocas.find(ptrreg_no) == llvm_riscv_allocas.end()) 
        {
            cur_block->push_back(rvconstructor->ConstructR(
                RISCV_ADD,
                GetPhysicalReg(RISCV_a0),
                GetllvmReg(ptrreg_no, INT64),
                GetPhysicalReg(RISCV_x0)
            ));
        } 
        else 
        {
            auto sp_offset = llvm_riscv_allocas[ptrreg_no];
            std::cout << "sp_offset: " << sp_offset << std::endl;
            auto ld_alloca = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a0), GetPhysicalReg(RISCV_sp), sp_offset);
            ((RiscV64Function *)cur_func)->AddAllocaIns(ld_alloca);
            cur_block->push_back(ld_alloca);
        }
        // parameters 1
        auto imm_op = (ImmI32Operand *)(ins->GetParameterList()[1].second);
        auto imm_val = imm_op->GetIntImmVal();
        Register target_reg = GetPhysicalReg(RISCV_a1);
        auto high_imm = (imm_val + 0x800) >> 12;
        auto low_imm = (imm_val << 20) >> 20;
        cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LUI, target_reg, high_imm));
        cur_block->push_back(rvconstructor->ConstructIImm(RISCV_ADDI, target_reg, target_reg, low_imm));
        // paramters 2
        auto param2Op = ins->GetParameterList()[2].second;
        if (param2Op->GetOperandType() == BasicOperand::IMMI32) 
        {
            int arr_sz = ((ImmI32Operand *)param2Op)->GetIntImmVal();
            Register target_reg = GetPhysicalReg(RISCV_a2);
            auto high_imm = (arr_sz + 0x800) >> 12;
            auto low_imm = (arr_sz << 20) >> 20;
            cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LUI, target_reg, high_imm));
            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_ADDI, target_reg, target_reg, low_imm));
        } 
        else 
        {
            int sizereg_no = ((RegOperand *)param2Op)->GetRegNo();
            if (llvm_riscv_allocas.find(sizereg_no) == llvm_riscv_allocas.end()) 
            {
                Register target_reg = GetPhysicalReg(RISCV_a2);
                Register source_reg = GetllvmReg(sizereg_no, INT64);
                cur_block->push_back(rvconstructor->ConstructR(RISCV_ADD, target_reg, source_reg, GetPhysicalReg(RISCV_x0)));
            } 
            else 
            {
                auto sp_offset = llvm_riscv_allocas[sizereg_no];
                auto ld_alloca = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a2), GetPhysicalReg(RISCV_sp), sp_offset);
                ((RiscV64Function *)cur_func)->AddAllocaIns(ld_alloca);
                cur_block->push_back(ld_alloca);
            }
        }
        // call
        cur_block->push_back(rvconstructor->ConstructCall(RISCV_CALL, "memset", 3, 0));
        //RiscV64Instruction *ConstructCall(int op, std::string funcname, int iregnum, int fregnum)
        return;
    }

    // Parameters放到寄存器里，并准备调用
    for (auto [type, arg_op] : ins->GetParameterList()) 
    {
        if (type == BasicInstruction::I32 || type == BasicInstruction::PTR) 
        {
            if (ireg_cnt < 8) 
            {
                if (arg_op->GetOperandType() == BasicOperand::REG) 
                {
                    auto arg_regop = (RegOperand *)arg_op;
                    auto arg_reg = GetllvmReg(arg_regop->GetRegNo(), INT64);
                    if (llvm_riscv_allocas.find(arg_regop->GetRegNo()) == llvm_riscv_allocas.end()) 
                    {
                        Register target_reg = GetPhysicalReg(RISCV_a0 + ireg_cnt);
                        cur_block->push_back(rvconstructor->ConstructIImm(RISCV_ADDI, target_reg, arg_reg, 0));
                    } 
                    else 
                    {
                        cur_block->push_back(rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a0 + ireg_cnt), GetPhysicalReg(RISCV_sp), llvm_riscv_allocas[arg_regop->GetRegNo()]));
                    }
                } 
                else if (arg_op->GetOperandType() == BasicOperand::IMMI32) 
                {
                    auto arg_immop = (ImmI32Operand *)arg_op;
                    auto imm_val = arg_immop->GetIntImmVal();
                    Register target_reg = GetPhysicalReg(RISCV_a0 + ireg_cnt);
                    cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LI, target_reg, imm_val));
                } 
                else if (arg_op->GetOperandType() == BasicOperand::GLOBAL) 
                {
                    auto mid_reg = GetNewReg(INT64);
                    auto arg_global = (GlobalOperand *)arg_op;
                    cur_block->push_back(rvconstructor->ConstructULabel(RISCV_LUI, mid_reg, RiscVLabel(arg_global->GetName(), true)));
                    cur_block->push_back(rvconstructor->ConstructILabel(RISCV_ADDI, GetPhysicalReg(RISCV_a0 + ireg_cnt), mid_reg, RiscVLabel(arg_global->GetName(), false)));
                } 
                else 
                {
                    ERROR("Unexpected Operand type");
                }
            }
            ireg_cnt++;
        } 
        else if (type == BasicInstruction::FLOAT32) 
        {
            if (freg_cnt < 8) 
            {
                if (arg_op->GetOperandType() == BasicOperand::REG) 
                {
                    auto arg_regop = (RegOperand *)arg_op;
                    auto arg_reg = GetllvmReg(arg_regop->GetRegNo(), FLOAT64);
                    Register target_reg = GetPhysicalReg(RISCV_fa0 + freg_cnt);
                    cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_S, target_reg, arg_reg));
                } 
                else if (arg_op->GetOperandType() == BasicOperand::IMMF32) 
                {
                    auto arg_immop = (ImmF32Operand *)arg_op;
                    auto arg_imm = arg_immop->GetFloatVal();
                    int imm_val = *(int *)&arg_imm;
                    Register int_reg = GetNewReg(INT64);
                    Register target_reg = GetPhysicalReg(RISCV_fa0 + freg_cnt);
                    int high_imm = (imm_val + 0x800) >> 12;
                    int low_imm = (imm_val << 20) >> 20;
                    cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LUI, int_reg, high_imm));
                    cur_block->push_back(rvconstructor->ConstructIImm(RISCV_ADDI, int_reg, int_reg, low_imm));
                    cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, target_reg, int_reg));
                } 
                else 
                {
                    ERROR("Unexpected Operand type");
                }
            }
            freg_cnt++;
        } 
        else if (type == BasicInstruction::DOUBLE) 
        {
            if (ireg_cnt < 8) 
            {
                if (arg_op->GetOperandType() == BasicOperand::REG) 
                {
                    auto arg_regop = (RegOperand *)arg_op;
                    auto arg_reg = GetllvmReg(arg_regop->GetRegNo(), FLOAT64);
                    cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_X_D, GetPhysicalReg(RISCV_a0 + ireg_cnt), arg_reg));
                } 
                else 
                {
                    ERROR("Unexpected Operand Type");
                }
            }
            ireg_cnt++;
        } 
        else 
        {
            ERROR("Unexpected parameter type %d", type);
        }
    };
    int i_spill = ireg_cnt > 8 ? ireg_cnt - 8 : 0;
    int f_spill = freg_cnt > 8 ? freg_cnt - 8 : 0;
    stkpara_cnt = i_spill + f_spill;
    //如果溢出处理溢出
    if (stkpara_cnt!= 0) 
    {
        ireg_cnt = freg_cnt = 0;
        int arg_off = 0;
        for (auto [type, arg_op] : ins->GetParameterList()) 
        {
            if (type == BasicInstruction::I32 || type == BasicInstruction::PTR) 
            {
                if (ireg_cnt < 8) 
                {
                    //不特殊处理，前面已经处理过
                } 
                else 
                {
                    if (arg_op->GetOperandType() == BasicOperand::REG) 
                    {
                        auto arg_regop = (RegOperand *)arg_op;
                        auto arg_reg = GetllvmReg(arg_regop->GetRegNo(), INT64);
                        if (llvm_riscv_allocas.find(arg_regop->GetRegNo()) == llvm_riscv_allocas.end()) 
                        {
                            cur_block->push_back(rvconstructor->ConstructSImm(RISCV_SD, arg_reg, GetPhysicalReg(RISCV_sp), arg_off));
                        } 
                        else 
                        {
                            auto sp_offset = llvm_riscv_allocas[arg_regop->GetRegNo()];
                            auto mid_reg = GetNewReg(INT64);
                            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_ADDI, mid_reg, GetPhysicalReg(RISCV_sp), sp_offset));
                            cur_block->push_back(rvconstructor->ConstructSImm(RISCV_SD, mid_reg, GetPhysicalReg(RISCV_sp), arg_off));
                        }
                    } 
                    else if (arg_op->GetOperandType() == BasicOperand::IMMI32) 
                    {
                        auto arg_immop = (ImmI32Operand *)arg_op;
                        auto imm_val = arg_immop->GetIntImmVal();
                        auto imm_reg = GetNewReg(INT64);
                        Register target_reg = imm_reg;
                        auto high_imm = ((imm_val + 0x800) >> 12);
                        auto low_imm = (imm_val << 20) >> 20;
                        cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LUI, target_reg, (high_imm << 12) >> 12));
                        cur_block->push_back(rvconstructor->ConstructIImm(RISCV_ADDI, target_reg, target_reg, low_imm));
                        cur_block->push_back(rvconstructor->ConstructSImm(RISCV_SD, imm_reg, GetPhysicalReg(RISCV_sp), arg_off));
                    } 
                    else if (arg_op->GetOperandType() == BasicOperand::GLOBAL) 
                    {
                        auto glb_reg1 = GetNewReg(INT64);
                        auto glb_reg2 = GetNewReg(INT64);
                        auto arg_glbop = (GlobalOperand *)arg_op;
                        cur_block->push_back(rvconstructor->ConstructULabel(RISCV_LUI, glb_reg1, RiscVLabel(arg_glbop->GetName(), true)));
                        cur_block->push_back(rvconstructor->ConstructILabel(RISCV_ADDI, glb_reg2, glb_reg1, RiscVLabel(arg_glbop->GetName(), false)));
                        cur_block->push_back(rvconstructor->ConstructSImm(RISCV_SD, glb_reg2, GetPhysicalReg(RISCV_sp), arg_off));
                    }
                    arg_off += 8;
                }
                ireg_cnt++;
            } 
            else if (type == BasicInstruction::FLOAT32) 
            {
                if (freg_cnt < 8) 
                {
                    //不特殊处理，前面已经处理过
                } 
                else 
                {
                    if (arg_op->GetOperandType() == BasicOperand::REG) 
                    {
                        auto arg_regop = (RegOperand *)arg_op;
                        auto arg_reg = GetllvmReg(arg_regop->GetRegNo(), FLOAT64);
                        cur_block->push_back(rvconstructor->ConstructSImm(RISCV_FSD, arg_reg, GetPhysicalReg(RISCV_sp), arg_off));
                    } 
                    else if (arg_op->GetOperandType() == BasicOperand::IMMF32) 
                    {
                        auto arg_immop = (ImmF32Operand *)arg_op;
                        auto arg_imm = arg_immop->GetFloatVal();
                        auto imm_reg = GetNewReg(INT64);
                        int imm_val = *(int *)&arg_imm;
                        Register target_reg = GetNewReg(FLOAT64);
                        int high_imm = (imm_val + 0x800) >> 12;
                        int low_imm = (imm_val << 20) >> 20;
                        cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LUI, imm_reg, high_imm));
                        cur_block->push_back(rvconstructor->ConstructIImm(RISCV_ADDI, imm_reg, imm_reg, low_imm));
                        cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_W_X, target_reg, imm_reg));
                        cur_block->push_back(rvconstructor->ConstructSImm(RISCV_SD, imm_reg, GetPhysicalReg(RISCV_sp), arg_off));
                    } 
                    else 
                    {
                        ERROR("Unexpected Operand type");
                    }
                    arg_off += 8;
                }
                freg_cnt++;
            } 
            else if (type == BasicInstruction::DOUBLE) 
            {
                if (ireg_cnt < 8) 
                {
                    
                } 
                else 
                {
                    if (arg_op->GetOperandType() == BasicOperand::REG) 
                    {
                        auto arg_regop = (RegOperand *)arg_op;
                        auto arg_reg = GetllvmReg(arg_regop->GetRegNo(), FLOAT64);
                        cur_block->push_back(rvconstructor->ConstructSImm(RISCV_FSD, arg_reg, GetPhysicalReg(RISCV_sp), arg_off));
                    } 
                    else 
                    {
                        ERROR("Unexpected Operand type");
                    }
                    arg_off += 8;
                }
                ireg_cnt++;
            } 
            else 
            {
                ERROR("Unexpected parameter type %d", type);
            }
        };
    }

    // Call Label
    auto call_funcname = ins->GetFunctionName();
    ireg_cnt = std::min(8,ireg_cnt);
    freg_cnt = std::min(8,freg_cnt);
    
    // Return Value
    auto return_type = ins->GetRetType();
    auto result_op = (RegOperand *)ins->GetResult();
    cur_block->push_back(rvconstructor->ConstructCall(RISCV_CALL, call_funcname, ireg_cnt, freg_cnt));
    cur_func->UpdateParaSize(stkpara_cnt * 8);
    if (return_type == BasicInstruction::I32) 
    {
        cur_block->push_back(rvconstructor->ConstructR(RISCV_ADD, GetllvmReg(result_op->GetRegNo(), INT64), GetPhysicalReg(RISCV_x0), GetPhysicalReg(RISCV_a0)));
    } 
    else if (return_type == BasicInstruction::FLOAT32) 
    {
        cur_block->push_back(rvconstructor->ConstructR2(RISCV_FMV_S, GetllvmReg(result_op->GetRegNo(), FLOAT64), GetPhysicalReg(RISCV_fa0)));
    } 
    else if (return_type == BasicInstruction::VOID) 
    {
        //无返回值，不用管
    } 
    else 
    {
        ERROR("Unexpected return type %d", return_type);
    }
}

//Reference: https://github.com/yuhuifishash/SysY
template <> void RiscV64Selector::ConvertAndAppend<RetInstruction *>(RetInstruction *ins) {
    // 检查返回值是否为空
    if (ins->GetRetVal()!= NULL) 
    {
        switch (ins->GetRetVal()->GetOperandType()) 
        {
            case BasicOperand::IMMI32: 
            {
                auto retimm_op = (ImmI32Operand *)ins->GetRetVal();
                auto imm = retimm_op->GetIntImmVal();
                auto retcopy_instr = rvconstructor->ConstructUImm(RISCV_LI, GetPhysicalReg(RISCV_a0), imm);
                cur_block->push_back(retcopy_instr);
                break;
            }
            case BasicOperand::IMMF32: 
            {
                // 实现浮点数的处理
                auto retimm_op = (ImmF32Operand *)ins->GetRetVal();
                auto t_imm1 = retimm_op->GetFloatVal();
                int t_imm1_val = *(int *)&t_imm1;
                Register t_midreg_imm1 = GetNewReg(INT64);
                int t_high_imm1 = (t_imm1_val + 0x800) >> 12;
                int t_low_imm1 = (t_imm1_val << 20) >> 20;
                auto t_lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, t_midreg_imm1, t_high_imm1);
                cur_block->push_back(t_lui_instr1);
                auto t_addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, t_midreg_imm1, t_midreg_imm1, t_low_imm1);
                cur_block->push_back(t_addi_instr1);
                auto t_fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, GetPhysicalReg(RISCV_fa0), t_midreg_imm1);
                cur_block->push_back(t_fmvwx_instr1);
                break;
            }
            case BasicOperand::REG: 
            {
                // 处理寄存器类型的返回值
                auto retreg_val = (RegOperand *)ins->GetRetVal();
                if (ins->GetType() == BasicInstruction::I32) 
                {
                    auto reg = GetllvmReg(retreg_val->GetRegNo(), INT64);
                    auto retcopy_instr = rvconstructor->ConstructR(RISCV_ADD, GetPhysicalReg(RISCV_a0), reg, GetPhysicalReg(RISCV_x0));
                    cur_block->push_back(retcopy_instr);
                } 
                else if (ins->GetType() == BasicInstruction::FLOAT32) 
                {
                    auto reg = GetllvmReg(retreg_val->GetRegNo(), FLOAT64);
                    auto retcopy_instr = rvconstructor->ConstructR2(RISCV_FMV_S, GetPhysicalReg(RISCV_fa0), reg);
                    cur_block->push_back(retcopy_instr);
                } 
                else 
                {
                    std::cout << "wrong type" << std::endl;
                }
                break;
            }
        }
    }
    // 构造并添加 JALR 指令，并根据指令类型设置返回类型
    auto ret_instr = rvconstructor->ConstructIImm(RISCV_JALR, GetPhysicalReg(RISCV_x0), GetPhysicalReg(RISCV_ra), 0);
    switch (ins->GetType()) 
    {
        case BasicInstruction::I32:
            ret_instr->setRetType(1);
            break;
        case BasicInstruction::FLOAT32:
            ret_instr->setRetType(2);
            break;
        default:
            ret_instr->setRetType(0);
            break;
    }
    cur_block->push_back(ret_instr);
}

//Reference: https://github.com/yuhuifishash/SysY
template <> void RiscV64Selector::ConvertAndAppend<FptosiInstruction *>(FptosiInstruction *ins) {
    //float->int，总的来说就是将一个浮点类型的寄存器或者立即数直接加载到一个结果寄存器里
    auto src_op = ins->GetSrc();
    auto dst_op = (RegOperand *)ins->GetResultReg();
    switch (src_op->GetOperandType()) {
        case BasicOperand::REG: 
        {
            auto regf = (RegOperand *)src_op;
            // 使用 FCVT.W.S 指令将浮点数转换为整数
            cur_block->push_back
            (
                rvconstructor->ConstructR2
                (
                    RISCV_FCVT_W_S, 
                    GetllvmReg(dst_op->GetRegNo(), INT64), 
                    GetllvmReg(regf->GetRegNo(), FLOAT64)
                )
            );
            break;
        }
        case BasicOperand::IMMF32: 
        {
            auto imm1_op = (ImmF32Operand *)src_op;    
            auto imm1 = imm1_op->GetFloatVal();
            int imm1_val = *(int *)&imm1;  // 将浮点数按int方式读取
            Register midreg_imm1 = GetNewReg(INT64);
            int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
            int low_imm1 = (imm1_val << 20) >> 20;  // 低 12 位
            auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
            cur_block->push_back(lui_instr1);
            auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
            cur_block->push_back(addi_instr1);
            auto ADD_instr1 = rvconstructor->ConstructR(RISCV_ADD, GetllvmReg(dst_op->GetRegNo(), INT64), midreg_imm1, GetPhysicalReg(RISCV_x0));
            cur_block->push_back(ADD_instr1);
            break;
        }
        default:
            ERROR("Unexpected Fptosi src type");
    }
}

//Reference: https://github.com/yuhuifishash/SysY
template <> void RiscV64Selector::ConvertAndAppend<SitofpInstruction *>(SitofpInstruction *ins) {
    //int->float
    auto src_op = ins->GetSrc();
    auto dst_op = (RegOperand *)ins->GetResultReg();
    switch (src_op->GetOperandType()) 
    {
        case BasicOperand::REG: 
        {
            // 源操作数是寄存器，使用 FCVT.S.W 指令进行转换
            auto regi = (RegOperand *)src_op;
            auto fmv = rvconstructor->ConstructR2(
                RISCV_FCVT_S_W, 
                GetllvmReg(dst_op->GetRegNo(), FLOAT64), 
                GetllvmReg(regi->GetRegNo(), INT64)
            );
            cur_block->push_back(fmv);
            break;
        }
        case BasicOperand::IMMI32: 
        {
            // 源操作数是立即数，先加载立即数到寄存器，再使用 FCVT.S.W
            auto imm_op = (ImmI32Operand *)src_op;
            auto imm1 = imm_op->GetIntImmVal();
            auto temp_regf = GetllvmReg(dst_op->GetRegNo(), FLOAT64);
            int imm1_val = *(int *)&imm1;
            Register midreg_imm1 = GetNewReg(INT64);
            int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
            int low_imm1 = (imm1_val << 20) >> 20;  // 低 12 位
            auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
            cur_block->push_back(lui_instr1);
            auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
            cur_block->push_back(
                rvconstructor->ConstructR2(RISCV_FCVT_S_W, GetllvmReg(dst_op->GetRegNo(), FLOAT64), midreg_imm1)
            );
            break;
        }
        default:
            ERROR("Unexpected Sitofp src type");
    }
}

//Reference: https://github.com/yuhuifishash/SysY
template <> void RiscV64Selector::ConvertAndAppend<ZextInstruction *>(ZextInstruction *ins) {

    // 检查源操作数和目标操作数的操作数类型是否为寄存器
    Assert(ins->GetSrc()->GetOperandType() == BasicOperand::REG);
    Assert(ins->GetDst()->GetOperandType() == BasicOperand::REG);

    auto ext_reg = GetllvmReg(((RegOperand *)ins->GetSrc())->GetRegNo(), INT64);
    auto cmp_ins = cmp_context[ext_reg];
    Assert(cmp_ins!= nullptr);
    auto result_reg = GetllvmReg(((RegOperand *)ins->GetDst())->GetRegNo(), INT64);

    switch (cmp_ins->GetOpcode()) 
    {
        case BasicInstruction::ICMP: 
        {
            auto icmp_ins = (IcmpInstruction *)cmp_ins;
            auto op1 = icmp_ins->GetOp1();
            auto op2 = icmp_ins->GetOp2();
            auto cur_cond = icmp_ins->GetCompareCondition();
            Register reg_1, reg_2;
            
            if (op1->GetOperandType() == BasicOperand::IMMI32) 
            {
                std::swap(op1, op2);
                switch (cur_cond) {
                    case BasicInstruction::sgt: cur_cond = BasicInstruction::slt; break;
                    case BasicInstruction::sge: cur_cond = BasicInstruction::sle; break;
                    case BasicInstruction::slt: cur_cond = BasicInstruction::sgt; break;
                    case BasicInstruction::sle: cur_cond = BasicInstruction::sge; break;
                    case BasicInstruction::ugt:
                    case BasicInstruction::uge:
                    case BasicInstruction::ult:
                    case BasicInstruction::ule:
                        ERROR("Unexpected ICMP cond");
                }
            }

            switch (op1->GetOperandType()) 
            {
                case BasicOperand::IMMI32: 
                {
                    Assert(op2->GetOperandType() == BasicOperand::IMMI32);
                    int op1_val = ((ImmI32Operand *)op1)->GetIntImmVal();
                    int op2_val = ((ImmI32Operand *)op2)->GetIntImmVal();
                    int rval;
                    switch (cur_cond) 
                    {
                        case BasicInstruction::eq: rval = (op1_val == op2_val); break;
                        case BasicInstruction::ne: rval = (op1_val!= op2_val); break;
                        case BasicInstruction::sgt: rval = (op1_val > op2_val); break;
                        case BasicInstruction::sge: rval = (op1_val >= op2_val); break;
                        case BasicInstruction::slt: rval = (op1_val < op2_val); break;
                        case BasicInstruction::sle: rval = (op1_val <= op2_val); break;
                        case BasicInstruction::ugt:
                        case BasicInstruction::uge:
                        case BasicInstruction::ult:
                        case BasicInstruction::ule:
                            ERROR("Unexpected ICMP cond");
                    }
                    cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LI, result_reg, rval));
                    return;
                }
                case BasicOperand::REG:
                    Assert(op2->GetOperandType() == BasicOperand::IMMI32 || op2->GetOperandType() == BasicOperand::REG);
                    break;
                default:
                    ERROR("Unexpected op1 type");
            }

            if (op2->GetOperandType() == BasicOperand::IMMI32) 
            {
                Assert(op1->GetOperandType() == BasicOperand::REG);
                auto op1_reg = GetllvmReg(((RegOperand *)op1)->GetRegNo(), INT64);
                if (((ImmI32Operand *)op2)->GetIntImmVal() == 0) 
                {
                    auto not_reg = GetNewReg(INT64);
                    switch (cur_cond) 
                    {
                        case BasicInstruction::eq:
                            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_SLTIU, result_reg, op1_reg, 1));
                            return;
                        case BasicInstruction::ne:
                            cur_block->push_back(
                                rvconstructor->ConstructR(RISCV_SLTU, result_reg, GetPhysicalReg(RISCV_x0), op1_reg));
                                // 0和比较结果比，如果小就是1，又因为是无符号比较，所以只要有数就是1
                            return;
                        case BasicInstruction::sgt:
                            cur_block->push_back(
                                rvconstructor->ConstructR(RISCV_SLT, result_reg, GetPhysicalReg(RISCV_x0), op1_reg));
                            return;
                        case BasicInstruction::sge:
                            cur_block->push_back(
                                rvconstructor->ConstructR(RISCV_SLT, not_reg, op1_reg, GetPhysicalReg(RISCV_x0)));
                            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_XORI, result_reg, not_reg, 1));
                            return;
                        case BasicInstruction::slt:
                            cur_block->push_back(
                                rvconstructor->ConstructR(RISCV_SLT, result_reg, op1_reg, GetPhysicalReg(RISCV_x0)));
                            return;
                        case BasicInstruction::sle:
                            cur_block->push_back(
                                rvconstructor->ConstructR(RISCV_SLT, not_reg, GetPhysicalReg(RISCV_x0), op1_reg));
                            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_XORI, result_reg, not_reg, 1));
                            return;
                        case BasicInstruction::ugt:
                        case BasicInstruction::uge:
                        case BasicInstruction::ult:
                        case BasicInstruction::ule:
                            ERROR("Unexpected ICMP cond");
                    }
                } 
                else if (cur_cond == BasicInstruction::slt) 
                {
                    int op2_imm = ((ImmI32Operand *)op2)->GetIntImmVal();
                    cur_block->push_back(rvconstructor->ConstructIImm(RISCV_SLTI, result_reg, op1_reg, op2_imm));
                    return;
                } 
                else if (cur_cond == BasicInstruction::ult) 
                {
                    int op2_imm = ((ImmI32Operand *)op2)->GetIntImmVal();
                    cur_block->push_back(rvconstructor->ConstructIImm(RISCV_SLTIU, result_reg, op1_reg, op2_imm));
                    return;
                }
                auto li_instr = rvconstructor->ConstructUImm(RISCV_LI, GetNewReg(INT64), ((ImmI32Operand *)op2)->GetIntImmVal());
                cur_block->push_back(li_instr);
            }

            Assert(op1->GetOperandType() == BasicOperand::REG);
            reg_1 = GetllvmReg(((RegOperand *)op1)->GetRegNo(), INT64);
            if (op2->GetOperandType() == BasicOperand::REG) 
            {
                reg_2 = GetllvmReg(((RegOperand *)op2)->GetRegNo(), INT64);
            }
            auto mid_reg = GetNewReg(INT64);
            switch (cur_cond) 
            {
                case BasicInstruction::eq:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_SUBW, mid_reg, reg_1, reg_2));
                    cur_block->push_back(rvconstructor->ConstructIImm(RISCV_SLTIU, result_reg, mid_reg, 1));
                    return;
                case BasicInstruction::ne:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_SUBW, mid_reg, reg_1, reg_2));
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_SLTU, result_reg, GetPhysicalReg(RISCV_x0), mid_reg));
                    return;
                case BasicInstruction::sgt:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, result_reg, reg_2, reg_1));
                    return;
                case BasicInstruction::sge:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, mid_reg, reg_1, reg_2));
                    cur_block->push_back(rvconstructor->ConstructIImm(RISCV_XORI, result_reg, mid_reg, 1));
                    return;
                case BasicInstruction::slt:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, result_reg, reg_1, reg_2));
                    return;
                case BasicInstruction::sle:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, mid_reg, reg_2, reg_1));
                    cur_block->push_back(rvconstructor->ConstructIImm(RISCV_XORI, result_reg, mid_reg, 1));
                    return;
                case BasicInstruction::ugt:
                case BasicInstruction::uge:
                case BasicInstruction::ult:
                case BasicInstruction::ule:
                default:
                    ERROR("Unexpected ICMP cond");
            }
            break;
        }
        case BasicInstruction::FCMP: 
        {
            auto fcmp_ins = (FcmpInstruction *)cmp_ins;
            Register cmp_op1, cmp_op2;

            auto handleImmF32 = [&](BasicOperand *op, Register &cmp_op) 
            {
                if (op->GetOperandType() == BasicOperand::IMMF32) 
                {
                    cmp_op = GetNewReg(FLOAT64);
                    float float_val = ((ImmF32Operand *)op)->GetFloatVal();
                    int imm1_val = *(int *)&float_val;  
                    Register midreg_imm1 = GetNewReg(INT64);
                    int high_imm1 = (imm1_val + 0x800) >> 12;  // 高 20 位
                    int low_imm1 = (imm1_val << 20) >> 20;  // 低 12 位
                    auto lui_instr1 = rvconstructor->ConstructUImm(RISCV_LUI, midreg_imm1, high_imm1);
                    cur_block->push_back(lui_instr1);
                    auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, midreg_imm1, midreg_imm1, low_imm1);
                    cur_block->push_back(addi_instr1);
                    auto fmvwx_instr1 = rvconstructor->ConstructR2(RISCV_FMV_W_X, cmp_op, midreg_imm1);
                    cur_block->push_back(fmvwx_instr1);
                } 
                else 
                {
                    ERROR("Unexpected FCMP op type");
                }
            };

            switch (fcmp_ins->GetOp1()->GetOperandType()) 
            {
                case BasicOperand::REG:
                    cmp_op1 = GetllvmReg(((RegOperand *)fcmp_ins->GetOp1())->GetRegNo(), FLOAT64);
                    break;
                case BasicOperand::IMMF32:
                    handleImmF32(fcmp_ins->GetOp1(), cmp_op1);
                    break;
                default:
                    ERROR("Unexpected FCMP op1 type");
            }

            switch (fcmp_ins->GetOp2()->GetOperandType()) 
            {
                case BasicOperand::REG:
                    cmp_op2 = GetllvmReg(((RegOperand *)fcmp_ins->GetOp2())->GetRegNo(), FLOAT64);
                    break;
                case BasicOperand::IMMF32:
                    handleImmF32(fcmp_ins->GetOp2(), cmp_op2);
                    break;
                default:
                    ERROR("Unexpected FCMP op2 type");
            }

            switch (fcmp_ins->GetCompareCondition()) 
            {
                case BasicInstruction::OEQ:
                case BasicInstruction::UEQ:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_FEQ_S, result_reg, cmp_op1, cmp_op2));
                    break;
                case BasicInstruction::OGT:
                case BasicInstruction::UGT:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_FLT_S, result_reg, cmp_op2, cmp_op1));
                    break;
                case BasicInstruction::OGE:
                case BasicInstruction::UGE:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_FLE_S, result_reg, cmp_op2, cmp_op1));
                    break;
                case BasicInstruction::OLT:
                case BasicInstruction::ULT:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_FLT_S, result_reg, cmp_op1, cmp_op2));
                    break;
                case BasicInstruction::OLE:
                case BasicInstruction::ULE:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_FLE_S, result_reg, cmp_op1, cmp_op2));
                    break;
                case BasicInstruction::ONE:
                case BasicInstruction::UNE:
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_FEQ_S, result_reg, cmp_op1, cmp_op2));
                    break;
                case BasicInstruction::ORD:
                case BasicInstruction::UNO:
                case BasicInstruction::TRUE:
                case BasicInstruction::FALSE:
                default:
                    ERROR("Unexpected FCMP cond");
            }
            break;
        }
        default:
            ERROR("Unexpected ins Before zext");
    }
}

//Reference: https://github.com/yuhuifishash/SysY
template <> void RiscV64Selector::ConvertAndAppend<GetElementptrInstruction *>(GetElementptrInstruction *ins) {
    auto global_op = (GlobalOperand *)ins->GetPtrVal();
    auto result_op = (RegOperand *)ins->GetResult();
    int dimsize = 1;
    for (auto size : ins->GetDims()) {
        dimsize *= size;
    }
    int const_offset = 0;
    auto offset_reg = GetNewReg(INT64);
    auto result_reg = GetllvmReg(result_op->GetRegNo(), INT64);
    int offset_reg_assigned = 0; //已经更新过了offset对应的寄存器

    // 处理索引操作数
    for (int i = 0; i < ins->GetIndexes().size(); ++i) 
    {
        auto index_op = ins->GetIndexes()[i];
        if (index_op->GetOperandType() == BasicOperand::IMMI32) 
        {
            const_offset += (((ImmI32Operand *)index_op)->GetIntImmVal()) * dimsize;
            //不断更新，从大的size基础上计算
        } 
        else 
        {
            auto index_reg = GetllvmReg(((RegOperand *)index_op)->GetRegNo(), INT64);
            if (dimsize!= 1) 
            {
                auto this_inc = GetNewReg(INT64);
                auto dimsize_reg = GetNewReg(INT64);
                cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LI, dimsize_reg, dimsize));
                cur_block->push_back(rvconstructor->ConstructR(RISCV_MUL, this_inc, index_reg, dimsize_reg));
                if (offset_reg_assigned == 0) 
                {
                    offset_reg_assigned = 1;
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_ADD, offset_reg, this_inc, GetPhysicalReg(RISCV_x0)));
                } 
                else 
                {
                    auto new_offset = GetNewReg(INT64);
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_ADD, new_offset, offset_reg, this_inc));
                    offset_reg = new_offset;
                }
            } 
            else 
            {
                if (offset_reg_assigned == 0) 
                {
                    offset_reg_assigned = 1;
                    auto mv_instr = rvconstructor->ConstructR(RISCV_ADD, offset_reg, index_reg, GetPhysicalReg(RISCV_x0));
                    cur_block->push_back(mv_instr);
                } 
                else 
                {
                    auto new_offset = GetNewReg(INT64);
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_ADD, new_offset, offset_reg, index_reg));
                    offset_reg = new_offset;
                }
            }
        }
        if (i < ins->GetDims().size()) 
        {
            dimsize /= ins->GetDims()[i];
        }
    }

    bool all_imm = false;//是否需要按字节偏移（需要除4）
    //更新offset_reg的值
    if (const_offset!= 0) 
    {
        if (offset_reg_assigned == 0) 
        {
            offset_reg_assigned = 1;
            all_imm = true;
            int value = const_offset * 4;
            cur_block->push_back(rvconstructor->ConstructUImm(RISCV_LI, offset_reg, value));
        } 
        else 
        {
            auto new_offset = GetNewReg(INT64);
            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_ADDI, new_offset, offset_reg, const_offset));
            offset_reg = new_offset;
        }
    }

    // 处理指针操作数
    switch (ins->GetPtrVal()->GetOperandType()) 
    {
        case BasicOperand::REG: {
            // Lazy("Not tested");
            auto ptr_op = (RegOperand *)ins->GetPtrVal();
            auto offsetfull_reg = GetNewReg(INT64);
            if (offset_reg_assigned) {
                auto sll_instr = rvconstructor->ConstructIImm(RISCV_SLLI, offsetfull_reg, offset_reg, 2);
                if (all_imm) 
                {
                    offsetfull_reg = offset_reg;
                }
                Register base_reg;
                if (llvm_riscv_allocas.find(ptr_op->GetRegNo()) == llvm_riscv_allocas.end()) 
                {
                    base_reg = GetllvmReg(ptr_op->GetRegNo(), INT64);
                    if (!all_imm) 
                    {
                        cur_block->push_back(sll_instr);
                    }
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_ADD, result_reg, base_reg, offsetfull_reg));
                } 
                else 
                {
                    auto sp_offset = llvm_riscv_allocas[ptr_op->GetRegNo()];
                    auto base_reg = GetNewReg(INT64);
                    auto load_basereg_instr = rvconstructor->ConstructIImm(RISCV_ADDI, base_reg, GetPhysicalReg(RISCV_sp), sp_offset);
                    ((RiscV64Function *)cur_func)->AddAllocaIns(load_basereg_instr);
                    cur_block->push_back(load_basereg_instr);
                    if (!all_imm) 
                    {
                        cur_block->push_back(sll_instr);
                    }
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_ADD, result_reg, base_reg, offsetfull_reg));
                }
            } 
            else 
            {
                if (llvm_riscv_allocas.find(ptr_op->GetRegNo()) == llvm_riscv_allocas.end()) 
                {
                    Register src_reg = GetllvmReg(ptr_op->GetRegNo(), INT64);
                    cur_block->push_back(rvconstructor->ConstructR(RISCV_ADD, result_reg, src_reg, GetPhysicalReg(RISCV_x0)));
                } 
                else 
                {
                    auto sp_offset = llvm_riscv_allocas[ptr_op->GetRegNo()];
                    auto load_basereg_instr =
                        rvconstructor->ConstructIImm(RISCV_ADDI, result_reg, GetPhysicalReg(RISCV_sp), sp_offset);
                    ((RiscV64Function *)cur_func)->AddAllocaIns(load_basereg_instr);
                    cur_block->push_back(load_basereg_instr);
                }
            }
            break;
        }
        case BasicOperand::GLOBAL: {
            if (offset_reg_assigned) 
            {
                auto basehi_reg = GetNewReg(INT64);
                auto basefull_reg = GetNewReg(INT64);
                auto offsetfull_reg = GetNewReg(INT64);
                auto lui_instr =
                    rvconstructor->ConstructULabel(RISCV_LUI, basehi_reg, RiscVLabel(global_op->GetName(), true));
                auto addi_instr = rvconstructor->ConstructILabel(RISCV_ADDI, basefull_reg, basehi_reg,
                                                       RiscVLabel(global_op->GetName(), false));
                auto sll_instr = rvconstructor->ConstructIImm(RISCV_SLLI, offsetfull_reg, offset_reg, 2);
                if (all_imm) {
                    offsetfull_reg = offset_reg;
                }
                cur_block->push_back(lui_instr);
                cur_block->push_back(addi_instr);
                if (!all_imm) {
                    cur_block->push_back(sll_instr);
                }
                cur_block->push_back(rvconstructor->ConstructR(RISCV_ADD, result_reg, basefull_reg, offsetfull_reg));
            } 
            else 
            {
                auto result_hi_reg = GetNewReg(INT64);
                auto lui_instr =
                    rvconstructor->ConstructULabel(RISCV_LUI, result_hi_reg, RiscVLabel(global_op->GetName(), true));
                auto addi_instr = rvconstructor->ConstructILabel(RISCV_ADDI, result_reg, result_hi_reg,
                                                       RiscVLabel(global_op->GetName(), false));
                cur_block->push_back(lui_instr);
                cur_block->push_back(addi_instr);
            }
            break;
        }
        default:
            ERROR("Unexpected OperandType");
    }
}

template <> void RiscV64Selector::ConvertAndAppend<PhiInstruction *>(PhiInstruction *ins) {
    TODO("PHI");
}

template <> void RiscV64Selector::ConvertAndAppend<Instruction>(Instruction inst) {
    switch (inst->GetOpcode()) {
    case BasicInstruction::LOAD:
        ConvertAndAppend<LoadInstruction *>((LoadInstruction *)inst);
        break;
    case BasicInstruction::STORE:
        ConvertAndAppend<StoreInstruction *>((StoreInstruction *)inst);
        break;
    case BasicInstruction::ADD:
    case BasicInstruction::SUB:
    case BasicInstruction::MUL:
    case BasicInstruction::DIV:
    case BasicInstruction::FADD:
    case BasicInstruction::FSUB:
    case BasicInstruction::FMUL:
    case BasicInstruction::FDIV:
    case BasicInstruction::MOD:
    case BasicInstruction::SHL:
    case BasicInstruction::BITXOR:
        ConvertAndAppend<ArithmeticInstruction *>((ArithmeticInstruction *)inst);
        break;
    case BasicInstruction::ICMP:
        ConvertAndAppend<IcmpInstruction *>((IcmpInstruction *)inst);
        break;
    case BasicInstruction::FCMP:
        ConvertAndAppend<FcmpInstruction *>((FcmpInstruction *)inst);
        break;
    case BasicInstruction::ALLOCA:
        ConvertAndAppend<AllocaInstruction *>((AllocaInstruction *)inst);
        break;
    case BasicInstruction::BR_COND:
        ConvertAndAppend<BrCondInstruction *>((BrCondInstruction *)inst);
        break;
    case BasicInstruction::BR_UNCOND:
        ConvertAndAppend<BrUncondInstruction *>((BrUncondInstruction *)inst);
        break;
    case BasicInstruction::RET:
        ConvertAndAppend<RetInstruction *>((RetInstruction *)inst);
        break;
    case BasicInstruction::ZEXT:
        ConvertAndAppend<ZextInstruction *>((ZextInstruction *)inst);
        break;
    case BasicInstruction::FPTOSI:
        ConvertAndAppend<FptosiInstruction *>((FptosiInstruction *)inst);
        break;
    case BasicInstruction::SITOFP:
        ConvertAndAppend<SitofpInstruction *>((SitofpInstruction *)inst);
        break;
    case BasicInstruction::GETELEMENTPTR:
        ConvertAndAppend<GetElementptrInstruction *>((GetElementptrInstruction *)inst);
        break;
    case BasicInstruction::CALL:
        ConvertAndAppend<CallInstruction *>((CallInstruction *)inst);
        break;
    case BasicInstruction::PHI:
        ConvertAndAppend<PhiInstruction *>((PhiInstruction *)inst);
        break;
    default:
        ERROR("Unknown LLVM IR instruction");
    }
}

void RiscV64Selector::SelectInstructionAndBuildCFG() {
    // 与中间代码生成一样, 如果你完全无从下手, 可以先看看输出是怎么写的
    // 即riscv64gc/instruction_print/*  common/machine_passes/machine_printer.h

    // 指令选择除了一些函数调用约定必须遵守的情况需要物理寄存器，其余情况必须均为虚拟寄存器
    dest->global_def = IR->global_def;
    // 遍历每个LLVM IR函数
    for (auto [defI,cfg] : IR->llvm_cfg) {
        if(cfg == nullptr){
            ERROR("LLVMIR CFG is Empty, you should implement BuildCFG in MidEnd first");
        }
        std::string name = cfg->function_def->GetFunctionName();

        cur_func = new RiscV64Function(name);
        cur_func->SetParent(dest);
        // 你可以使用cur_func->GetNewRegister来获取新的虚拟寄存器
        dest->functions.push_back(cur_func);

        auto cur_mcfg = new MachineCFG;
        cur_func->SetMachineCFG(cur_mcfg);

        // 清空指令选择状态(可能需要自行添加初始化操作)
        ClearFunctionSelectState();

        // TODO: 添加函数参数(推荐先阅读一下riscv64_lowerframe.cc中的代码和注释)
        // See MachineFunction::AddParameter()
        //TODO("Add function parameter if you need");
        //Reference: https://github.com/yuhuifishash/SysY
        
        for (int i = 0; i < defI->formals.size(); ++i) {
            MachineDataType type;
            if (defI->formals[i] == BasicInstruction::LLVMType::I32 || defI->formals[i] == BasicInstruction::LLVMType::PTR) {
                type = INT64;
            } else if (defI->formals[i] == BasicInstruction::LLVMType::FLOAT32) {
                type = FLOAT64;
            } else {
                ERROR("Unknown llvm type");
            }            
            cur_func->AddParameter(GetllvmReg(((RegOperand *)defI->formals_reg[i])->GetRegNo(),type));
            
        }



        // 遍历每个LLVM IR基本块
        for (auto [id, block] : *(cfg->block_map)) {
            cur_block = new RiscV64Block(id);
            // 将新块添加到Machine CFG中
            cur_mcfg->AssignEmptyNode(id, cur_block);
            cur_func->UpdateMaxLabel(id);

            cur_block->setParent(cur_func);
            cur_func->blocks.push_back(cur_block);

            // 指令选择主要函数, 请注意指令选择时需要维护变量cur_offset
            // 遍历进行指令选择
            for (auto instruction : block->Instruction_list) {
                // Log("Selecting Instruction");
                ConvertAndAppend<Instruction>(instruction);
            }
        }

        // RISCV 8字节对齐（）
        if (cur_offset % 8 != 0) {
            cur_offset = ((cur_offset + 7) / 8) * 8;
        }
        cur_func->SetStackSize(cur_offset + cur_func->GetParaSize());
        ((RiscV64Function *)cur_func)->AddParameterSize(cur_func->GetParaSize());
        // 控制流图连边
        for (int i = 0; i < cfg->G.size(); i++) {
            const auto &arcs = cfg->G[i];
            for (auto arc : arcs) {
                cur_mcfg->MakeEdge(i, arc->block_id);
            }
        }
    }
}

void RiscV64Selector::ClearFunctionSelectState() { cur_offset = 0; llvmreg_reuse_table.clear(); llvm_riscv_allocas.clear();cmp_context.clear();}


Register RiscV64Selector::GetllvmReg(int ir_reg, MachineDataType type) {
    if(llvmreg_reuse_table.find(ir_reg) != llvmreg_reuse_table.end())
    {   
        return llvmreg_reuse_table[ir_reg];
    }
    else
    {
        llvmreg_reuse_table[ir_reg]=(cur_func->GetNewRegister(type.data_type, type.data_length));
        return llvmreg_reuse_table[ir_reg];
    }
}

Register RiscV64Selector::GetNewReg(MachineDataType type, bool save_across_call) {
    Register temp = cur_func->GetNewRegister(type.data_type, type.data_length, save_across_call);
    return temp;
}