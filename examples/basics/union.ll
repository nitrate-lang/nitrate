; ModuleID = 'examples/basics/union.q'
source_filename = "examples/basics/union.q"

%0 = type { i32, [4 x i8] }
%1 = type { i32, [8 x i8] }

@_ZJ0l2x07u1g1i1d0 = global %0 zeroinitializer
@_ZJ0l2x17u1e1l1q0 = global %1 zeroinitializer

define void @main() {
entry:
  br label %end

end:                                              ; preds = %entry
  ret void
}
