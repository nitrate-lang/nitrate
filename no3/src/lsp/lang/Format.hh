#pragma once

#include <nitrate-parser/Node.h>

#include <lsp/lang/CambrianStyleFormatter.hh>
#include <lsp/lang/FmtInterface.hh>

namespace lsp::fmt {
  enum class Styleguide { Cambrian };

  class FormatterFactory final {
  public:
    static std::unique_ptr<ICodeFormatter> create(Styleguide style,
                                                  std::ostream& out) {
      switch (style) {
        case Styleguide::Cambrian: {
          return std::make_unique<CambrianFormatter>(out);
        }
      }
    }
  };
}  // namespace lsp::fmt
