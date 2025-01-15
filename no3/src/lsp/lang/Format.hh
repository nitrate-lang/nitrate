#pragma once

#include <lsp/lang/CambrianStyleFormatter.hh>
#include <lsp/lang/FmtInterface.hh>
#include <nitrate-parser/AST.hh>

namespace lsp::fmt {
  enum class Styleguide { Cambrian };

  class FormatterFactory final {
  public:
    static std::unique_ptr<ICodeFormatter> Create(Styleguide style,
                                                  std::ostream& out) {
      switch (style) {
        case Styleguide::Cambrian: {
          return std::make_unique<CambrianFormatter>(out);
        }
      }
    }
  };
}  // namespace lsp::fmt
