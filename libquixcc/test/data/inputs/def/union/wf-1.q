union Point {
    x: i32,
    y: i32,
}

union Line {
    start: Point,
    end: Point,
}

union Texture {
    data: *u8,
    width: i32,
    height: i32,
}

union RectanglurPrism3d {
    vertices: [Point; 8],
    lines: [Line; 12],
    textures: [*Texture; 6],
}