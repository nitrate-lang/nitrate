struct Point {
    x: i32,
    y: i32,
}

struct Line {
    start: Point,
    end: Point,
}

struct Texture {
    data: *u8,
    width: i32,
    height: i32,
}

struct RectanglurPrism3d {
    vertices: [Point; 8],
    lines: [Line; 12],
    textures: [*Texture; 6],
}