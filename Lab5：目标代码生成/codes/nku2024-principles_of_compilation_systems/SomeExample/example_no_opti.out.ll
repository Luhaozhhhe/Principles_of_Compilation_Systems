declare i32 @getint()
declare i32 @getch()
declare float @getfloat()
declare i32 @getarray(ptr)
declare i32 @getfarray(ptr)
declare void @putint(i32)
declare void @putch(i32)
declare void @putfloat(float)
declare void @putarray(i32,ptr)
declare void @putfarray(i32,ptr)
declare void @_sysy_starttime(i32)
declare void @_sysy_stoptime(i32)
declare void @llvm.memset.p0.i32(ptr,i8,i32,i1)
declare i32 @llvm.umax.i32(i32,i32)
declare i32 @llvm.umin.i32(i32,i32)
declare i32 @llvm.smax.i32(i32,i32)
declare i32 @llvm.smin.i32(i32,i32)
define i32 @main()
{
L0:  ;
    %r11 = alloca i32
    %r0 = alloca i32
    br label %L1
L1:  ;
    %r1 = add i32 5,0
    store i32 %r1, ptr %r0
    %r2 = load i32, ptr %r0
    %r3 = add i32 5,0
    %r4 = icmp sgt i32 %r2,%r3
    br i1 %r4, label %L2, label %L3
L2:  ;
    %r5 = add i32 5,0
    ret i32 %r5
    br label %L3
L3:  ;
    %r6 = load i32, ptr %r0
    %r7 = add i32 6,0
    %r8 = icmp sgt i32 %r6,%r7
    br i1 %r8, label %L4, label %L5
L4:  ;
    %r9 = add i32 7,0
    ret i32 %r9
    br label %L5
L5:  ;
    %r10 = add i32 0,0
    ret i32 %r10
    %r12 = add i32 10,0
    store i32 %r12, ptr %r11
    ret i32 0
}
