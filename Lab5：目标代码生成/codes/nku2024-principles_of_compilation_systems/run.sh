#!/bin/bash

# Step 1: Compile SysY code to LLVM IR
./bin/SysYc -llvm -o mem2reg_test.out.ll mem2reg_test.sy -O1

# Step 2: Compile the LLVM IR to an object file
clang-15 -w mem2reg_test.out.ll -c -o tmp.o

# Step 3: Link the object file with the static library
clang-15 -static tmp.o lib/libsysy_x86.a

# Step 4: Run the generated executable with input from parser_test.in
./a.out < adce_test.in

# Step 5: Output the return code of the main function
echo "Program return code: $?"
