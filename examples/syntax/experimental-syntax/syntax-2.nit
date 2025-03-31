@import traits;

scope Simulator {
    group Vec<<N>> {
        data: [f32; N as ct_number],

        require {
            N > 0,
        }

        try fn operator+(other: Vec<N>): Vec<N> {
            ret data.map((x: f32, i: i32): f32 => x + other.data[i]);
        }

        try fn operator-(other: Vec<N>): Vec<N> {
            ret data.map((x: f32, i: i32): f32 => x - other.data[i]);
        }

        try fn operator*(other: Vec<N>): f32 {
            ret data.map((x: f32, i: i32): f32 => x * other.data[i]).sum();
        }

        try fn operator*(scalar: f32): Vec<N> {
            ret data.map((x: f32): f32 => x * scalar);
        }

        try fn operator/(scalar: f32): Vec<N> {
            ret data.map((x: f32): f32 => x / scalar);
        }
    } : iterator{data}, finite_sequence{data}, conv{data}

    group Mat<<A>x<<B>> {
        data: [[f32; B as ct_number]; A as ct_number],

        require {
            A > 0,
            B > 0,
        }

        try fn operator*(other: Mat<A>x<B>): Mat<A>x<B> {
            ret data.map((row: [f32; B as ct_number], i: i32): [f32; B as ct_number]: {
                ret row.map((x: f32, j: i32): f32 => x * other.data[j][i]).sum();
            });
        }

        try fn operator*(vec: Vec<B>): Vec<A> {
            ret data.map((row: [f32; B as ct_number]): f32 => row * vec);
        }

        try fn operator*(scalar: f32): Mat<A>x<B> {
            ret data.map((row: [f32; B as ct_number]): [f32; B as ct_number]: {
                ret row.map((x: f32): f32 => x * scalar);
            });
        }

        try fn operator/(scalar: f32): Mat<A>x<B> {
            ret data.map((row: [f32; B as ct_number]): [f32; B as ct_number]: {
                ret row.map((x: f32): f32 => x / scalar);
            });
        }

        try fn operator+(other: Mat<A>x<B>): Mat<A>x<B> {
            ret data.map((row: [f32; B as ct_number], i: i32): [f32; B as ct_number]: {
                ret row.map((x: f32, j: i32): f32 => x + other.data[i][j]);
            });
        }

        try fn operator-(other: Mat<A>x<B>): Mat<A>x<B> {
            ret data.map((row: [f32; B as ct_number], i: i32): [f32; B as ct_number]: {
                ret row.map((x: f32, j: i32): f32 => x - other.data[i][j]);
            });
        }

        try fn det(): f32 {
            require {
                A == B,
            }

            retif A == 1, data(0);
            retif A == 2, data(0) * data(1) - data[0][1] * data[1][0];

            ret data[0].map((x: f32, i: i32): f32 => {
                ret x * Mat<A-1>x<B-1>{data.map((row: [f32; B as ct_number], j: i32): [f32; B-1 as ct_number]: {
                    ret row.filter((_, k: i32): bool => k != i).map((x: f32): f32 => x);
                })}.det() * if i % 2 == 0 {1.0} else {-1.0};
            }).sum();
        }

        try fn transpose(): Mat<B>x<A> {
            ret data.map((row: [f32; B as ct_number], i: i32): [f32; A as ct_number]: {
                ret row.map((x: f32, j: i32): f32 => data[j][i]);
            });
        }

        try fn inverse(): Mat<A>x<B> {
            require {
                A == B,
                det() != 0.0,
            }

            ret Mat<A>x<B>{data.map((row: [f32; B as ct_number], i: i32): [f32; B as ct_number]: {
                ret row.map((_, j: i32): f32 => {
                    ret Mat<A-1>x<B-1>{data.map((row: [f32; B as ct_number], k: i32): [f32; B-1 as ct_number]: {
                        ret row.filter((_, l: i32): bool => l != j).map((x: f32): f32 => x);
                    }).filter((_, k: i32): bool => k != i).det() * if (i + j) % 2 == 0 {1.0} else {-1.0};
                });
            })}.transpose() / det();
        }
    } : iterator{data}, md_array{data}, conv{data}

    type PointCloud3D = [Vec3];

    group Rigidbody3D {
        pos: Vec3,
        vel: Vec3,
        acc: Vec3,
        mass: f32,
        shape: PointCloud3D,

        require {
            mass > 0.0,
            acc.all((x: f32) => +x != inf;),
            vel.all((x: f32) => +x != inf;),
            shape.len() > 0,
        }

        fn is_colliding(other: Rigidbody3D): bool {
            const epsilon = 0.0001;

            ret shape.iter().any((point: Vec3): bool => {
                ret other.points.iter().any((other_point: Vec3): bool => {
                    ret (point - other_point).length() < epsilon;
                })
            });
        }

        fn apply_force(force: Vec3) {
            this.acc += force / this.mass;
        }
    } : conv{pos, vel, acc, mass, shape}<trunc|lz4>

    type render_agent_f = fn (obj: Rigidbody3D);

    fn simulate(cb: render_agent_f) {
        let r = Rigidbody3D(
            Vec3::zeros(),
            Vec3::zeros(),
            Vec3::zeros(),
            1.0,
            [Vec3::zeros()],
        );

        let g = 9.81;
        let dt = 0.01;
        let i: usize = 0;
        const fps = 60;
        const max_iter = 1000;

        const rot: Mat4x4 = [
            [1.0, 0.0, 0.0, 0.0],
            [0.0, cos(PI / 4), -sin(PI / 4), 0.0],
            [0.0, sin(PI / 4), cos(PI / 4), 0.0],
            [0.0, 0.0, 0.0, 1.0],
        ];

        chrono::every<fps>(fps, (t: f32): void => {
            cb(r);
        }).as(r != undef).body(() => {
            r.apply_force(Vec3{0.0, -r.mass * g, 0.0});
            r.vel += r.acc * dt;
            r.pos += r.vel * dt;
            r.acc = Vec3::zeros();

            r.shape = r.shape.map((point: Vec3): Vec3 => {
                ret (rot * point.extend(1.0)).trunc();
            });

            brkif i++ >= max_iter;
        }).go();
    }
}

fn main(): i32 {
    Simulator::simulate();
    ret 0;
}

@import core;
@import math;
@import chrono;
@import physics;
@import window;
@import graphics;

group Terrain {

} :