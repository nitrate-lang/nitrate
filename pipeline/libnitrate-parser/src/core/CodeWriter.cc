////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <boost/multiprecision/cpp_int.hpp>
#include <core/CodeWriter.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Grammar.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

namespace ncc::parse {
  class CodeWriter final : public parse::ICodeWriter {
    std::ostream& m_os;
    lex::TokenType m_last{};
    lex::TokenData m_ldata;
    bool m_did_root{};

    static bool IsWordOperator(lex::Operator op) {
      using namespace ncc::lex;

      switch (op) {
        case OpAs:
        case OpBitcastAs:
        case OpIn:
        case OpOut:
        case OpSizeof:
        case OpBitsizeof:
        case OpAlignof:
        case OpTypeof:
        case OpComptime:
          return true;

        default:
          return false;
      }
    }

    void PutKeyword(lex::Keyword kw) {
      using namespace ncc::lex;

      switch (m_last) {
        case KeyW:
        case Name:
        case Macr:
        case NumL: {
          m_os << ' ' << kw;
          break;
        }

        case Oper: {
          if (IsWordOperator(m_ldata.m_op)) {
            m_os << ' ' << kw;
          } else {
            m_os << kw;
          }
          break;
        }

        case EofF:
        case Punc:
        case IntL:
        case Text:
        case Char:
        case MacB:
        case Note: {
          m_os << kw;
          break;
        }
      }

      m_last = KeyW;
      m_ldata = kw;
    }

    void PutOperator(lex::Operator op) {
      using namespace ncc::lex;

      switch (m_last) {
        case KeyW:
        case Name:
        case Macr:
        case IntL:
        case NumL: {
          if (IsWordOperator(op)) {
            m_os << ' ' << op;
          } else {
            m_os << op;
          }
          break;
        }

        case Oper: {
          auto table_row = static_cast<size_t>(m_ldata.m_op) - static_cast<size_t>(Op_First);
          auto table_col = static_cast<size_t>(op) - static_cast<size_t>(Op_First);
          auto table_idx = table_row * kNumberOfOperators + table_col;

          if (kOpOnOpAction[table_idx] == ' ') {
            m_os << ' ' << op;
          } else {
            m_os << op;
          }
          break;
        }

        case Punc: {
          m_os << op;
          break;
        }

        case EofF:
        case Text:
        case Char:
        case MacB:
        case Note: {
          m_os << op;
          break;
        }
      }

      m_last = Oper;
      m_ldata = op;
    }

    static constexpr auto kNumberOfOperators = lex::Op_Last - lex::Op_First + 1;

    /// FIXME: Optimize this lookup table to minimize redundant whitespace
    static constexpr std::array<char, (kNumberOfOperators * kNumberOfOperators) + 1> kOpOnOpAction = {
        /*
         OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
         ppppppppppppppppppppppppppppppppppppppppppppppppppppppp
         PMTSPBBBBLRRRLLLLLGLGENSPMTSPBBBLLLLRRRIDABIOSBATCDREAT
         liileiiiiSSOOooooTTEEqEeliileiiioooSSOOnesinuiilyooalre
         unmartttthhTTgggg      tunmartttggghhTTcc t tztipmtnlrr
         suescAOXNiiLRiiii       suescAOXiiiiiLR   c  esgep gion
          sshenrooff  cccc       SsshenrocccffSS   a  oinot epwa
             nd rttt  AOXN       eSSSndSrAOXttee   s  fzofi  s r
             t        nroo       teeetSeSnroSStt   t   ef m  i y
                      d rt        tttSetedSree     A   o  e  s
                                     et tSeStt     s   f
                                     t   ete
                                         t t                                */
        "                                                       " /* OpPlus */
        "                                                       " /* OpMinus */
        "                                                       " /* OpTimes */
        "                                                       " /* OpSlash */
        "                                                       " /* OpPercent */
        "                                                       " /* OpBitAnd */
        "                                                       " /* OpBitOr */
        "                                                       " /* OpBitXor */
        "                                                       " /* OpBitNot */
        "                                                       " /* OpLShift */
        "                                                       " /* OpRShift */
        "                                                       " /* OpROTL */
        "                                                       " /* OpROTR */
        "                                                       " /* OpLogicAnd */
        "                                                       " /* OpLogicOr */
        "                                                       " /* OpLogicXor */
        "                                                       " /* OpLogicNot */
        "                                                       " /* OpLT */
        "                                                       " /* OpGT */
        "                                                       " /* OpLE */
        "                                                       " /* OpGE */
        "                                                       " /* OpEq */
        "                                                       " /* OpNE */
        "                                                       " /* OpSet */
        "                                                       " /* OpPlusSet */
        "                                                       " /* OpMinusSet */
        "                                                       " /* OpTimesSet */
        "                                                       " /* OpSlashSet */
        "                                                       " /* OpPercentSet */
        "                                                       " /* OpBitAndSet */
        "                                                       " /* OpBitOrSet */
        "                                                       " /* OpBitXorSet */
        "                                                       " /* OpLogicAndSet */
        "                                                       " /* OpLogicOrSet */
        "                                                       " /* OpLogicXorSet */
        "                                                       " /* OpLShiftSet */
        "                                                       " /* OpRShiftSet */
        "                                                       " /* OpROTLSet */
        "                                                       " /* OpROTRSet */
        "                                                       " /* OpInc */
        "                                                       " /* OpDec */
        "                                                       " /* OpAs */
        "                                                       " /* OpBitcastAs */
        "                                                       " /* OpIn */
        "                                                       " /* OpOut */
        "                                                       " /* OpSizeof */
        "                                                       " /* OpBitsizeof */
        "                                                       " /* OpAlignof */
        "                                                       " /* OpTypeof */
        "                                                       " /* OpComptime */
        "                                                       " /* OpDot */
        "                                                       " /* OpRange */
        "                                                       " /* OpEllipsis */
        "                                                       " /* OpArrow */
        "                                                       " /* OpTernary */
    };

    static std::string StringEscape(std::string_view str) {
      std::stringstream ss;

      for (char ch : str) {
        switch (ch) {
          case '\n': {
            ss << "\\n";
            break;
          }

          case '\t': {
            ss << "\\t";
            break;
          }

          case '\r': {
            ss << "\\r";
            break;
          }

          case '\v': {
            ss << "\\v";
            break;
          }

          case '\f': {
            ss << "\\f";
            break;
          }

          case '\b': {
            ss << "\\b";
            break;
          }

          case '\a': {
            ss << "\\a";
            break;
          }

          case '\\': {
            ss << "\\\\";
            break;
          }

          case '"': {
            ss << "\\\"";
            break;
          }

          default: {
            ss << ch;
            break;
          }
        }
      }

      return ss.str();
    }

    void PutPunctor(lex::Punctor punc) {
      using namespace ncc::lex;

      switch (m_last) {
        case Oper: {
          m_os << punc;
          break;
        }

        case Punc: {
          if ((punc == PuncColn || punc == PuncScope) && m_ldata.m_punc == PuncColn) {
            m_os << ' ' << punc;
          } else {
            m_os << punc;
          }

          break;
        }

        case IntL:
        case NumL:
        case Macr:
        case EofF:
        case KeyW:
        case Name:
        case Text:
        case Char:
        case MacB:
        case Note: {
          m_os << punc;
          break;
        }
      }

      m_last = Punc;
      m_ldata = punc;
    }

    void PutIdentifier(std::string_view name) {
      using namespace ncc::lex;

      switch (m_last) {
        case Oper: {
          if (IsWordOperator(m_ldata.m_op)) {
            m_os << ' ' << name;
          } else {
            m_os << name;
          }
          break;
        }

        case KeyW:
        case Name:
        case Macr: {
          m_os << ' ' << name;
          break;
        }

        case NumL: {
          /// FIXME: Minimize redundant whitespace
          m_os << ' ' << name;
          break;
        }

        case IntL: {
          bool was_it_hex = m_ldata.m_str->starts_with("0x");
          if (was_it_hex) {
            if (std::isxdigit(name.front()) == 0) {
              m_os << name;
            } else {
              m_os << ' ' << name;
            }
          } else {
            /**
             * Otherwise, it was in decimal. Therefore the allowed character is [0-9].
             * Because identifiers cannot start with a digit, we don't require whitespace.
             */

            m_os << name;
          }
          break;
        }

        case EofF:
        case Punc:
        case Text:
        case Char:
        case MacB:
        case Note: {
          m_os << name;
          break;
        }
      }

      m_last = Name;
      m_ldata = string(name);
    }

    static void WriteSmallestInteger(std::ostream& os, std::string_view num) {
      constexpr boost::multiprecision::uint128_t kFormatSwitchThreshold = 1000000000000;

      boost::multiprecision::uint128_t number(num);

      if (number < kFormatSwitchThreshold) {
        os << num;
        return;
      }

      os << "0x" << std::hex << number << std::dec;
    }

    void PutInteger(std::string_view num) {
      using namespace ncc::lex;

      switch (m_last) {
        case Oper: {
          if (IsWordOperator(m_ldata.m_op)) {
            m_os << ' ';
            WriteSmallestInteger(m_os, num);
          } else {
            WriteSmallestInteger(m_os, num);
          }
          break;
        }

        case KeyW:
        case Name:
        case IntL:
        case NumL:
        case Macr: {
          m_os << ' ';
          WriteSmallestInteger(m_os, num);
          break;
        }

        case EofF:
        case Punc:
        case Text:
        case Char:
        case MacB:
        case Note: {
          WriteSmallestInteger(m_os, num);
          break;
        }
      }

      m_last = IntL;
      m_ldata = string(num);
    }

    void PutFloat(std::string_view num) {
      using namespace ncc::lex;

      /// FIXME: Find algorigm to compress floating point numbers

      switch (m_last) {
        case Oper: {
          if (IsWordOperator(m_ldata.m_op)) {
            m_os << ' ' << num;
          } else {
            m_os << num;
          }
          break;
        }

        case KeyW:
        case Name:
        case IntL:
        case NumL:
        case Macr: {
          m_os << ' ' << num;
          break;
        }

        case EofF:
        case Punc:
        case Text:
        case Char:
        case MacB:
        case Note: {
          m_os << num;
          break;
        }
      }

      m_last = NumL;
      m_ldata = string(num);
    }

    void PutString(std::string_view str) {
      using namespace ncc::lex;

      switch (m_last) {
        case Text: {
          /* Adjacent strings are always concatenated */
          m_os << '"' << StringEscape(str) << '"';
          break;
        }

        case Char:
        case Oper:
        case KeyW:
        case Name:
        case IntL:
        case NumL:
        case Macr:
        case EofF:
        case Punc:
        case MacB:
        case Note: {
          m_os << '"' << StringEscape(str) << '"';
          break;
        }
      }

      m_last = Text;
      m_ldata = string(str);
    }

    void PutCharacter(uint8_t ch) {
      using namespace ncc::lex;

      std::string_view str(reinterpret_cast<const char*>(&ch), 1);

      switch (m_last) {
        case Char: {
          m_os << '\'' << StringEscape(str) << '\'';
          break;
        }

        case Text:
        case Oper:
        case KeyW:
        case Name:
        case IntL:
        case NumL:
        case Macr:
        case EofF:
        case Punc:
        case MacB:
        case Note: {
          m_os << '\'' << StringEscape(str) << '\'';
          break;
        }
      }

      m_last = Char;
      m_ldata = string(str);
    }

    void PutMacroBlock(std::string_view macro) {
      using namespace ncc::lex;

      switch (m_last) {
        case Text:
        case Char:
        case Oper:
        case KeyW:
        case Name:
        case IntL:
        case NumL:
        case Macr:
        case EofF:
        case Punc:
        case MacB:
        case Note: {
          m_os << "@(" << macro << ")";
          break;
        }
      }

      m_last = MacB;
      m_ldata = string(macro);
    }

    void PutMacroCall(std::string_view macro) {
      using namespace ncc::lex;

      switch (m_last) {
        case Text:
        case Char:
        case Oper:
        case KeyW:
        case Name:
        case IntL:
        case NumL:
        case Macr:
        case EofF:
        case Punc:
        case MacB:
        case Note: {
          m_os << "@" << macro;
          break;
        }
      }

      m_last = Macr;
      m_ldata = string(macro);
    }

    void PutComment(std::string_view note) {
      using namespace ncc::lex;

      switch (m_last) {
        case Text:
        case Char:
        case Oper:
        case KeyW:
        case Name:
        case IntL:
        case NumL:
        case Macr:
        case EofF:
        case Punc:
        case MacB:
        case Note: {
          m_os << "/*" << note << "*/";
          break;
        }
      }

      m_last = Note;
      m_ldata = string(note);
    }

    void PutTypeStuff(const FlowPtr<parse::Type>& n);
    void PrintLeading(const FlowPtr<parse::Expr>& n);
    void PrintTrailing(const FlowPtr<parse::Expr>& n);

  protected:
    void Visit(FlowPtr<parse::NamedTy> n) override;
    void Visit(FlowPtr<parse::InferTy> n) override;
    void Visit(FlowPtr<parse::TemplateType> n) override;
    void Visit(FlowPtr<parse::U1> n) override;
    void Visit(FlowPtr<parse::U8> n) override;
    void Visit(FlowPtr<parse::U16> n) override;
    void Visit(FlowPtr<parse::U32> n) override;
    void Visit(FlowPtr<parse::U64> n) override;
    void Visit(FlowPtr<parse::U128> n) override;
    void Visit(FlowPtr<parse::I8> n) override;
    void Visit(FlowPtr<parse::I16> n) override;
    void Visit(FlowPtr<parse::I32> n) override;
    void Visit(FlowPtr<parse::I64> n) override;
    void Visit(FlowPtr<parse::I128> n) override;
    void Visit(FlowPtr<parse::F16> n) override;
    void Visit(FlowPtr<parse::F32> n) override;
    void Visit(FlowPtr<parse::F64> n) override;
    void Visit(FlowPtr<parse::F128> n) override;
    void Visit(FlowPtr<parse::VoidTy> n) override;
    void Visit(FlowPtr<parse::PtrTy> n) override;
    void Visit(FlowPtr<parse::OpaqueTy> n) override;
    void Visit(FlowPtr<parse::TupleTy> n) override;
    void Visit(FlowPtr<parse::ArrayTy> n) override;
    void Visit(FlowPtr<parse::RefTy> n) override;
    void Visit(FlowPtr<parse::FuncTy> n) override;
    void Visit(FlowPtr<parse::Unary> n) override;
    void Visit(FlowPtr<parse::Binary> n) override;
    void Visit(FlowPtr<parse::PostUnary> n) override;
    void Visit(FlowPtr<parse::Ternary> n) override;
    void Visit(FlowPtr<parse::Integer> n) override;
    void Visit(FlowPtr<parse::Float> n) override;
    void Visit(FlowPtr<parse::Boolean> n) override;
    void Visit(FlowPtr<parse::String> n) override;
    void Visit(FlowPtr<parse::Character> n) override;
    void Visit(FlowPtr<parse::Null> n) override;
    void Visit(FlowPtr<parse::Undefined> n) override;
    void Visit(FlowPtr<parse::Call> n) override;
    void Visit(FlowPtr<parse::TemplateCall> n) override;
    void Visit(FlowPtr<parse::List> n) override;
    void Visit(FlowPtr<parse::Assoc> n) override;
    void Visit(FlowPtr<parse::Index> n) override;
    void Visit(FlowPtr<parse::Slice> n) override;
    void Visit(FlowPtr<parse::FString> n) override;
    void Visit(FlowPtr<parse::Identifier> n) override;
    void Visit(FlowPtr<parse::Block> n) override;
    void Visit(FlowPtr<parse::Variable> n) override;
    void Visit(FlowPtr<parse::Assembly> n) override;
    void Visit(FlowPtr<parse::If> n) override;
    void Visit(FlowPtr<parse::While> n) override;
    void Visit(FlowPtr<parse::For> n) override;
    void Visit(FlowPtr<parse::Foreach> n) override;
    void Visit(FlowPtr<parse::Break> n) override;
    void Visit(FlowPtr<parse::Continue> n) override;
    void Visit(FlowPtr<parse::Return> n) override;
    void Visit(FlowPtr<parse::ReturnIf> n) override;
    void Visit(FlowPtr<parse::Case> n) override;
    void Visit(FlowPtr<parse::Switch> n) override;
    void Visit(FlowPtr<parse::Typedef> n) override;
    void Visit(FlowPtr<parse::Function> n) override;
    void Visit(FlowPtr<parse::Struct> n) override;
    void Visit(FlowPtr<parse::Enum> n) override;
    void Visit(FlowPtr<parse::Scope> n) override;
    void Visit(FlowPtr<parse::Export> n) override;

  public:
    CodeWriter(std::ostream& os);
    ~CodeWriter() override = default;
  };
}  // namespace ncc::parse

static bool IsNamedParameter(std::string_view name) { return std::isdigit(name.at(0)) == 0; }

///=============================================================================

void CodeWriter::PutTypeStuff(const FlowPtr<parse::Type>& n) {
  if (n->GetRangeBegin() || n->GetRangeEnd()) {
    PutPunctor(PuncColn);
    PutPunctor(PuncLBrk);
    if (n->GetRangeBegin()) {
      n->GetRangeBegin().value()->Accept(*this);
    }
    PutPunctor(PuncColn);
    if (n->GetRangeEnd()) {
      n->GetRangeEnd().value()->Accept(*this);
    }
    PutPunctor(PuncRBrk);
  }

  if (n->GetWidth()) {
    PutPunctor(PuncColn);
    n->GetWidth().value()->Accept(*this);
  }
}

void CodeWriter::PrintLeading(const FlowPtr<parse::Expr>& n) {
  auto depth = n->GetParenthesisDepth();
  for (size_t i = 0; i < depth; ++i) {
    PutPunctor(PuncLPar);
  }
}

void CodeWriter::PrintTrailing(const FlowPtr<parse::Expr>& n) {
  auto depth = n->GetParenthesisDepth();
  for (size_t i = 0; i < depth; ++i) {
    PutPunctor(PuncRPar);
  }
}

void CodeWriter::Visit(FlowPtr<parse::NamedTy> n) {
  PrintLeading(n);

  PutIdentifier(n->GetName());
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::InferTy> n) {
  PrintLeading(n);

  PutOperator(OpTernary);
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::TemplateType> n) {
  PrintLeading(n);

  n->GetTemplate()->Accept(*this);
  PutOperator(OpLT);
  for (auto it = n->GetArgs().begin(); it != n->GetArgs().end(); ++it) {
    if (it != n->GetArgs().begin()) {
      PutPunctor(PuncComa);
    }

    const auto [pname, pval] = *it;
    if (IsNamedParameter(pname)) {
      PutIdentifier(pname);
      PutPunctor(PuncColn);
    }

    pval->Accept(*this);
  }
  PutOperator(OpGT);

  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::U1> n) {
  PrintLeading(n);

  PutIdentifier("u1");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::U8> n) {
  PrintLeading(n);

  PutIdentifier("u8");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::U16> n) {
  PrintLeading(n);

  PutIdentifier("u16");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::U32> n) {
  PrintLeading(n);

  PutIdentifier("u32");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::U64> n) {
  PrintLeading(n);

  PutIdentifier("u64");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::U128> n) {
  PrintLeading(n);

  PutIdentifier("u128");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::I8> n) {
  PrintLeading(n);

  PutIdentifier("i8");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::I16> n) {
  PrintLeading(n);

  PutIdentifier("i16");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::I32> n) {
  PrintLeading(n);

  PutIdentifier("i32");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::I64> n) {
  PrintLeading(n);

  PutIdentifier("i64");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::I128> n) {
  PrintLeading(n);

  PutIdentifier("i128");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::F16> n) {
  PrintLeading(n);

  PutIdentifier("f16");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::F32> n) {
  PrintLeading(n);

  PutIdentifier("f32");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::F64> n) {
  PrintLeading(n);

  PutIdentifier("f64");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::F128> n) {
  PrintLeading(n);

  PutIdentifier("f128");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::VoidTy> n) {
  PrintLeading(n);

  PutIdentifier("void");
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::PtrTy> n) {
  PrintLeading(n);

  PutOperator(OpTimes);
  n->GetItem()->Accept(*this);
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::OpaqueTy> n) {
  PrintLeading(n);

  PutKeyword(Opaque);
  PutPunctor(PuncLPar);
  PutIdentifier(n->GetName());
  PutPunctor(PuncRPar);
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::TupleTy> n) {
  PrintLeading(n);

  PutPunctor(PuncLPar);
  for (auto it = n->GetItems().begin(); it != n->GetItems().end(); ++it) {
    if (it != n->GetItems().begin()) {
      PutPunctor(PuncComa);
    }

    (*it)->Accept(*this);
  }
  PutPunctor(PuncRPar);
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::ArrayTy> n) {
  PrintLeading(n);

  PutPunctor(PuncLBrk);
  n->GetItem()->Accept(*this);
  PutPunctor(PuncColn);
  n->GetSize()->Accept(*this);
  PutPunctor(PuncRBrk);
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::RefTy> n) {
  PrintLeading(n);

  PutOperator(OpBitAnd);
  n->GetItem()->Accept(*this);
  PutTypeStuff(n);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::FuncTy> n) {
  PrintLeading(n);

  /// TODO: Implement code writer
  qcore_implement();
  (void)n;

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Unary> n) {
  PrintLeading(n);

  PutOperator(n->GetOp());
  n->GetRHS()->Accept(*this);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Binary> n) {
  PrintLeading(n);

  n->GetLHS()->Accept(*this);
  PutOperator(n->GetOp());
  n->GetRHS()->Accept(*this);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::PostUnary> n) {
  PrintLeading(n);

  n->GetLHS()->Accept(*this);
  PutOperator(n->GetOp());

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Ternary> n) {
  PrintLeading(n);

  n->GetCond()->Accept(*this);
  PutOperator(OpTernary);
  n->GetLHS()->Accept(*this);
  PutPunctor(PuncColn);
  n->GetRHS()->Accept(*this);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Integer> n) {
  PrintLeading(n);

  PutInteger(n->GetValue());

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Float> n) {
  PrintLeading(n);

  PutFloat(n->GetValue());

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Boolean> n) {
  PrintLeading(n);

  PutKeyword(n->GetValue() ? True : False);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::String> n) {
  PrintLeading(n);

  PutString(n->GetValue());

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Character> n) {
  PrintLeading(n);

  PutCharacter(n->GetValue());

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Null> n) {
  PrintLeading(n);

  PutKeyword(lex::Null);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Undefined> n) {
  PrintLeading(n);

  PutKeyword(Undef);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Call> n) {
  PrintLeading(n);

  n->GetFunc()->Accept(*this);
  PutPunctor(PuncLPar);
  for (auto it = n->GetArgs().begin(); it != n->GetArgs().end(); ++it) {
    if (it != n->GetArgs().begin()) {
      PutPunctor(PuncComa);
    }

    if (IsNamedParameter(it->first)) {
      PutIdentifier(it->first);
      PutPunctor(PuncColn);
    }

    it->second->Accept(*this);
  }
  PutPunctor(PuncRPar);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::TemplateCall> n) {
  PrintLeading(n);

  n->GetFunc()->Accept(*this);
  PutOperator(OpLT);
  for (auto it = n->GetTemplateArgs().begin(); it != n->GetTemplateArgs().end(); ++it) {
    if (it != n->GetTemplateArgs().begin()) {
      PutPunctor(PuncComa);
    }

    if (IsNamedParameter(it->first)) {
      PutIdentifier(it->first);
      PutPunctor(PuncColn);
    }

    it->second->Accept(*this);
  }
  PutOperator(OpGT);
  PutPunctor(PuncLPar);
  for (auto it = n->GetArgs().begin(); it != n->GetArgs().end(); ++it) {
    if (it != n->GetArgs().begin()) {
      PutPunctor(PuncComa);
    }

    if (IsNamedParameter(it->first)) {
      PutIdentifier(it->first);
      PutPunctor(PuncColn);
    }

    it->second->Accept(*this);
  }
  PutPunctor(PuncRPar);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::List> n) {
  PrintLeading(n);

  PutPunctor(PuncLBrk);
  for (auto it = n->GetItems().begin(); it != n->GetItems().end(); ++it) {
    if (it != n->GetItems().begin()) {
      PutPunctor(PuncComa);
    }

    (*it)->Accept(*this);
  }
  PutPunctor(PuncRBrk);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Assoc> n) {
  PrintLeading(n);

  PutPunctor(PuncLCur);
  n->GetKey()->Accept(*this);
  PutPunctor(PuncColn);
  n->GetValue()->Accept(*this);
  PutPunctor(PuncRCur);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Index> n) {
  PrintLeading(n);

  n->GetBase()->Accept(*this);
  PutPunctor(PuncLBrk);
  n->GetIndex()->Accept(*this);
  PutPunctor(PuncRBrk);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Slice> n) {
  PrintLeading(n);

  n->GetBase()->Accept(*this);
  PutPunctor(PuncLBrk);
  n->GetStart()->Accept(*this);
  PutPunctor(PuncColn);
  n->GetEnd()->Accept(*this);
  PutPunctor(PuncRBrk);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::FString> n) {
  PrintLeading(n);

  /// TODO: Implement code writer
  qcore_implement();
  (void)n;

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Identifier> n) {
  PrintLeading(n);

  PutIdentifier(n->GetName());

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Block> n) {
  PrintLeading(n);

  bool use_braces = m_did_root;

  if (!m_did_root) {
    m_did_root = true;
  }

  if (use_braces) [[likely]] {
    PutPunctor(PuncLCur);
  }

  for (const auto& stmt : n->GetStatements()) {
    stmt->Accept(*this);
    PutPunctor(PuncSemi);
  }

  if (use_braces) [[likely]] {
    PutPunctor(PuncRCur);
  }

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Variable> n) {
  PrintLeading(n);

  switch (n->GetVariableKind()) {
    case parse::VariableType::Var:
      PutKeyword(lex::Var);
      break;
    case parse::VariableType::Let:
      PutKeyword(lex::Let);
      break;
    case parse::VariableType::Const:
      PutKeyword(lex::Const);
      break;
  }

  if (!n->GetAttributes().empty()) {
    PutPunctor(PuncLBrk);
    for (auto it = n->GetAttributes().begin(); it != n->GetAttributes().end(); ++it) {
      if (it != n->GetAttributes().begin()) {
        PutPunctor(PuncComa);
      }

      it->Accept(*this);
    }
    PutPunctor(PuncRBrk);
  }

  PutIdentifier(n->GetName());
  if (!n->GetType()->Is(parse::QAST_INFER)) {
    PutPunctor(PuncColn);
    n->GetType()->Accept(*this);
  }

  if (n->GetInitializer()) {
    PutOperator(OpSet);
    n->GetInitializer().value()->Accept(*this);
  }

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Assembly> n) {
  PrintLeading(n);

  /// TODO: Implement code writer
  qcore_implement();
  (void)n;

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::If> n) {
  PrintLeading(n);

  PutKeyword(lex::If);
  n->GetCond()->Accept(*this);
  n->GetThen()->Accept(*this);
  if (n->GetElse()) {
    PutKeyword(lex::Else);
    n->GetElse().value()->Accept(*this);
  }

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::While> n) {
  PrintLeading(n);

  PutKeyword(lex::While);
  n->GetCond()->Accept(*this);
  n->GetBody()->Accept(*this);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::For> n) {
  PrintLeading(n);

  PutKeyword(lex::For);

  if (n->GetInit()) {
    n->GetInit().value()->Accept(*this);
  } else {
    PutPunctor(PuncSemi);
  }

  if (n->GetCond()) {
    n->GetCond().value()->Accept(*this);
  } else {
    PutPunctor(PuncSemi);
  }

  if (n->GetStep()) {
    n->GetStep().value()->Accept(*this);
  }

  n->GetBody()->Accept(*this);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Foreach> n) {
  PrintLeading(n);

  /// TODO: Implement code writer
  qcore_implement();
  (void)n;

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Break> n) {
  PrintLeading(n);

  PutKeyword(lex::Break);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Continue> n) {
  PrintLeading(n);

  PutKeyword(lex::Continue);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Return> n) {
  PrintLeading(n);

  PutKeyword(lex::Return);
  if (n->GetValue()) {
    n->GetValue().value()->Accept(*this);
  }

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::ReturnIf> n) {
  PrintLeading(n);

  PutKeyword(lex::Retif);
  n->GetCond()->Accept(*this);
  if (n->GetValue()) {
    PutPunctor(PuncComa);
    n->GetValue().value()->Accept(*this);
  }

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Case> n) {
  PrintLeading(n);

  n->GetCond()->Accept(*this);
  PutOperator(OpArrow);
  n->GetBody()->Accept(*this);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Switch> n) {
  PrintLeading(n);

  PutKeyword(lex::Switch);
  n->GetCond()->Accept(*this);
  PutPunctor(PuncLCur);
  for (const auto& c : n->GetCases()) {
    c->Accept(*this);
  }
  if (n->GetDefault()) {
    PutIdentifier("_");
    PutOperator(OpArrow);
    n->GetDefault().value()->Accept(*this);
  }
  PutPunctor(PuncRCur);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Typedef> n) {
  PrintLeading(n);

  PutKeyword(lex::Type);
  PutIdentifier(n->GetName());
  PutOperator(OpSet);
  n->GetType()->Accept(*this);

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Function> n) {
  PrintLeading(n);

  /// TODO: Implement code writer
  qcore_implement();
  (void)n;

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Struct> n) {
  PrintLeading(n);

  /// TODO: Implement code writer
  qcore_implement();
  (void)n;

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Enum> n) {
  PrintLeading(n);

  /// TODO: Implement code writer
  qcore_implement();
  (void)n;

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Scope> n) {
  PrintLeading(n);

  /// TODO: Implement code writer
  qcore_implement();
  (void)n;

  PrintTrailing(n);
}

void CodeWriter::Visit(FlowPtr<parse::Export> n) {
  PrintLeading(n);

  /// TODO: Implement code writer
  qcore_implement();
  (void)n;

  PrintTrailing(n);
}

CodeWriter::CodeWriter(std::ostream& os) : m_os(os), m_ldata(TokenData::GetDefault(EofF)) {}

std::unique_ptr<parse::ICodeWriter> parse::CodeWriterFactory::Create(std::ostream& os, SyntaxVersion ver) {
  switch (ver) {
    case NITRATE_1_0: {
      return std::make_unique<CodeWriter>(os);
    }

    default: {
      return nullptr;
    }
  }
}
