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
  %0 = load i32, i32* %x, align 4
  store i32* %x, i32** %x_ptr, align 8
  %1 = load i32, i32* %x, align 4
  %2 = load i32, i32* %x, align 4
  %3 = load i32*, i32** %x_ptr, align 8
  %4 = load i32*, i32** %x_ptr, align 8
  %5 = load i32*, i32** %x_ptr, align 8
  %6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @0, i32 0, i32 0), i32 %1, i32* %x, i32* %3, i32* %5)
  store i32 20000000, i32* %x, align 4
  %7 = load i32, i32* %x, align 4
  %8 = load i32, i32* %x, align 4
  %9 = load i32*, i32** %x_ptr, align 8
  %10 = load i32*, i32** %x_ptr, align 8
  %11 = load i32*, i32** %x_ptr, align 8
  %12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @1, i32 0, i32 0), i32 %7, i32* %x, i32* %9, i32* %11)
  %13 = load i32*, i32** %x_ptr, align 8
  store i32 30000000, i32* %13, align 4
  %14 = load i32, i32* %x, align 4
  %15 = load i32, i32* %x, align 4
  %16 = load i32*, i32** %x_ptr, align 8
  %17 = load i32*, i32** %x_ptr, align 8
  %18 = load i32*, i32** %x_ptr, align 8
  %19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @2, i32 0, i32 0), i32 %14, i32* %x, i32* %16, i32* %18)
  br label %end

end:                                              ; preds = %entry
  ret void
}
