; ModuleID = 'examples/basics/fn-stack-multi-alloc.q'
source_filename = "examples/basics/fn-stack-multi-alloc.q"

define i8 @main() {
entry:
  %x = alloca i8, align 1
  %y = alloca i8, align 1
  %z = alloca i8, align 1
  store i8 6, i8* %z, align 1
  %x1 = load i8, i8* %x, align 1
  %y2 = load i8, i8* %y, align 1
  %z3 = load i8, i8* %z, align 1
  %0 = add i8 %y2, %z3
  %1 = add i8 %x1, %0
  ret i8 %1
}
