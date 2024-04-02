; ModuleID = 'examples/basics/for.q'
source_filename = "examples/basics/for.q"

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  %x = alloca i8, align 1
  store i8 20, i8* %x, align 1
  br label %1

end:                                              ; preds = %4
  %0 = load i8, i8* %ret, align 1
  ret i8 %0

1:                                                ; preds = %3, %entry
  %x1 = load i8, i8* %x, align 1
  %2 = icmp slt i8 %x1, 100
  br i1 %2, label %3, label %4

3:                                                ; preds = %1
  br label %1

4:                                                ; preds = %1
  store i8 60, i8* %ret, align 1
  br label %end
}
