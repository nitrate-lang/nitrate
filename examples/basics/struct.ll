; ModuleID = 'examples/basics/struct.q'
source_filename = "examples/basics/struct.q"

%RectanglurPrism3d.3 = type { [8 x %Point.0], [12 x %Line.1], [6 x %Texture.2*] }
%Point.0 = type { i32, i32 }
%Line.1 = type { %Point.0, %Point.0 }
%Texture.2 = type { i8*, i32, i32 }

@_ZJ0l4data141t17RectanglurPrism3d24m21a15t5Point3m1i3m1i2x856m53a46t4Line18m15t5Point3m1i3m1i18m15t5Point3m1i3m1i3x1235m32a26p23t7Texture5m3p1b3m1i3m1i2x60 = global %RectanglurPrism3d.3 zeroinitializer

define void @main() {
entry:
  br label %end

end:                                              ; preds = %entry
  ret void
}
