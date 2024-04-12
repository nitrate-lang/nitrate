; ModuleID = 'examples/basics/struct-member-access.q'
source_filename = "examples/basics/struct-member-access.q"

%PersonInfo.0 = type { i8*, i32 }

@0 = private unnamed_addr constant [18 x i8] c"Enter your name: \00", align 1
@1 = private unnamed_addr constant [17 x i8] c"Enter your age: \00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@3 = private unnamed_addr constant [10 x i8] c"Name: %s\0A\00", align 1
@4 = private unnamed_addr constant [9 x i8] c"Age: %d\0A\00", align 1

declare i32 @printf(i8*, ...)

declare i32 @scanf(i8*, ...)

declare i8* @gets(i8*)

declare i8* @malloc(i64)

declare void @free(i8*)

define private %PersonInfo.0 @_ZJ0f17get__person__info22f17t10PersonInfo1x1i1p1p() {
entry:
  %ret = alloca %PersonInfo.0, align 8
  %info = alloca %PersonInfo.0, align 8
  %info1 = load %PersonInfo.0, %PersonInfo.0* %info, align 8
  %0 = getelementptr inbounds %PersonInfo.0, %PersonInfo.0* %info, i32 0, i32 0
  %1 = xor i64 -101, -1
  %2 = call i8* @malloc(i64 %1)
  store i8* %2, i8** %0, align 8
  %3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @0, i32 0, i32 0))
  %info2 = load %PersonInfo.0, %PersonInfo.0* %info, align 8
  %4 = alloca %PersonInfo.0, align 8
  store %PersonInfo.0 %info2, %PersonInfo.0* %4, align 8
  %5 = getelementptr inbounds %PersonInfo.0, %PersonInfo.0* %4, i32 0, i32 0
  %6 = load i8*, i8** %5, align 8
  %7 = call i8* @gets(i8* %6)
  %8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @1, i32 0, i32 0))
  %info3 = load %PersonInfo.0, %PersonInfo.0* %info, align 8
  %9 = alloca %PersonInfo.0, align 8
  store %PersonInfo.0 %info3, %PersonInfo.0* %9, align 8
  %10 = getelementptr inbounds %PersonInfo.0, %PersonInfo.0* %9, i32 0, i32 1
  %11 = load i32, i32* %10, align 4
  %info4 = load %PersonInfo.0, %PersonInfo.0* %info, align 8
  %12 = getelementptr inbounds %PersonInfo.0, %PersonInfo.0* %info, i32 0, i32 1
  %13 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i32* %12)
  %info5 = load %PersonInfo.0, %PersonInfo.0* %info, align 8
  store %PersonInfo.0 %info5, %PersonInfo.0* %ret, align 8
  br label %end

end:                                              ; preds = %entry
  %14 = load %PersonInfo.0, %PersonInfo.0* %ret, align 8
  ret %PersonInfo.0 %14
}

define void @main() {
entry:
  %res = alloca %PersonInfo.0, align 8
  %0 = call %PersonInfo.0 @_ZJ0f17get__person__info22f17t10PersonInfo1x1i1p1p()
  store %PersonInfo.0 %0, %PersonInfo.0* %res, align 8
  %res1 = load %PersonInfo.0, %PersonInfo.0* %res, align 8
  %1 = alloca %PersonInfo.0, align 8
  store %PersonInfo.0 %res1, %PersonInfo.0* %1, align 8
  %2 = getelementptr inbounds %PersonInfo.0, %PersonInfo.0* %1, i32 0, i32 0
  %3 = load i8*, i8** %2, align 8
  %4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @3, i32 0, i32 0), i8* %3)
  %res2 = load %PersonInfo.0, %PersonInfo.0* %res, align 8
  %5 = alloca %PersonInfo.0, align 8
  store %PersonInfo.0 %res2, %PersonInfo.0* %5, align 8
  %6 = getelementptr inbounds %PersonInfo.0, %PersonInfo.0* %5, i32 0, i32 1
  %7 = load i32, i32* %6, align 4
  %8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @4, i32 0, i32 0), i32 %7)
  %res3 = load %PersonInfo.0, %PersonInfo.0* %res, align 8
  %9 = alloca %PersonInfo.0, align 8
  store %PersonInfo.0 %res3, %PersonInfo.0* %9, align 8
  %10 = getelementptr inbounds %PersonInfo.0, %PersonInfo.0* %9, i32 0, i32 0
  %11 = load i8*, i8** %10, align 8
  call void @free(i8* %11)
  br label %end

end:                                              ; preds = %entry
  ret void
}
