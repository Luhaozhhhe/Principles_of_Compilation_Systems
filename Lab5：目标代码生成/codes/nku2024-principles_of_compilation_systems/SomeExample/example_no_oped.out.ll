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
    %r4 = alloca i32    //c
    %r2 = alloca i32    //b
    %r0 = alloca i32    //a

    %r1 = add i32 5,0
    store i32 %r1, ptr %r0
    //a=5;

    %r3 = add i32 6,0
    store i32 %r3, ptr %r2
    //b=6;

    %r5 = add i32 7,0
    store i32 %r5, ptr %r4
    //c=7;

    %r6 = load i32, ptr %r0
    %r7 = add i32 5,0
    %r8 = add i32 %r6,%r7
    store i32 %r8, ptr %r0
    //a=a+5;
    


    %r9 = load i32, ptr %r0
    %r10 = load i32, ptr %r4
    %r11 = add i32 %r9,%r10
    //b=a+c

    store i32 %r11, ptr %r2
    %r12 = load i32, ptr %r0
    %r13 = load i32, ptr %r4
    %r14 = add i32 %r12,%r13
    ret i32 %r14
    
    //re=a+c;
}
