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

subsystem Math {
    pub ~> Trigonometric Functions 
    {
        opaque try fn nothrow sin<acc|perf>(x: number): number;
        opaque try fn nothrow cos<acc|perf>(x: number): number;
        opaque try fn nothrow tan<acc|perf>(x: number): number;
        opaque try fn nothrow asin<acc|perf>(x: number): number;
        opaque try fn nothrow acos<acc|perf>(x: number): number;
        opaque try fn nothrow atan<acc|perf>(x: number): number;
        opaque try fn nothrow sinh<acc|perf>(x: number): number;
        opaque try fn nothrow cosh<acc|perf>(x: number): number;
        opaque try fn nothrow tanh<acc|perf>(x: number): number;
        opaque try fn nothrow asinh<acc|perf>(x: number): number;
        opaque try fn nothrow acosh<acc|perf>(x: number): number;
        opaque try fn nothrow atanh<acc|perf>(x: number): number;
    }
    
    pub ~> Power & Logarithmic Functions
    {
        opaque try fn nothrow exp<acc|perf>(x: number): number;
        opaque try fn nothrow loge<acc|perf>(x: number): number;
        opaque try fn nothrow log10<acc|perf>(x: number): number;
        opaque try fn nothrow log2<acc|perf>(x: number): number;
        opaque try fn nothrow pow<acc|perf>(x: number, y: number): number;
        opaque try fn nothrow sqrt<acc|perf>(x: number): number;
        opaque try fn nothrow cbrt<acc|perf>(x: number): number;
        opaque try fn nothrow rootn<acc|perf>(x: number, n: number): number;
    }
    
    pub ~> Rounding Functions
    {
        opaque try fn nothrow abs<acc|perf>(x: number): number;
        opaque try fn nothrow ceil<acc|perf>(x: number): number;
        opaque try fn nothrow floor<acc|perf>(x: number): number;
        opaque try fn nothrow round<acc|perf>(x: number): number;
    }
    
    pub ~> Range Functions
    {
        opaque try fn nothrow min<acc|perf>(x: number, y: number): number;
        opaque try fn nothrow max<acc|perf>(x: number, y: number): number;
        opaque try fn nothrow clamp<acc|perf>(x: number, min: number, max: number): number;
        opaque try fn nothrow sign<acc|perf>(x: number): bool;
    }

    pub ~> Activation Functions
    {
        opaque try fn nothrow sigmoid<acc|perf>(x: number): number;
        opaque try fn nothrow lerp<acc|perf>(x: number, y: number, t: number): number;
        opaque try fn nothrow relu<acc|perf>(x: number): number;
        opaque try fn nothrow leaky_relu<acc|perf>(x: number, alpha: number): number;
        opaque try fn nothrow elu<acc|perf>(x: number, alpha: number): number;
        opaque try fn nothrow softplus<acc|perf>(x: number): number;
        opaque try fn nothrow swish<acc|perf>(x: number, beta: number): number;
        opaque try fn nothrow mish<acc|perf>(x: number): number;
        opaque try fn nothrow gelu<acc|perf>(x: number): number;
    }

    pub ~> Statistical Functions
    {
        opaque try fn nothrow mean<acc|perf>(x: [number]): number;
        opaque try fn nothrow median<acc|perf>(x: [number]): number;
        opaque try fn nothrow mode<acc|perf>(x: [number]): number;
        opaque try fn nothrow variance<acc|perf>(x: [number]): number;
        opaque try fn nothrow stdev<acc|perf>(x: [number]): number;
        opaque try fn nothrow skewness<acc|perf>(x: [number]): number;
        opaque try fn nothrow kurtosis<acc|perf>(x: [number]): number;
        opaque try fn nothrow percentile<acc|perf>(x: [number], p: number): number;
        opaque try fn nothrow quantile<acc|perf>(x: [number], q: number): number;
        opaque try fn nothrow iqr<acc|perf>(x: [number]): number;
    }
}
