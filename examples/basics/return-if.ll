; ModuleID = 'examples/basics/return-if.q'
source_filename = "examples/basics/return-if.q"

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  %0 = icmp eq i1 true, true
  %1 = xor i1 %0, true
  br i1 %1, label %3, label %4

end:                                              ; preds = %9, %8, %5, %3
  %2 = load i8, i8* %ret, align 1
  ret i8 %2

3:                                                ; preds = %entry
  br label %end

4:                                                ; preds = %entry
  br i1 true, label %5, label %6

5:                                                ; preds = %4
  store i8 10, i8* %ret, align 1
  br label %end

6:                                                ; preds = %4
  %7 = xor i1 false, true
  br i1 %7, label %8, label %9

8:                                                ; preds = %6
  store i8 20, i8* %ret, align 1
  br label %end

9:                                                ; preds = %6
  store i8 60, i8* %ret, align 1
  br label %end
}
