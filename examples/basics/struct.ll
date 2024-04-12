; ModuleID = 'examples/basics/struct.q'
source_filename = "examples/basics/struct.q"

%RectanglurPrism3d.3 = type { [8 x %Point.0], [12 x %Line.1], [6 x %Texture.2*] }
%Point.0 = type { i32, i32 }
%Line.1 = type { %Point.0, %Point.0 }
%Texture.2 = type { i8*, i32, i32 }

@_ZJ0l4data108t17RectanglurPrism3d17a11t5Point1i1i2x839a32t4Line11t5Point1i1i11t5Point1i1i3x1226a20p17t7Texture3p1b1i1i2x60 = global %RectanglurPrism3d.3 zeroinitializer

define void @main() {
entry:
  br label %end

end:                                              ; preds = %entry
  ret void
}
