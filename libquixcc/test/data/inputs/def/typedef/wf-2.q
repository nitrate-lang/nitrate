struct RetVal {  x: i32, }
struct ArgA { x: i32, }
struct ArgB { x: i32, }

type callback = fn _(x: ArgA, y: ArgB, z: i32 = 10): *RetVal;