////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///           ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░                  ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒▒▓███▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///           ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///             ░▒▓█▓▒░                                                      ///
///              ░▒▓██▓▒░                                                    ///
///                                                                          ///
///   * QUIX PACKAGE MANAGER - The official tool for the Quix language.      ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <core/Config.hh>

#if QPKG_DEV_TOOLS

#include <quixcc/EngineAPI.h>

#include <dev/test/test.hh>

#define KW(x) quixcc_tok_new_kw(x)
#define OP(x) quixcc_tok_new_op(x)
#define PUNCT(x) quixcc_tok_new_punct(x)
#define IDENT(x) quixcc_tok_new_ident(NULL, x)
#define INT(x) quixcc_tok_new(NULL, QUIXCC_LEX_INT, x)
#define FLOAT(x) quixcc_tok_new(NULL, QUIXCC_LEX_FLOAT, x)
#define NOTE(x) quixcc_tok_new(NULL, QUIXCC_LEX_NOTE, x)
#define MACROSLN(x) quixcc_tok_new(NULL, QUIXCC_LEX_METASEG, x)

const std::map<std::pair<std::string_view, std::string_view>,
               std::pair<std::string_view, std::vector<quixcc_tok_t>>>
    qpkg::dev::test::g_lexer_test_vectors = {
        {{"L001", "KwIdentMix"},
         {
             "subsystem.import.pub.type.let.var.const.static.struct.region."
             "group.union.opaque.enum.fstring.fn.noexcept.foreign.impure.tsafe."
             "pure.quasipure.retropure.crashpoint.inline.unsafe.safe.if.else."
             "for.while.do.switch.case.default.break.continue.ret.retif.retz."
             "retv.form.foreach.__asm__.void.undef.null.true.false.std::panic."
             "std::__weird.std::_19031__9_1::panic.binga0::____1__00::panic."
             "binga0::____1__00_::__weird.hello.hello0.\n",
             {
                 KW(QUIXCC_KW_SUBSYSTEM),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_IMPORT),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_PUB),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_TYPE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_LET),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_VAR),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_CONST),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_STATIC),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_STRUCT),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_REGION),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_GROUP),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_UNION),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_OPAQUE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_ENUM),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_FSTRING),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_FN),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_NOTHROW),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_FOREIGN),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_IMPURE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_TSAFE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_PURE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_QUASIPURE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_RETROPURE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_CRASHPOINT),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_INLINE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_UNSAFE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_SAFE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_IF),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_ELSE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_FOR),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_WHILE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_DO),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_SWITCH),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_CASE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_DEFAULT),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_BREAK),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_CONTINUE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_RETURN),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_RETIF),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_RETZ),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_RETV),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_FORM),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_FOREACH),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_ASM),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_VOID),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_UNDEF),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_NULL),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_TRUE),
                 OP(QUIXCC_OP_DOT),
                 KW(QUIXCC_KW_FALSE),
                 OP(QUIXCC_OP_DOT),
                 IDENT("std::panic"),
                 OP(QUIXCC_OP_DOT),
                 IDENT("std::__weird"),
                 OP(QUIXCC_OP_DOT),
                 IDENT("std::_19031__9_1::panic"),
                 OP(QUIXCC_OP_DOT),
                 IDENT("binga0::____1__00::panic"),
                 OP(QUIXCC_OP_DOT),
                 IDENT("binga0::____1__00_::__weird"),
                 OP(QUIXCC_OP_DOT),
                 IDENT("hello"),
                 OP(QUIXCC_OP_DOT),
                 IDENT("hello0"),
                 OP(QUIXCC_OP_DOT),
             },
         }},
        // {
        //     {"L002", "OpCrunch"}, {" ", {}}  //
        // },
        {{"L003", "PuncCrunch"},
         {"(){}[],:;><=+-*/%&|^!~?@#_\\\\\n",
          {
              PUNCT(QUIXCC_PUNCT_OPEN_PAREN),
              PUNCT(QUIXCC_PUNCT_CLOSE_PAREN),
              PUNCT(QUIXCC_PUNCT_OPEN_BRACE),
              PUNCT(QUIXCC_PUNCT_CLOSE_BRACE),
              PUNCT(QUIXCC_PUNCT_OPEN_BRACKET),
              PUNCT(QUIXCC_PUNCT_CLOSE_BRACKET),
              PUNCT(QUIXCC_PUNCT_COMMA),
              PUNCT(QUIXCC_PUNCT_COLON),
              PUNCT(QUIXCC_PUNCT_SEMICOLON),
              OP(QUIXCC_OP_GT),
              OP(QUIXCC_OP_LE),
              OP(QUIXCC_OP_PLUS),
              OP(QUIXCC_OP_MINUS),
              OP(QUIXCC_OP_MUL),
              OP(QUIXCC_OP_DIV),
              OP(QUIXCC_OP_MOD),
              OP(QUIXCC_OP_BIT_AND),
              OP(QUIXCC_OP_BIT_OR),
              OP(QUIXCC_OP_BIT_XOR),
              OP(QUIXCC_OP_NOT),
              OP(QUIXCC_OP_BIT_NOT),
              OP(QUIXCC_OP_TERNARY),
              MACROSLN("#_\\\\"),
          }}},
        // {
        //     {"L004", "IntCrunch"}, {" ", {}}  //
        // },
        {{"L005", "IntNorm"},
         {R"(## Random
0xA83FAEA1_F9F9_4562_BD5B_D693C68A2E1Eyssupehtnierehkcuf
0b10101000001111111010111010100001111110011111100101000101011000101011110101011011110101101001001111000110100010100010111000011110yssupehtnierehkcuf

# Max values
0b11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111yssupehtnierehkcuf
0xffffffffffffffffffffffffffffffffyssupehtnierehkcuf
0o3777777777777777777777777777777777777777777yssupehtnierehkcuf
0d340282366920938463463374607431768211455yssupehtnierehkcuf
340282366920938463463374607431768211455yssupehtnierehkcuf

# Random with separators
0b1101__00001111010010010110100111__101000010111001000101001000100110101001110100000001011101100011001101111011100001101000111110000__01yssupehtnierehkcuf
0b10011_11_0000000101111101110101010110010100100111___________________001101111100100000101101110100101100001010101010101010011110110100010110011100110yssupehtnierehkcuf
0o__2_36005756526__244715744_055_6454125252366426346yssupehtnierehkcuf
0x__9e02_fbaa___c_a_4_e_6__f905ba___5855553da2c__e6yssupehtnierehkcuf

# FP
1.602176634e-19yssupehtnierehkcuf
6.67430e-11yssupehtnierehkcuf
6.62607015e-34yssupehtnierehkcuf
299792458.0yssupehtnierehkcuf
8.8541878188e-12yssupehtnierehkcuf
1.25663706127e-6yssupehtnierehkcuf
9.1093837139e-31yssupehtnierehkcuf
0.0072973525643yssupehtnierehkcuf
483597.8484e9yssupehtnierehkcuf
10973731.568157yssupehtnierehkcuf
25812.80745yssupehtnierehkcuf
)",
          {
              NOTE("# Random"),
              INT("223640959966321878614961064210395180574"),
              IDENT("yssupehtnierehkcuf"),
              INT("223640959966321878614961064210395180574"),
              IDENT("yssupehtnierehkcuf"),
              NOTE(" Max values"),
              INT("340282366920938463463374607431768211455"),
              IDENT("yssupehtnierehkcuf"),
              INT("340282366920938463463374607431768211455"),
              IDENT("yssupehtnierehkcuf"),
              INT("340282366920938463463374607431768211455"),
              IDENT("yssupehtnierehkcuf"),
              INT("340282366920938463463374607431768211455"),
              IDENT("yssupehtnierehkcuf"),
              INT("340282366920938463463374607431768211455"),
              IDENT("yssupehtnierehkcuf"),
              NOTE(" Random with separators"),
              INT("277749398477639291874003272634275678145"),
              IDENT("yssupehtnierehkcuf"),
              INT("210033512343942716611728599312458460390"),
              IDENT("yssupehtnierehkcuf"),
              INT("210033512343942716611728599312458460390"),
              IDENT("yssupehtnierehkcuf"),
              INT("210033512343942716611728599312458460390"),
              IDENT("yssupehtnierehkcuf"),
              NOTE(" FP"),
              FLOAT("1.6021766340000001344974730936248e-19"),
              IDENT("yssupehtnierehkcuf"),
              FLOAT("6.6742999999999993790012436692643e-11"),
              IDENT("yssupehtnierehkcuf"),
              FLOAT("6.6260701499999998297249066757463e-34"),
              IDENT("yssupehtnierehkcuf"),
              FLOAT("299792458.0"),
              IDENT("yssupehtnierehkcuf"),
              FLOAT("8.8541878188000004144484891152275e-12"),
              IDENT("yssupehtnierehkcuf"),
              FLOAT("1.2566370612699998395240044701815e-06"),
              IDENT("yssupehtnierehkcuf"),
              FLOAT("9.1093837138999998164445228073493e-31"),
              IDENT("yssupehtnierehkcuf"),
              FLOAT("0.0072973525643"),
              IDENT("yssupehtnierehkcuf"),
              FLOAT("483597848400000"),
              IDENT("yssupehtnierehkcuf"),
              FLOAT("10973731.568157"),
              IDENT("yssupehtnierehkcuf"),
              FLOAT("25812.80745"),
              IDENT("yssupehtnierehkcuf"),
          }}},
        // {
        //     {"L006", "StrCrunch"}, {" ", {}}  //
        // },
        // {
        //     {"L007", "MacCrunch"}, {" ", {}}  //
        // },
        {
            {"L008", "NoteMix"},
            {R"(/* MY COMMENT // Path: test.q
 /*
  🌆🔥🩶

  编码很有趣
 */

 -> 0x1F306 0x1F525 0x1FA76
*/

#!/usr/bin/env q

/** DOC */

 // BUG: This is a bug
fn complex_api() { ~>QQ Comment /* DEBUG ST YADA */


  // If a is b, then 2+2=5

  ~># HELLO
  /*


  🌆
  🔥 --> __abort 🩶
  */

  ~> 0x1F306 0x1F525 0x1FA76
}

## test.q
)",
             {
                 NOTE(R"( MY COMMENT // Path: test.q
 /*
  🌆🔥🩶

  编码很有趣
 */

 -> 0x1F306 0x1F525 0x1FA76
)"),
                 NOTE("!/usr/bin/env q"),
                 NOTE("* DOC "),
                 NOTE(" BUG: This is a bug"),
                 KW(QUIXCC_KW_FN),
                 IDENT("complex_api"),
                 PUNCT(QUIXCC_PUNCT_OPEN_PAREN),
                 PUNCT(QUIXCC_PUNCT_CLOSE_PAREN),
                 PUNCT(QUIXCC_PUNCT_OPEN_BRACE),
                 NOTE(R"(QQ Comment /* DEBUG ST YADA */)"),
                 NOTE(" If a is b, then 2+2=5"),
                 NOTE("# HELLO"),
                 NOTE(R"(


  🌆
  🔥 --> __abort 🩶
  )"),
                 NOTE(" 0x1F306 0x1F525 0x1FA76"),
                 PUNCT(QUIXCC_PUNCT_CLOSE_BRACE),
                 NOTE("# test.q"),
             }},
        },
};

#endif
