; ModuleID = 'examples/basics/dereference.q'
source_filename = "examples/basics/dereference.q"

@0 = private unnamed_addr constant [42 x i8] c"x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\0A\00", align 1
@1 = private unnamed_addr constant [42 x i8] c"x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\0A\00", align 1
@2 = private unnamed_addr constant [42 x i8] c"x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\0A\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %x = alloca i32, align 4
  store i32 10000000, i32* %x, align 4
  %x_ptr = alloca i32*, align 8
  %x1 = load i32, i32* %x, align 4
  store i32* %x, i32** %x_ptr, align 8
  %x2 = load i32, i32* %x, align 4
  %x3 = load i32, i32* %x, align 4
  %x_ptr4 = load i32*, i32** %x_ptr, align 8
  %x_ptr5 = load i32*, i32** %x_ptr, align 8
  %0 = bitcast i32* %x_ptr5 to i32**
  %1 = load i32*, i32** %0, align 8
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @0, i32 0, i32 0), i32 %x2, i32* %x, i32* %x_ptr4, i32* %1)
  store i32 20000000, i32* %x, align 4
  %x6 = load i32, i32* %x, align 4
  %x7 = load i32, i32* %x, align 4
  %x_ptr8 = load i32*, i32** %x_ptr, align 8
  %x_ptr9 = load i32*, i32** %x_ptr, align 8
  %3 = bitcast i32* %x_ptr9 to i32**
  %4 = load i32*, i32** %3, align 8
  %5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @1, i32 0, i32 0), i32 %x6, i32* %x, i32* %x_ptr8, i32* %4)
  %x_ptr10 = load i32*, i32** %x_ptr, align 8
  store i32 30000000, i32* %x_ptr10, align 4
  %x11 = load i32, i32* %x, align 4
  %x12 = load i32, i32* %x, align 4
  %x_ptr13 = load i32*, i32** %x_ptr, align 8
  %x_ptr14 = load i32*, i32** %x_ptr, align 8
  %6 = bitcast i32* %x_ptr14 to i32**
  %7 = load i32*, i32** %6, align 8
  %8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @2, i32 0, i32 0), i32 %x11, i32* %x, i32* %x_ptr13, i32* %7)
  br label %end

end:                                              ; preds = %entry
  ret void
}
