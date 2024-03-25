; ModuleID = 'examples/basics/if-statement.q'
source_filename = "examples/basics/if-statement.q"

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  %0 = icmp eq i8 2, 2
  br i1 %0, label %2, label %4

end:                                              ; preds = %12, %10, %9, %8, %7, %6
  %1 = load i8, i8* %ret, align 1
  ret i8 %1

2:                                                ; preds = %entry
  %3 = icmp eq i8 3, 3
  br i1 %3, label %6, label %7

4:                                                ; preds = %entry
  %5 = icmp eq i8 5, 4
  br i1 %5, label %9, label %10

6:                                                ; preds = %2
  store i8 10, i8* %ret, align 1
  br label %end

7:                                                ; preds = %2
  store i8 20, i8* %ret, align 1
  br label %end

8:                                                ; No predecessors!
  store i8 30, i8* %ret, align 1
  br label %end

9:                                                ; preds = %4
  store i8 40, i8* %ret, align 1
  br label %end

10:                                               ; preds = %4
  store i8 50, i8* %ret, align 1
  br label %end

11:                                               ; No predecessors!
  br label %12

12:                                               ; preds = %11
  store i8 60, i8* %ret, align 1
  br label %end
}
