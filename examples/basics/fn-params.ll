; ModuleID = 'examples/basics/fn-params.q'
source_filename = "examples/basics/fn-params.q"

define private i8 @_ZJ0f11CallMeFirst9f1c1a1c1p1p(i8 %a) {
entry:
  %ret = alloca i8, align 1
  %0 = add i8 %a, 10
  store i8 %0, i8* %ret, align 1
  br label %end

end:                                              ; preds = %entry
  %1 = load i8, i8* %ret, align 1
  ret i8 %1
}

define private i8 @_ZJ0f12CallMeSecond9f1c1b1c1p1p(i8 %b) {
entry:
  %ret = alloca i8, align 1
  %0 = mul i8 %b, 20
  store i8 %0, i8* %ret, align 1
  br label %end

end:                                              ; preds = %entry
  %1 = load i8, i8* %ret, align 1
  ret i8 %1
}
