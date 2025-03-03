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
    %r7 = add i32 0,0
    br label %L1
L1:  ;
    br label %L3
L2:  ;
    br label %L1
L3:  ;
    %r17 = add i32 0,0
    br label %L4
L4:  ;
    %r33 = phi i32 [%r17,%L3],[%r26,%L5]
    %r30 = phi i32 [%r7,%L3],[%r23,%L5]
    %r19 = add i32 10,0
    %r20 = icmp slt i32 %r33,%r19
    br i1 %r20, label %L5, label %L6
L5:  ;
    %r23 = add i32 %r30,%r33
    %r25 = add i32 1,0
    %r26 = add i32 %r33,%r25
    br label %L4
L6:  ;
    call void @putint(i32 %r30)
    %r28 = add i32 10,0
    call void @putch(i32 %r28)
    %r29 = add i32 0,0
    ret i32 %r29
}
