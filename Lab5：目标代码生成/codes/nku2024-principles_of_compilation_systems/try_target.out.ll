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
define i32 @add(i32 %r0,i32 %r1)
{
L0:  ;
    %r3 = alloca i32
    %r2 = alloca i32
    store i32 %r0, ptr %r2
    store i32 %r1, ptr %r3
    %r4 = load i32, ptr %r2
    %r5 = load i32, ptr %r3
    %r6 = add i32 %r4,%r5
    ret i32 %r6
}
define i32 @main()
{
L0:  ;
    %r0 = add i32 5,0
    %r1 = add i32 6,0
    %r2 = call i32 @add(i32 %r0,i32 %r1)
    ret i32 %r2
}
