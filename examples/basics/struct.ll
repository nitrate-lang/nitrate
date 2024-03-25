; ModuleID = 'examples/basics/struct.q'
source_filename = "examples/basics/struct.q"

%0 = type { [8 x %1], [12 x %2], [6 x void*] }
%1 = type { i32, i32 }
%2 = type { %3, %4 }
%3 = type { i32, i32 }
%4 = type { i32, i32 }

@_ZJ0l4data44t10a5t1i1i2x820a13t5t1i1i5t1i1i3x128a3p1v2x60 = global %0 zeroinitializer

define void @main() {
entry:
  ret void
}
