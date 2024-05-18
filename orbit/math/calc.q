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
    pub ~> General Math
    {
        group Term ~> ax^b
        {
            a: Real,
            b: Real,

            try fn operator()(x: Real): Real {
                ret a * pow(x, b);
            }
        }

        group Polynomial ~> a_0 + a_1x + a_2x^2 + ... + a_nx^n
        {
            terms: [Term],

            require {
                terms.len() > 0;
            }

            fn clean() {
                terms = terms.filter((term) => term.a != 0);
            }

            pub try fn operator()(x: Real): Real {
                ret terms.each((term) => term(x)).sum();
            }

            pub try fn operator()(x: [Real]): Real {
                ret terms.each((term, i) => term(x[i])).sum();
            }

            pub try fn derivative(): Polynomial {
                ret Polynomial(terms.each((term) => {
                    ret {
                        a: term.a * term.b,
                        b: term.b - 1
                    };
                })).clean();
            }

            pub try fn integral(): Polynomial {
                ret Polynomial(terms.each((term) => {
                    ret {
                        a: term.a / (term.b + 1),
                        b: term.b + 1
                    };
                }).append({a: 1, b: 1})).clean(); ~> Don't forget C!
            }
        }
    }

    pub ~> Discrete Calculus Functions
    {
        fn gradient(f: (x: Real): Real): (x: Real): Real {
            ~> Let f(x) = y
            ~> Return the derivative of f(x)
        }
    }
}