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
////////////////////////////////////////////////////////////////////////////////

#include <nitrate-lexer/Lexer.hh>
#include <nlohmann/json.hpp>

using namespace nitrate::compiler::lexer;

BOOST_SYMBOL_EXPORT auto Token::dump(std::ostream& os, bool include_source_range) const -> std::ostream& {
  nlohmann::ordered_json j;

  switch (type()) {
    case TokenType::Identifier: {
      j["type"] = "name";
      j["value"] = std::get<Identifier>(m_value).name();
      j["is_raw"] = std::get<Identifier>(m_value).is_atypical();
      break;
    }

    case TokenType::Keyword: {
      j["type"] = "keyword";
      j["value"] = keyword_to_string(std::get<Keyword>(m_value));
      break;
    }

    case TokenType::Operator: {
      j["type"] = "operator";
      j["value"] = operator_to_string(std::get<Operator>(m_value));
      break;
    }

    case TokenType::Punctor: {
      j["type"] = "punctor";
      j["value"] = punctor_to_string(std::get<Punctor>(m_value));
      break;
    }

    case TokenType::StringLiteral: {
      j["type"] = "string";
      j["value"] = std::get<StringLiteral>(m_value).value();

      switch (std::get<StringLiteral>(m_value).type()) {
        case StringType::SingleQuote: {
          j["string_type"] = "single_quoted";
          break;
        }

        case StringType::DoubleQuote: {
          j["string_type"] = "double_quoted";
          break;
        }

        case StringType::RawString: {
          j["string_type"] = "raw";
          break;
        }
      }
      break;
    }

    case TokenType::NumberLiteral: {
      j["type"] = "number";
      j["value"] = std::get<NumberLiteral>(m_value).value();

      switch (std::get<NumberLiteral>(m_value).type()) {
        case NumberLiteralType::UIntBin: {
          j["number_type"] = "uint_bin";
          break;
        }

        case NumberLiteralType::UIntOct: {
          j["number_type"] = "uint_oct";
          break;
        }

        case NumberLiteralType::UIntDec: {
          j["number_type"] = "uint_dec";
          break;
        }

        case NumberLiteralType::UIntHex: {
          j["number_type"] = "uint_hex";
          break;
        }

        case NumberLiteralType::UFloatIEEE754: {
          j["number_type"] = "ufloat_ieee754";
          break;
        }
      }

      break;
    }

    case TokenType::Comment: {
      j["type"] = "comment";
      j["value"] = std::get<Comment>(m_value).value();

      switch (std::get<Comment>(m_value).type()) {
        case CommentType::SingleLine: {
          j["comment_type"] = "line";
          break;
        }

        case CommentType::MultiLine: {
          j["comment_type"] = "block";
          break;
        }
      }
      break;
    }
  }

  if (include_source_range) {
    j["source_range"] = {{"file", source_range().file()},
                         {"begin",
                          {{"line", source_range().begin().line()},
                           {"column", source_range().begin().column()},
                           {"offset", source_range().begin().offset()}}},
                         {"end",
                          {{"line", source_range().end().line()},
                           {"column", source_range().end().column()},
                           {"offset", source_range().end().offset()}}}};
  }

  os << j.dump();

  return os;
}
