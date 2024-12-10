#pragma once

#include <nitrate-parser/Node.h>

#include <ostream>

namespace lsp::fmt {
  class ICodeFormatter {
  public:
    virtual ~ICodeFormatter() = default;
    virtual bool format(npar_node_t* root, std::ostream& out) = 0;
  };

}  // namespace lsp::fmt
