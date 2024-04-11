; ModuleID = 'examples/basics/group.q'
source_filename = "examples/basics/group.q"

%0 = type { [12 x %1], [8 x %4], [6 x %5*] }
%1 = type { %2, %3 }
%2 = type { i32, i32 }
%3 = type { i32, i32 }
%4 = type { i32, i32 }
%5 = type { i8*, i32, i32 }

@_ZJ0l4data54t20a13t5t1i1i5t1i1i3x1210a5t1i1i2x817a11p9t3p1b1i1i2x60 = global %0 zeroinitializer

define void @main() {
entry:
  br label %end

end:                                              ; preds = %entry
  ret void
}
