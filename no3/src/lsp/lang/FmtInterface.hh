#pragma once

#include <nitrate-parser/AST.hh>

namespace lsp::fmt {
  class ICodeFormatter {
  public:
    virtual ~ICodeFormatter() = default;
    virtual auto Format(ncc::FlowPtr<ncc::parse::Base> root) -> bool = 0;
  };

}  // namespace lsp::fmt
