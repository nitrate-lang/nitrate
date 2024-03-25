; ModuleID = 'examples/basics/return-stmt.q'
source_filename = "examples/basics/return-stmt.q"

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  %0 = add i8 1, 2
  %1 = mul i8 %0, 3
  %2 = sub i8 %1, 9
  store i8 %2, i8* %ret, align 1
  br label %end

end:                                              ; preds = %entry
  %3 = load i8, i8* %ret, align 1
  ret i8 %3
}
