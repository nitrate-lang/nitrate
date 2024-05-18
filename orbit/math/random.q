///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///    ░▒▓██████▓▒░  ░▒▓███████▓▒░  ░▒▓███████▓▒░  ░▒▓█▓▒░ ░▒▓████████▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///   ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///   ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓███████▓▒░  ░▒▓███████▓▒░  ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///   ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///   ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///    ░▒▓██████▓▒░  ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓███████▓▒░  ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///                                                                         ///
///     * ORBIT STANDARD LIBRARY - The official stdlib for QUIX.            ///
///     * Copyright (c) 2024, Wesley C. Jones.                              ///
///     * License terms may be found in the LICENSE file.                   ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

@use "v1.0";

@copyright "Wesley Jones";
@description "Mathematics Library";
@language "en";
@warranty "As-Is; No Warranty";

@import core;

subsystem Math {
    pub opaque notry fn randomfill<T> (min: f64, max: f64): T;

    pub fn uniform<T>(min: f64, max: f64): T {
        ret randomfill<T>(min, max);
    }

    pub fn normal<T>(mean: f64, stddev: f64): T {
        // Box-Muller transform
        const gen = (): f64 => {
            let u: f64, v: f64, help: f64;

            do {
                u = Math.uniform<f64>(-0.5, 0.5);
                v = Math.uniform<f64>(-0.5, 0.5);
                help = u * u + v * v;
            } while (help >= 0.25);

            help = sqrt(log(help * 4.0) / help * -2.0);
            u *= help;
            v *= help;

            ret u * stddev + mean;
        };

        ret T().bytes().map(_ => gen()) as T;
    }
}

group Packet {
    x: f32,
    y: f32,
    v: [f32; 2],
}

fn demo() {
    let r = Math.uniform<Packet>(0, 100);
    println("Random packet: ", r);

    let n = Math.normal<Packet>(0, 1);
    println("Normal packet: ", n);
}