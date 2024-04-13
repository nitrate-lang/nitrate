; ModuleID = 'examples/basics/for.q'
source_filename = "examples/basics/for.q"

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  %x = alloca i8, align 1
  store i8 20, i8* %x, align 1
  br label %1

end:                                              ; preds = %5
  %0 = load i8, i8* %ret, align 1
  ret i8 %0

1:                                                ; preds = %4, %entry
  %2 = load i8, i8* %x, align 1
  %3 = icmp slt i8 %2, 100
  br i1 %3, label %4, label %5

4:                                                ; preds = %1
  br label %1

5:                                                ; preds = %1
  store i8 60, i8* %ret, align 1
  br label %end
}
