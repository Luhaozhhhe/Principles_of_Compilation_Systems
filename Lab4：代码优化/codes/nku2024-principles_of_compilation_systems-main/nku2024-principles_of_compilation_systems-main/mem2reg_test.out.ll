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
@w = global i32 512
@h = global i32 270
@imgIn = global [512x [270x float]] zeroinitializer
@imgOut = global [512x [270x float]] zeroinitializer
@my_y1 = global [512x [270x float]] zeroinitializer
@my_y2 = global [512x [270x float]] zeroinitializer
@alpha = global float 0x3fd0000000000000
define float @newExp(float %r0)
{
L0:  ;
    %r2 = fadd float 0x3ff0000000000000,0x0
    %r4 = add i32 256,0
    %r5 = sitofp i32 %r4 to float
    %r6 = fdiv float %r0,%r5
    %r7 = fadd float %r2,%r6
    %r10 = fmul float %r7,%r7
    %r13 = fmul float %r10,%r10
    %r16 = fmul float %r13,%r13
    %r19 = fmul float %r16,%r16
    %r22 = fmul float %r19,%r19
    %r25 = fmul float %r22,%r22
    %r28 = fmul float %r25,%r25
    %r31 = fmul float %r28,%r28
    ret float %r31
}
define float @newPow(float %r0,i32 %r1)
{
L0:  ;
    %r5 = add i32 0,0
    %r6 = icmp slt i32 %r1,%r5
    br i1 %r6, label %L1, label %L2
L1:  ;
    %r7 = fadd float 0x3ff0000000000000,0x0
    %r10 = sub i32 0,%r1
    %r11 = call float @newPow(float %r0,i32 %r10)
    %r12 = fdiv float %r7,%r11
    ret float %r12
L2:  ;
    %r14 = add i32 0,0
    %r15 = icmp eq i32 %r1,%r14
    br i1 %r15, label %L4, label %L5
L3:  ;
    ret float 0x0
L4:  ;
    %r16 = fadd float 0x3ff0000000000000,0x0
    ret float %r16
L5:  ;
    %r18 = add i32 0,0
    %r19 = icmp sgt i32 %r1,%r18
    br i1 %r19, label %L7, label %L8
L6:  ;
    br label %L3
L7:  ;
    %r23 = add i32 1,0
    %r24 = sub i32 %r1,%r23
    %r25 = call float @newPow(float %r0,i32 %r24)
    %r26 = fmul float %r0,%r25
    ret float %r26
L8:  ;
    br label %L6
}
define void @kernel_deriche(i32 %r0,i32 %r1,float %r2,ptr %r3,ptr %r4,ptr %r5,ptr %r6)
{
L0:  ;
    %r60 = fadd float 0x3ff0000000000000,0x0
    %r62 = fsub float 0x0,%r2
    %r63 = call float @newExp(float %r62)
    %r64 = fsub float %r60,%r63
    %r65 = fadd float 0x3ff0000000000000,0x0
    %r67 = fsub float 0x0,%r2
    %r68 = call float @newExp(float %r67)
    %r69 = fsub float %r65,%r68
    %r70 = fmul float %r64,%r69
    %r71 = fadd float 0x3ff0000000000000,0x0
    %r72 = fadd float 0x4000000000000000,0x0
    %r74 = fmul float %r72,%r2
    %r76 = fsub float 0x0,%r2
    %r77 = call float @newExp(float %r76)
    %r78 = fmul float %r74,%r77
    %r79 = fadd float %r71,%r78
    %r80 = fadd float 0x4000000000000000,0x0
    %r82 = fmul float %r80,%r2
    %r83 = call float @newExp(float %r82)
    %r84 = fsub float %r79,%r83
    %r85 = fdiv float %r70,%r84
    %r90 = fsub float 0x0,%r2
    %r91 = call float @newExp(float %r90)
    %r92 = fmul float %r85,%r91
    %r94 = fadd float 0x3ff0000000000000,0x0
    %r95 = fsub float %r2,%r94
    %r96 = fmul float %r92,%r95
    %r100 = fsub float 0x0,%r2
    %r101 = call float @newExp(float %r100)
    %r102 = fmul float %r85,%r101
    %r104 = fadd float 0x3ff0000000000000,0x0
    %r105 = fadd float %r2,%r104
    %r106 = fmul float %r102,%r105
    %r109 = fsub float 0x0,%r85
    %r110 = fadd float 0x4000000000000000,0x0
    %r111 = fsub float 0x0,%r110
    %r113 = fmul float %r111,%r2
    %r114 = call float @newExp(float %r113)
    %r115 = fmul float %r109,%r114
    %r117 = fadd float 0x4000000000000000,0x0
    %r119 = fsub float 0x0,%r2
    %r120 = fptosi float %r119 to i32
    %r121 = call float @newPow(float %r117,i32 %r120)
    %r122 = fadd float 0x4000000000000000,0x0
    %r123 = fsub float 0x0,%r122
    %r125 = fmul float %r123,%r2
    %r126 = call float @newExp(float %r125)
    %r127 = fsub float 0x0,%r126
    %r128 = add i32 1,0
    %r129 = sitofp i32 %r128 to float
    %r130 = add i32 1,0
    %r131 = sitofp i32 %r130 to float
    %r132 = add i32 0,0
    br label %L1
L1:  ;
    %r416 = phi i32 [%r132,%L0],[%r178,%L6]
    %r135 = icmp slt i32 %r416,%r0
    br i1 %r135, label %L2, label %L3
L2:  ;
    %r136 = fadd float 0x0,0x0
    %r137 = fadd float 0x0,0x0
    %r138 = fadd float 0x0,0x0
    %r139 = add i32 0,0
    br label %L4
L3:  ;
    %r179 = add i32 0,0
    br label %L7
L4:  ;
    %r407 = phi i32 [%r139,%L2],[%r175,%L5]
    %r405 = phi float [%r138,%L2],[%r167,%L5]
    %r399 = phi float [%r136,%L2],[%r172,%L5]
    %r395 = phi float [%r137,%L2],[%r399,%L5]
    %r142 = icmp slt i32 %r407,%r1
    br i1 %r142, label %L5, label %L6
L5:  ;
    %r145 = getelementptr [270 x float], ptr %r5, i32 %r416, i32 %r407
    %r149 = getelementptr [270 x float], ptr %r3, i32 %r416, i32 %r407
    %r150 = load float, ptr %r149
    %r151 = fmul float %r85,%r150
    %r154 = fmul float %r96,%r405
    %r155 = fadd float %r151,%r154
    %r158 = fmul float %r121,%r399
    %r159 = fadd float %r155,%r158
    %r162 = fmul float %r127,%r395
    %r163 = fadd float %r159,%r162
    store float %r163, ptr %r145
    %r166 = getelementptr [270 x float], ptr %r3, i32 %r416, i32 %r407
    %r167 = load float, ptr %r166
    %r171 = getelementptr [270 x float], ptr %r5, i32 %r416, i32 %r407
    %r172 = load float, ptr %r171
    %r174 = add i32 1,0
    %r175 = add i32 %r407,%r174
    br label %L4
L6:  ;
    %r177 = add i32 1,0
    %r178 = add i32 %r416,%r177
    br label %L1
L7:  ;
    %r417 = phi i32 [%r179,%L3],[%r226,%L12]
    %r182 = icmp slt i32 %r417,%r0
    br i1 %r182, label %L8, label %L9
L8:  ;
    %r183 = fadd float 0x0,0x0
    %r184 = fadd float 0x0,0x0
    %r185 = fadd float 0x0,0x0
    %r186 = fadd float 0x0,0x0
    %r188 = add i32 1,0
    %r189 = sub i32 %r1,%r188
    br label %L10
L9:  ;
    %r227 = add i32 0,0
    br label %L13
L10:  ;
    %r409 = phi i32 [%r189,%L8],[%r223,%L11]
    %r393 = phi float [%r185,%L8],[%r215,%L11]
    %r391 = phi float [%r186,%L8],[%r393,%L11]
    %r383 = phi float [%r183,%L8],[%r220,%L11]
    %r379 = phi float [%r184,%L8],[%r383,%L11]
    %r191 = add i32 0,0
    %r192 = icmp sge i32 %r409,%r191
    br i1 %r192, label %L11, label %L12
L11:  ;
    %r195 = getelementptr [270 x float], ptr %r6, i32 %r417, i32 %r409
    %r198 = fmul float %r106,%r393
    %r201 = fmul float %r115,%r391
    %r202 = fadd float %r198,%r201
    %r205 = fmul float %r121,%r383
    %r206 = fadd float %r202,%r205
    %r209 = fmul float %r127,%r379
    %r210 = fadd float %r206,%r209
    store float %r210, ptr %r195
    %r214 = getelementptr [270 x float], ptr %r3, i32 %r417, i32 %r409
    %r215 = load float, ptr %r214
    %r219 = getelementptr [270 x float], ptr %r6, i32 %r417, i32 %r409
    %r220 = load float, ptr %r219
    %r222 = add i32 1,0
    %r223 = sub i32 %r409,%r222
    br label %L10
L12:  ;
    %r225 = add i32 1,0
    %r226 = add i32 %r417,%r225
    br label %L7
L13:  ;
    %r418 = phi i32 [%r227,%L9],[%r254,%L18]
    %r230 = icmp slt i32 %r418,%r0
    br i1 %r230, label %L14, label %L15
L14:  ;
    %r231 = add i32 0,0
    br label %L16
L15:  ;
    %r255 = add i32 0,0
    br label %L19
L16:  ;
    %r411 = phi i32 [%r231,%L14],[%r251,%L17]
    %r234 = icmp slt i32 %r411,%r1
    br i1 %r234, label %L17, label %L18
L17:  ;
    %r237 = getelementptr [270 x float], ptr %r4, i32 %r418, i32 %r411
    %r241 = getelementptr [270 x float], ptr %r5, i32 %r418, i32 %r411
    %r242 = load float, ptr %r241
    %r245 = getelementptr [270 x float], ptr %r6, i32 %r418, i32 %r411
    %r246 = load float, ptr %r245
    %r247 = fadd float %r242,%r246
    %r248 = fmul float %r129,%r247
    store float %r248, ptr %r237
    %r250 = add i32 1,0
    %r251 = add i32 %r411,%r250
    br label %L16
L18:  ;
    %r253 = add i32 1,0
    %r254 = add i32 %r418,%r253
    br label %L13
L19:  ;
    %r412 = phi i32 [%r255,%L15],[%r301,%L24]
    %r258 = icmp slt i32 %r412,%r1
    br i1 %r258, label %L20, label %L21
L20:  ;
    %r259 = fadd float 0x0,0x0
    %r260 = fadd float 0x0,0x0
    %r261 = fadd float 0x0,0x0
    %r262 = add i32 0,0
    br label %L22
L21:  ;
    %r302 = add i32 0,0
    br label %L25
L22:  ;
    %r420 = phi i32 [%r262,%L20],[%r298,%L23]
    %r403 = phi float [%r259,%L20],[%r290,%L23]
    %r401 = phi float [%r260,%L20],[%r295,%L23]
    %r397 = phi float [%r261,%L20],[%r401,%L23]
    %r265 = icmp slt i32 %r420,%r0
    br i1 %r265, label %L23, label %L24
L23:  ;
    %r268 = getelementptr [270 x float], ptr %r5, i32 %r420, i32 %r412
    %r272 = getelementptr [270 x float], ptr %r4, i32 %r420, i32 %r412
    %r273 = load float, ptr %r272
    %r274 = fmul float %r85,%r273
    %r277 = fmul float %r96,%r403
    %r278 = fadd float %r274,%r277
    %r281 = fmul float %r121,%r401
    %r282 = fadd float %r278,%r281
    %r285 = fmul float %r127,%r397
    %r286 = fadd float %r282,%r285
    store float %r286, ptr %r268
    %r289 = getelementptr [270 x float], ptr %r4, i32 %r420, i32 %r412
    %r290 = load float, ptr %r289
    %r294 = getelementptr [270 x float], ptr %r5, i32 %r420, i32 %r412
    %r295 = load float, ptr %r294
    %r297 = add i32 1,0
    %r298 = add i32 %r420,%r297
    br label %L22
L24:  ;
    %r300 = add i32 1,0
    %r301 = add i32 %r412,%r300
    br label %L19
L25:  ;
    %r413 = phi i32 [%r302,%L21],[%r349,%L30]
    %r305 = icmp slt i32 %r413,%r1
    br i1 %r305, label %L26, label %L27
L26:  ;
    %r306 = fadd float 0x0,0x0
    %r307 = fadd float 0x0,0x0
    %r308 = fadd float 0x0,0x0
    %r309 = fadd float 0x0,0x0
    %r311 = add i32 1,0
    %r312 = sub i32 %r0,%r311
    br label %L28
L27:  ;
    %r350 = add i32 0,0
    br label %L31
L28:  ;
    %r422 = phi i32 [%r312,%L26],[%r346,%L29]
    %r389 = phi float [%r306,%L26],[%r338,%L29]
    %r387 = phi float [%r307,%L26],[%r389,%L29]
    %r385 = phi float [%r308,%L26],[%r343,%L29]
    %r381 = phi float [%r309,%L26],[%r385,%L29]
    %r314 = add i32 0,0
    %r315 = icmp sge i32 %r422,%r314
    br i1 %r315, label %L29, label %L30
L29:  ;
    %r318 = getelementptr [270 x float], ptr %r6, i32 %r422, i32 %r413
    %r321 = fmul float %r106,%r389
    %r324 = fmul float %r115,%r387
    %r325 = fadd float %r321,%r324
    %r328 = fmul float %r121,%r385
    %r329 = fadd float %r325,%r328
    %r332 = fmul float %r127,%r381
    %r333 = fadd float %r329,%r332
    store float %r333, ptr %r318
    %r337 = getelementptr [270 x float], ptr %r4, i32 %r422, i32 %r413
    %r338 = load float, ptr %r337
    %r342 = getelementptr [270 x float], ptr %r6, i32 %r422, i32 %r413
    %r343 = load float, ptr %r342
    %r345 = add i32 1,0
    %r346 = sub i32 %r422,%r345
    br label %L28
L30:  ;
    %r348 = add i32 1,0
    %r349 = add i32 %r413,%r348
    br label %L25
L31:  ;
    %r423 = phi i32 [%r350,%L27],[%r377,%L36]
    %r353 = icmp slt i32 %r423,%r0
    br i1 %r353, label %L32, label %L33
L32:  ;
    %r354 = add i32 0,0
    br label %L34
L33:  ;
    ret void
L34:  ;
    %r415 = phi i32 [%r354,%L32],[%r374,%L35]
    %r357 = icmp slt i32 %r415,%r1
    br i1 %r357, label %L35, label %L36
L35:  ;
    %r360 = getelementptr [270 x float], ptr %r4, i32 %r423, i32 %r415
    %r364 = getelementptr [270 x float], ptr %r5, i32 %r423, i32 %r415
    %r365 = load float, ptr %r364
    %r368 = getelementptr [270 x float], ptr %r6, i32 %r423, i32 %r415
    %r369 = load float, ptr %r368
    %r370 = fadd float %r365,%r369
    %r371 = fmul float %r131,%r370
    store float %r371, ptr %r360
    %r373 = add i32 1,0
    %r374 = add i32 %r415,%r373
    br label %L34
L36:  ;
    %r376 = add i32 1,0
    %r377 = add i32 %r423,%r376
    br label %L31
}
define i32 @main()
{
L0:  ;
    %r0 = getelementptr [512 x [270 x float]], ptr @imgIn, i32 0
    %r1 = call i32 @getfarray(ptr %r0)
    %r2 = load i32, ptr @w
    %r3 = load i32, ptr @h
    %r4 = load float, ptr @alpha
    %r5 = getelementptr [512 x [270 x float]], ptr @imgIn, i32 0
    %r6 = getelementptr [512 x [270 x float]], ptr @imgOut, i32 0
    %r7 = getelementptr [512 x [270 x float]], ptr @my_y1, i32 0
    %r8 = getelementptr [512 x [270 x float]], ptr @my_y2, i32 0
    call void @kernel_deriche(i32 %r2,i32 %r3,float %r4,ptr %r5,ptr %r6,ptr %r7,ptr %r8)
    %r9 = load i32, ptr @w
    %r10 = load i32, ptr @h
    %r11 = mul i32 %r9,%r10
    %r12 = getelementptr [512 x [270 x float]], ptr @imgOut, i32 0
    call void @putfarray(i32 %r11,ptr %r12)
    %r13 = add i32 0,0
    ret i32 %r13
}
