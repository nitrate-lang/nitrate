region Point {
    x: i32,
    y: i32,
}

region Line {
    start: Point,
    end: Point,
}

region Texture {
    data: *u8,
    width: i32,
    height: i32,
}

region RectanglurPrism3d {
    vertices: [Point; 8],
    lines: [Line; 12],
    textures: [*Texture; 6],
}