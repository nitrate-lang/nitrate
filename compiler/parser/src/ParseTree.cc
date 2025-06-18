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

#include <nitrate-parser/ParseTree.hh>
#include <nitrate-parser/Parser.hh>

using namespace nitrate::compiler::parser;
using namespace nitrate::compiler::lexer;

static std::unordered_map<uint64_t, boost::flyweight<FileSourceRange>> SOURCE_RANGES_GLOBAL;
static uint64_t SOURCE_RANGES_ID_CTR_GLOBAL;
static std::mutex SOURCE_RANGES_LOCK_GLOBAL;

BOOST_SYMBOL_EXPORT ExprBase::LocationTag::LocationTag(lexer::FileSourceRange source_range) {
  std::lock_guard lock(SOURCE_RANGES_LOCK_GLOBAL);
  m_id = ++SOURCE_RANGES_ID_CTR_GLOBAL;
  SOURCE_RANGES_GLOBAL.emplace(static_cast<uint64_t>(m_id), std::move(source_range));
}

BOOST_SYMBOL_EXPORT ExprBase::LocationTag::~LocationTag() {
  if (m_id != 0) {
    std::lock_guard lock(SOURCE_RANGES_LOCK_GLOBAL);
    SOURCE_RANGES_GLOBAL.erase(m_id);
  }
}

BOOST_SYMBOL_EXPORT auto ExprBase::LocationTag::get() const -> const lexer::FileSourceRange& {
  std::lock_guard lock(SOURCE_RANGES_LOCK_GLOBAL);
  return SOURCE_RANGES_GLOBAL.at(m_id).get();
}

// BOOST_SYMBOL_EXPORT auto Expr::operator==(const Expr& o) const -> bool {
//   // FIXME: Optimize this comparison

//   std::stringstream a;
//   std::stringstream b;

//   dump(a);
//   o.dump(b);

//   return a.str() == b.str();
// }

// BOOST_SYMBOL_EXPORT auto Expr::hash() const -> size_t {
//   // FIXME: Optimize this hashing

//   std::stringstream ss;
//   dump(ss);

//   return std::hash<std::string>{}(ss.str());
// }
