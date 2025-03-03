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
    %r2 = alloca i32
    %r0 = alloca i32
    %r1 = add i32 0,0
    store i32 %r1, ptr %r0
    %r3 = add i32 100000,0
    store i32 %r3, ptr %r2
    br label %L1
L1:  ;
    %r4 = load i32, ptr %r0
    %r5 = load i32, ptr %r2
    %r6 = icmp slt i32 %r4,%r5
    br i1 %r6, label %L2, label %L3
L2:  ;
    %r7 = load i32, ptr %r0
    %r8 = add i32 9961,0
    %r9 = icmp sgt i32 %r7,%r8
    br i1 %r9, label %L4, label %L5
L3:  ;
    %r22 = load i32, ptr %r0
    %r23 = add i32 5000,0
    %r24 = icmp sgt i32 %r22,%r23
    br i1 %r24, label %L9, label %L10
L4:  ;
    br label %L6
L5:  ;
    %r19 = load i32, ptr %r0
    %r20 = add i32 1,0
    %r21 = add i32 %r19,%r20
    store i32 %r21, ptr %r0
    br label %L1
L6:  ;
    %r10 = load i32, ptr %r0
    %r11 = load i32, ptr %r2
    %r12 = icmp slt i32 %r10,%r11
    br i1 %r12, label %L7, label %L8
L7:  ;
    %r13 = load i32, ptr %r0
    call void @putint(i32 %r13)
    %r14 = add i32 10,0
    call void @putch(i32 %r14)
    %r15 = add i32 0,0
    ret i32 %r15
L8:  ;
    br label %L5
L9:  ;
    %r25 = add i32 0,0
    ret i32 %r25
L10:  ;
    %r918 = add i32 0,0
    ret i32 %r918
}
