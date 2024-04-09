; ModuleID = 'examples/basics/hello_world.q'
source_filename = "examples/basics/hello_world.q"

@0 = private unnamed_addr constant [66 x i8] c"Hi. Behold, the first Hello World program for the QUIX language!\0A\00", align 1
@1 = private unnamed_addr constant [45 x i8] c"This is the first of many programs to come.\0A\00", align 1
@2 = private unnamed_addr constant [60 x i8] c"Long live QUIX! May it reign supreme for %d years to come!\0A\00", align 1

declare i32 @printf(i8*, ...)

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([66 x i8], [66 x i8]* @0, i32 0, i32 0))
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @1, i32 0, i32 0))
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([60 x i8], [60 x i8]* @2, i32 0, i32 0), i16 1000)
  store i8 0, i8* %ret, align 1
  br label %end

end:                                              ; preds = %entry
  %3 = load i8, i8* %ret, align 1
  ret i8 %3
}
