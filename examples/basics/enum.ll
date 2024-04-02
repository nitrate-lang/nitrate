; ModuleID = 'examples/basics/enum.q'
source_filename = "examples/basics/enum.q"

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  store i8 11, i8* %ret, align 1
  br label %end

end:                                              ; preds = %entry
  %0 = load i8, i8* %ret, align 1
  ret i8 %0
}
