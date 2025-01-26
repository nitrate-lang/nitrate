#pragma once

#include <lsp/lang/FmtInterface.hh>
#include <lsp/lang/format/Formatter.hh>
#include <nitrate-parser/AST.hh>

namespace no3::lsp::fmt {
  enum class Styleguide { Cambrian };

  class FormatterFactory final {
  public:
    static auto Create(Styleguide style,
                       std::ostream& out) -> std::unique_ptr<ICodeFormatter> {
      switch (style) {
        case Styleguide::Cambrian: {
          return std::make_unique<CambrianFormatter>(out);
        }
      }
    }
  };
}  // namespace no3::lsp::fmt
