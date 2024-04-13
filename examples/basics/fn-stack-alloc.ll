; ModuleID = 'examples/basics/fn-stack-alloc.q'
source_filename = "examples/basics/fn-stack-alloc.q"

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  %x = alloca i8, align 1
  store i8 10, i8* %x, align 1
  %y = alloca i8, align 1
  store i8 20, i8* %y, align 1
  %0 = load i8, i8* %x, align 1
  %1 = load i8, i8* %y, align 1
  %2 = add i8 %0, %1
  store i8 %2, i8* %ret, align 1
  br label %end

end:                                              ; preds = %entry
  %3 = load i8, i8* %ret, align 1
  ret i8 %3
}
