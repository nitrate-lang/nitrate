; ModuleID = 'examples/basics/while.q'
source_filename = "examples/basics/while.q"

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  %x = alloca i8, align 1
  store i8 20, i8* %x, align 1
  br label %1

end:                                              ; preds = %7, %5
  %0 = load i8, i8* %ret, align 1
  ret i8 %0

1:                                                ; preds = %6, %entry
  %2 = icmp eq i8 1, 1
  br i1 %2, label %3, label %7

3:                                                ; preds = %1
  %x1 = load i8, i8* %x, align 1
  %4 = icmp eq i8 %x1, 21
  br i1 %4, label %5, label %6

5:                                                ; preds = %3
  store i8 20, i8* %ret, align 1
  br label %end

6:                                                ; preds = %3
  br label %1

7:                                                ; preds = %1
  store i8 60, i8* %ret, align 1
  br label %end
}
