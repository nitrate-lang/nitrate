; ModuleID = 'examples/basics/fn-call.q'
source_filename = "examples/basics/fn-call.q"

define private i8 @_ZJ0f11CallMeFirst7f1c1c1p1p(i8 %a) {
entry:
  %ret = alloca i8, align 1
  %0 = add i8 %a, 10
  store i8 %0, i8* %ret, align 1
  br label %end

end:                                              ; preds = %entry
  %1 = load i8, i8* %ret, align 1
  ret i8 %1
}

define private i8 @_ZJ0f12CallMeSecond7f1c1c1p1p(i8 %b) {
entry:
  %ret = alloca i8, align 1
  %0 = mul i8 %b, 20
  store i8 %0, i8* %ret, align 1
  br label %end

end:                                              ; preds = %entry
  %1 = load i8, i8* %ret, align 1
  ret i8 %1
}

define i8 @main(i32 %argc) {
entry:
  %ret = alloca i8, align 1
  %calltmp = call i8 @_ZJ0f11CallMeFirst7f1c1c1p1p(i8 10)
  %calltmp1 = call i8 @_ZJ0f12CallMeSecond7f1c1c1p1p(i8 %calltmp)
  %0 = add i8 60, %calltmp1
  store i8 %0, i8* %ret, align 1
  br label %end

end:                                              ; preds = %entry
  %1 = load i8, i8* %ret, align 1
  ret i8 %1
}
