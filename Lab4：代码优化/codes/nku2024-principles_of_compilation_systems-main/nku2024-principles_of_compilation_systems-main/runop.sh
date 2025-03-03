#!/bin/bash

./bin/SysYc -llvm -o try_target.out.ll try_target.sy -O1

./bin/SysYc -S -o try_target.out.s try_target.sy -O1

clang-15 -S --target=riscv64 -o try_target_right.s try_target.out.ll

riscv64-unknown-linux-gnu-gcc try_target.out.s -c -o try_target.o -w

riscv64-unknown-linux-gnu-gcc -static try_target.o lib/libsysy_rv.a

#./a.out < adce_test.in
qemu-riscv64 ./a.out < try_target_in > try_target_out
# Step 5: Output the return code of the main function
echo ""
echo "Program return code: $?"