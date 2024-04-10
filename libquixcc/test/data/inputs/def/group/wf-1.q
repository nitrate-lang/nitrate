group Point {
    x: i32,
    y: i32,
}

group Line {
    start: Point,
    end: Point,
}

group Texture {
    data: *u8,
    width: i32,
    height: i32,
}

group RectanglurPrism3d {
    vertices: [Point; 8],
    lines: [Line; 12],
    textures: [*Texture; 6],
}