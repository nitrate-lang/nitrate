; ModuleID = 'examples/basics/while.q'
source_filename = "examples/basics/while.q"

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  %x = alloca i8, align 1
  store i8 20, i8* %x, align 1
  br label %1

end:                                              ; preds = %8, %6
  %0 = load i8, i8* %ret, align 1
  ret i8 %0

1:                                                ; preds = %7, %entry
  %2 = icmp eq i8 1, 1
  br i1 %2, label %3, label %8

3:                                                ; preds = %1
  %4 = load i8, i8* %x, align 1
  %5 = icmp eq i8 %4, 21
  br i1 %5, label %6, label %7

6:                                                ; preds = %3
  store i8 20, i8* %ret, align 1
  br label %end

7:                                                ; preds = %3
  br label %1

8:                                                ; preds = %1
  store i8 60, i8* %ret, align 1
  br label %end
}
