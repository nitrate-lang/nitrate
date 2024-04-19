; ModuleID = 'examples/basics/union.q'
source_filename = "examples/basics/union.q"

%0 = type { i32, [4 x i8] }
%1 = type { i32, [8 x i8] }

@_ZJ0l2x022u8F32_I32U3m1g3m1i3m1d0 = global %0 zeroinitializer
@_ZJ0l2x122u8F64_I64U3m1e3m1l3m1q0 = global %1 zeroinitializer

define void @main() {
entry:
  br label %end

end:                                              ; preds = %entry
  ret void
}
