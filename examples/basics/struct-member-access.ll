; ModuleID = 'examples/basics/struct-member-access.q'
source_filename = "examples/basics/struct-member-access.q"

%PersonInfo.0 = type { i8*, i8 }

@0 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@1 = private unnamed_addr constant [9 x i8] c"Age: %d\0A\00", align 1

declare i32 @printf(i8*, ...)

declare i8* @gets(i8*)

declare i32 @scanf(i8*, ...)

declare i8* @malloc(i64)

define private %PersonInfo.0 @_ZJ0f17get__person__info22f17t10PersonInfo1x1b1p1p() {
entry:
  %ret = alloca %PersonInfo.0, align 8
  %info = alloca %PersonInfo.0, align 8
  %info1 = load %PersonInfo.0, %PersonInfo.0* %info, align 8
  %0 = alloca %PersonInfo.0, align 8
  store %PersonInfo.0 %info1, %PersonInfo.0* %0, align 8
  %1 = getelementptr inbounds %PersonInfo.0, %PersonInfo.0* %0, i32 0, i32 1
  %2 = load i8, i8* %1, align 1
  %3 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 %2)
  %info2 = load %PersonInfo.0, %PersonInfo.0* %info, align 8
  store %PersonInfo.0 %info2, %PersonInfo.0* %ret, align 8
  br label %end

end:                                              ; preds = %entry
  %4 = load %PersonInfo.0, %PersonInfo.0* %ret, align 8
  ret %PersonInfo.0 %4
}

define void @main() {
entry:
  %res = alloca %PersonInfo.0, align 8
  %0 = call %PersonInfo.0 @_ZJ0f17get__person__info22f17t10PersonInfo1x1b1p1p()
  store %PersonInfo.0 %0, %PersonInfo.0* %res, align 8
  %res1 = load %PersonInfo.0, %PersonInfo.0* %res, align 8
  %1 = alloca %PersonInfo.0, align 8
  store %PersonInfo.0 %res1, %PersonInfo.0* %1, align 8
  %2 = getelementptr inbounds %PersonInfo.0, %PersonInfo.0* %1, i32 0, i32 1
  %3 = load i8, i8* %2, align 1
  %4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @1, i32 0, i32 0), i8 %3)
  br label %end

end:                                              ; preds = %entry
  ret void
}
