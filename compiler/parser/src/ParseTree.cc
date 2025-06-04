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

#include <boost/config.hpp>
#include <nitrate-parser/ParseTree.hh>
#include <nitrate-parser/Parser.hh>

using namespace nitrate::compiler::parser;
using namespace nitrate::compiler::lexer;

std::unordered_map<uint64_t, boost::flyweight<FileSourceRange>> SOURCE_RANGES_GLOBAL;
uint64_t SOURCE_RANGES_ID_CTR_GLOBAL;
std::mutex SOURCE_RANGES_LOCK_GLOBAL;

BOOST_SYMBOL_EXPORT Expr::SourceLocationTag::SourceLocationTag(boost::flyweight<lexer::FileSourceRange> source_range) {
  std::lock_guard lock(SOURCE_RANGES_LOCK_GLOBAL);
  m_id = ++SOURCE_RANGES_ID_CTR_GLOBAL;
  SOURCE_RANGES_GLOBAL.emplace(static_cast<uint64_t>(m_id), std::move(source_range));
}

BOOST_SYMBOL_EXPORT Expr::SourceLocationTag::~SourceLocationTag() {
  if (m_id != 0) {
    std::lock_guard lock(SOURCE_RANGES_LOCK_GLOBAL);
    SOURCE_RANGES_GLOBAL.erase(m_id);
  }
}

BOOST_SYMBOL_EXPORT [[nodiscard]] auto Expr::SourceLocationTag::get() const -> const lexer::FileSourceRange& {
  std::lock_guard lock(SOURCE_RANGES_LOCK_GLOBAL);
  return SOURCE_RANGES_GLOBAL.at(m_id).get();
}

BOOST_SYMBOL_EXPORT auto Expr::operator==(const Expr& o) const -> bool {
  // FIXME: Implement a proper equality check
  (void)o;
  return true;
}

BOOST_SYMBOL_EXPORT auto Expr::hash() const -> size_t {
  // TODO: Implement a proper hash function
  return 0;
}

void test() {  //
  boost::flyweight<TupleTy> x;
}
