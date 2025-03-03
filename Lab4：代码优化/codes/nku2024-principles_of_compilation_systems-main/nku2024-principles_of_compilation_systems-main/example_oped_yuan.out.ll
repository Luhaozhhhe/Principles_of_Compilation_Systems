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
    %r1 = call i32 @getint()
    %r3 = add i32 0,0
    %r5 = add i32 0,0
    %r7 = add i32 0,0
    %r10 = add i32 %r5,%r7
    %r13 = sub i32 %r5,%r7
    br label %L1
L1:  ;
    %r38 = phi i32 [%r3,%L0],[%r22,%L2]
    %r37 = phi i32 [%r5,%L0],[%r19,%L2]
    %r16 = icmp slt i32 %r38,%r1
    br i1 %r16, label %L2, label %L3
L2:  ;
    %r19 = add i32 %r37,%r38
    %r21 = add i32 1,0
    %r22 = add i32 %r38,%r21
    br label %L1
L3:  ;
    %r23 = add i32 0,0
    br label %L4
L4:  ;
    %r39 = phi i32 [%r23,%L3],[%r32,%L5]
    %r36 = phi i32 [%r7,%L3],[%r29,%L5]
    %r25 = add i32 10,0
    %r26 = icmp slt i32 %r39,%r25
    br i1 %r26, label %L5, label %L6
L5:  ;
    %r29 = add i32 %r36,%r39
    %r31 = add i32 1,0
    %r32 = add i32 %r39,%r31
    br label %L4
L6:  ;
    call void @putint(i32 %r36)
    %r34 = add i32 10,0
    call void @putch(i32 %r34)
    %r35 = add i32 0,0
    ret i32 %r35
}
