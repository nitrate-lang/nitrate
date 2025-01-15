#pragma once

#include <nitrate-parser/AST.hh>

namespace lsp::fmt {
  class ICodeFormatter {
  public:
    virtual ~ICodeFormatter() = default;
    virtual bool Format(ncc::FlowPtr<ncc::parse::Base> root) = 0;
  };

}  // namespace lsp::fmt
