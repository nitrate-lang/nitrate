; ModuleID = 'struct.q'
source_filename = "struct.q"

%0 = type { %1, i1, i16, [8 x i8] }
%1 = type { %2, %3 }
%2 = type { [32 x i8] }
%3 = type { [32 x i8] }

@_ZJ0c23____AUTHOR____struct__q1x0 = private global i8* getelementptr inbounds ([13 x i8], [13 x i8]* @_JncCHryDsb, i32 0, i32 0)
@_JncCHryDsb = private global [13 x i8] c"Wesley Jones\00"
@_ZJ0c24____LICENSE____struct__q1x0 = private global i8* getelementptr inbounds ([4 x i8], [4 x i8]* @_zayy4cBWDx, i32 0, i32 0)
@_zayy4cBWDx = private global [4 x i8] c"mit\00"
@_ZJ0v7keypair32t19t8t6a1b2328t6a1b2321y1s5a1b180 = global %0 zeroinitializer

define private void @_start() {
entry:
  br label %loop

loop:                                             ; preds = %loop, %entry
  br label %loop
}
