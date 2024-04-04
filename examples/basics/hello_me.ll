; ModuleID = 'examples/basics/hello_me.q'
source_filename = "examples/basics/hello_me.q"

@_fa37JncCHr = private global [11 x i8] c"Hello, %s\0A\00"
@_yDsbzayy4c = private global [7 x i8] c"Wesley\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %ret = alloca i32, align 4
  %calltmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @_fa37JncCHr, i32 0, i32 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @_yDsbzayy4c, i32 0, i32 0))
  store i32 %calltmp, i32* %ret, align 4
  br label %end

end:                                              ; preds = %entry
  %0 = load i32, i32* %ret, align 4
  ret i32 %0
}
