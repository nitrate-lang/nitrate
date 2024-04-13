; ModuleID = 'examples/basics/namespace.q'
source_filename = "examples/basics/namespace.q"

@_ZJ0l18Number_CONSTANT__A1c0 = private global i8 10
@_ZJ0l18Number_CONSTANT__B1c0 = private global i8 20

define i8 @main() {
entry:
  %ret = alloca i8, align 1
  %x = alloca i8, align 1
  %0 = load i8, i8* @_ZJ0l18Number_CONSTANT__A1c0, align 1
  store i8 %0, i8* %x, align 1
  %y = alloca i8, align 1
  %1 = load i8, i8* @_ZJ0l18Number_CONSTANT__B1c0, align 1
  store i8 %1, i8* %y, align 1
  store i8 9, i8* %ret, align 1
  br label %end

end:                                              ; preds = %entry
  %2 = load i8, i8* %ret, align 1
  ret i8 %2
}
