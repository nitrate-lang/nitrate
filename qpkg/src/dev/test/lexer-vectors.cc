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
        {
            {"L002", "OpCrunch"}, {" ", {}}  //
        },
        {
            {"L003", "PuncCrunch"}, {" ", {}}  //
        },
        {
            {"L004", "IntCrunch"}, {" ", {}}  //
        },
        {
            {"L005", "IntNorm"}, {" ", {}}  //
        },
        {
            {"L006", "StrCrunch"}, {" ", {}}  //
        },
        {
            {"L007", "MacCrunch"}, {" ", {}}  //
        },
        {
            {"L008", "NoteMix"}, {" ", {}}  //
        },
};

#endif
