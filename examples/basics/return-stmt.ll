; ModuleID = 'examples/basics/return-stmt.q'
source_filename = "examples/basics/return-stmt.q"

define i8 @main() {
entry:
  %0 = add i8 1, 2
  %1 = mul i8 %0, 3
  %2 = sub i8 %1, 9
  ret i8 %2
}
