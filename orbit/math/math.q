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
    @import constants;
    // @import core;
    // @import calc;
    // @import random;

    pub "C" fn quixcc_callme() {
        fn printf(fmt: string): i32;

        printf("** You have successfully called a QUIX function from C!\n");
        printf("Don't you just love it when languages interoperate?\n");
        printf("I sure do!\n");
        printf("Okay. Thats enough for now.\n");
        printf("** Goodbye!\n");
    }
}