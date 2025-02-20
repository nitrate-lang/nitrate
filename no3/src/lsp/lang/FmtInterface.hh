#pragma once

#include <nitrate-core/FlowPtr.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTFwd.hh>

namespace no3::lsp::fmt {
  class ICodeFormatter {
  public:
    virtual ~ICodeFormatter() = default;
    virtual auto Format(ncc::FlowPtr<ncc::parse::Base> root) -> bool = 0;
  };

}  // namespace no3::lsp::fmt
