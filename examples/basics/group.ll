; ModuleID = 'examples/basics/group.q'
source_filename = "examples/basics/group.q"

%RectanglurPrism3d.3 = type { [12 x %Line.2], [8 x %Point.0], [6 x %Texture.1*] }
%Line.2 = type { %Point.0, %Point.0 }
%Point.0 = type { i32, i32 }
%Texture.1 = type { i8*, i32, i32 }

@_ZJ0l4data108t17RectanglurPrism3d39a32t4Line11t5Point1i1i11t5Point1i1i3x1217a11t5Point1i1i2x826a20p17t7Texture3p1b1i1i2x60 = global %RectanglurPrism3d.3 zeroinitializer

define void @main() {
entry:
  br label %end

end:                                              ; preds = %entry
  ret void
}
