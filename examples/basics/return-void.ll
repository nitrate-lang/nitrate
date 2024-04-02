; ModuleID = 'examples/basics/return-void.q'
source_filename = "examples/basics/return-void.q"

define void @main() {
entry:
  br label %end

end:                                              ; preds = %entry
  ret void
}
