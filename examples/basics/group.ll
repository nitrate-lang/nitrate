; ModuleID = 'examples/basics/group.q'
source_filename = "examples/basics/group.q"

%RectanglurPrism3d.3 = type { [12 x %Line.2], [8 x %Point.0], [6 x %Texture.1*] }
%Line.2 = type { %Point.0, %Point.0 }
%Point.0 = type { i32, i32 }
%Texture.1 = type { i8*, i32, i32 }

@_ZJ0l4data141t17RectanglurPrism3d56m53a46t4Line18m15t5Point3m1i3m1i18m15t5Point3m1i3m1i3x1224m21a15t5Point3m1i3m1i2x835m32a26p23t7Texture5m3p1b3m1i3m1i2x60 = global %RectanglurPrism3d.3 zeroinitializer

define void @main() {
entry:
  br label %end

end:                                              ; preds = %entry
  ret void
}
