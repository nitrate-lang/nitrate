; ModuleID = 'examples/basics/address-of.q'
source_filename = "examples/basics/address-of.q"

@0 = private unnamed_addr constant [18 x i8] c"address of x: %p\0A\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %x = alloca i8, align 1
  store i8 10, i8* %x, align 1
  %x1 = load i8, i8* %x, align 1
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @0, i32 0, i32 0), i8* %x)
  br label %end

end:                                              ; preds = %entry
  ret void
}
