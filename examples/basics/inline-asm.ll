; ModuleID = 'examples/basics/inline-asm.q'
source_filename = "examples/basics/inline-asm.q"

@0 = private unnamed_addr constant [8 x i8] c"x = %d\0A\00", align 1

declare i32 @printf(i8*, ...)

define private i32 @_ZJ0f15do__inline__asm5f1i1p1p() {
entry:
  %ret = alloca i32, align 4
  %x = alloca i32, align 4
  %0 = load i32, i32* %x, align 4
  %1 = call i32 asm sideeffect "mov $$42, $0", "=r,~{esi},~{dirflag},~{fpsr},~{flags}"()
  store i32 %1, i32* %x, align 4
  %2 = load i32, i32* %x, align 4
  store i32 %2, i32* %ret, align 4
  br label %end

end:                                              ; preds = %entry
  %3 = load i32, i32* %ret, align 4
  ret i32 %3
}

define void @main() {
entry:
  %x = alloca i32, align 4
  %0 = call i32 @_ZJ0f15do__inline__asm5f1i1p1p()
  store i32 %0, i32* %x, align 4
  %1 = load i32, i32* %x, align 4
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @0, i32 0, i32 0), i32 %1)
  br label %end

end:                                              ; preds = %entry
  ret void
}
