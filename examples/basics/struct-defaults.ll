; ModuleID = 'examples/basics/struct-defaults.q'
source_filename = "examples/basics/struct-defaults.q"

%Person.1 = type { %Address.0, i8*, i8 }
%Address.0 = type { i8*, i8*, i8*, i8* }

@0 = private unnamed_addr constant [10 x i8] c"Name: %s\0A\00", align 1
@1 = private unnamed_addr constant [9 x i8] c"Age: %d\0A\00", align 1
@2 = private unnamed_addr constant [13 x i8] c"Address: %s\0A\00", align 1
@3 = private unnamed_addr constant [10 x i8] c"City: %s\0A\00", align 1
@4 = private unnamed_addr constant [11 x i8] c"State: %s\0A\00", align 1
@5 = private unnamed_addr constant [17 x i8] c"Postal Code: %s\0A\00", align 1
@6 = private unnamed_addr constant [8 x i8] c"&p: %p\0A\00", align 1

declare i32 @printf(i8*, ...)

define private void @_ZJ0f13print__person44f1v37p34t6Person20m17t7Address1x1x1x1x1x1b1p1p(%Person.1* %person) {
entry:
  %0 = getelementptr inbounds %Person.1, %Person.1* %person, i32 0, i32 1
  %1 = load i8*, i8** %0, align 8
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @0, i32 0, i32 0), i8* %1)
  %3 = getelementptr inbounds %Person.1, %Person.1* %person, i32 0, i32 2
  %4 = load i8, i8* %3, align 1
  %5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @1, i32 0, i32 0), i8 %4)
  %6 = getelementptr inbounds %Person.1, %Person.1* %person, i32 0, i32 0
  %7 = getelementptr inbounds %Address.0, %Address.0* %6, i32 0, i32 0
  %8 = load i8*, i8** %7, align 8
  %9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @2, i32 0, i32 0), i8* %8)
  %10 = getelementptr inbounds %Person.1, %Person.1* %person, i32 0, i32 0
  %11 = getelementptr inbounds %Address.0, %Address.0* %10, i32 0, i32 1
  %12 = load i8*, i8** %11, align 8
  %13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @3, i32 0, i32 0), i8* %12)
  %14 = getelementptr inbounds %Person.1, %Person.1* %person, i32 0, i32 0
  %15 = getelementptr inbounds %Address.0, %Address.0* %14, i32 0, i32 2
  %16 = load i8*, i8** %15, align 8
  %17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @4, i32 0, i32 0), i8* %16)
  %18 = getelementptr inbounds %Person.1, %Person.1* %person, i32 0, i32 0
  %19 = getelementptr inbounds %Address.0, %Address.0* %18, i32 0, i32 3
  %20 = load i8*, i8** %19, align 8
  %21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @5, i32 0, i32 0), i8* %20)
  br label %end

end:                                              ; preds = %entry
  ret void
}

define void @main() {
entry:
  %p = alloca %Person.1, align 8
  %0 = load %Person.1, %Person.1* %p, align 8
  call void @_ZJ0f13print__person44f1v37p34t6Person20m17t7Address1x1x1x1x1x1b1p1p(%Person.1* %p)
  %1 = load %Person.1, %Person.1* %p, align 8
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @6, i32 0, i32 0), %Person.1* %p)
  br label %end

end:                                              ; preds = %entry
  ret void
}
