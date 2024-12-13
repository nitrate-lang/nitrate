#pragma once

#include <nitrate-parser/AST.hh>

namespace lsp::fmt {
  class ICodeFormatter {
  public:
    virtual ~ICodeFormatter() = default;
    virtual bool format(npar_node_t* root) = 0;
  };

}  // namespace lsp::fmt
