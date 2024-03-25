; ModuleID = 'examples/basics/fn-stack-alloc.q'
source_filename = "examples/basics/fn-stack-alloc.q"

define i8 @main() {
entry:
  %x = alloca i8, align 1
  store i8 10, i8* %x, align 1
  %y = alloca i8, align 1
  store i8 20, i8* %y, align 1
  %x1 = load i8, i8* %x, align 1
  %y2 = load i8, i8* %y, align 1
  %0 = add i8 %x1, %y2
  ret i8 %0
}
