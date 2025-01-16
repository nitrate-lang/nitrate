#include <rapidjson/document.h>

#include <charconv>
#include <lsp/core/SyncFS.hh>
#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/Context.hh>
#include <sstream>
#include <string>

using namespace rapidjson;
using namespace ncc::lex;
using namespace ncc::parse;

struct Position {
  size_t m_line = 0;
  size_t m_character = 0;
};

struct Range {
  Position m_start;
  Position m_end;
};

struct ColorInformation {
  Range m_range;

  double m_red = 0, m_green = 0, m_blue = 0, m_alpha = 0;
};

struct RGBA {
  float m_r, m_g, m_b, m_a;
};

enum class ColorMode {
  RGBA,
  HSLA,
};

static auto HslaToRgba(float h, float s, float l, float a) -> RGBA {
  // Clamp h to [0, 360), s and l to [0, 100], and a to [0, 1]
  h = fmod(h, 360.0);
  if (h < 0) {
    h += 360.0F;
  }
  s = std::clamp(s, 0.0F, 100.0F);
  l = std::clamp(l, 0.0F, 100.0F);
  a = std::clamp(a, 0.0F, 1.0F);

  h /= 360.0F;
  s /= 100.0F;
  l /= 100.0F;

  float c = (1 - std::abs(2 * l - 1)) * s;
  float x = c * (1 - std::abs(fmod(h * 6.0F, 2.0F) - 1));
  float m = l - c / 2;

  float r;
  float g;
  float b;

  if (h >= 0 && h < 1.0F / 6.0F) {
    r = c;
    g = x;
    b = 0;
  } else if (h >= 1.0F / 6.0F && h < 2.0F / 6.0F) {
    r = x;
    g = c;
    b = 0;
  } else if (h >= 2.0F / 6.0F && h < 3.0F / 6.0F) {
    r = 0;
    g = c;
    b = x;
  } else if (h >= 3.0F / 6.0F && h < 4.0F / 6.0F) {
    r = 0;
    g = x;
    b = c;
  } else if (h >= 4.0F / 6.0F && h < 5.0F / 6.0F) {
    r = x;
    g = 0;
    b = c;
  } else {
    r = c;
    g = 0;
    b = x;
  }

  return RGBA{(r + m) * 255, (g + m) * 255, (b + m) * 255, a * 255};
}

template <size_t Argc>
static auto ParseColorFunction(ncc::FlowPtr<Call> n, ColorMode,
                               IScanner&) -> std::optional<ColorInformation> {
  static_assert(Argc == 3 || Argc == 4,
                "Invalid number of arguments. Indexs will be out-of-range.");

  let args = n->GetArgs();

  if (args.size() != Argc) {
    return std::nullopt;
  }

  std::array<float, Argc> values;
  for (size_t i = 0; i < Argc; i++) {
    let arg_expr = args[i].second;

    std::string_view value;

    if (arg_expr->is(QAST_FLOAT)) {
      value = arg_expr->as<ConstFloat>()->GetValue();
    } else if (arg_expr->is(QAST_INT)) {
      value = arg_expr->as<ConstInt>()->GetValue();
    } else {
      return std::nullopt;
    }

    float x;
    if (std::from_chars(value.data(), value.data() + value.size(), x).ec ==
        std::errc()) {
      values[i] = x;
    } else {
      return std::nullopt;
    }
  }

  /// TODO: Fix source location tracking
  (void)HslaToRgba;

  return std::nullopt;

  // let start_offset = N->get_();

  // let start_line = L.GetRow(std::get<0>(start_offset)),
  //     start_column = L.GetCol(std::get<0>(start_offset));

  // let end_line = L.GetRow(std::get<1>(start_offset)),
  //     end_column = L.GetCol(std::get<1>(start_offset));

  // if (mode == ColorMode::RGBA) {
  //   return ColorInformation{
  //       .range = {.start = {.line = start_line, .character = start_column},
  //                 .end = {.line = end_line, .character = end_column}},
  //       .red = values[0],
  //       .green = values[1],
  //       .blue = values[2],
  //       .alpha = Argc == 4 ? values[3] : 1.0F,
  //   };
  // } else if (mode == ColorMode::HSLA) {
  //   let rgba = hslaToRgba(values[0], values[1], values[2],
  //                         Argc == 4 ? values[3] : 1.0F);

  //   return ColorInformation{
  //       .range = {.start = {.line = start_line, .character = start_column},
  //                 .end = {.line = end_line, .character = end_column}},
  //       .red = rgba.r,
  //       .green = rgba.g,
  //       .blue = rgba.b,
  //       .alpha = rgba.a,
  //   };
  // } else {
  //   return std::nullopt;
  // }
}

void DoDocumentColor(const lsp::RequestMessage& req,
                     lsp::ResponseMessage& resp) {
  if (!req.Params().HasMember("textDocument")) {
    resp.Error(lsp::ErrorCodes::InvalidParams, "Missing textDocument");
    return;
  }

  if (!req.Params()["textDocument"].IsObject()) {
    resp.Error(lsp::ErrorCodes::InvalidParams, "textDocument is not an object");
    return;
  }

  if (!req.Params()["textDocument"].HasMember("uri")) {
    resp.Error(lsp::ErrorCodes::InvalidParams, "Missing textDocument.uri");
    return;
  }

  if (!req.Params()["textDocument"]["uri"].IsString()) {
    resp.Error(lsp::ErrorCodes::InvalidParams,
               "textDocument.uri is not a string");
    return;
  }

  let uri = req.Params()["textDocument"]["uri"].GetString();

  LOG(INFO) << "Requested document color box";

  let file_opt = SyncFS::The().Open(uri);
  if (!file_opt.has_value()) {
    resp.Error(lsp::ErrorCodes::InternalError, "Failed to open file");
    return;
  }
  let file = file_opt.value();

  auto env = std::make_shared<ncc::Environment>();
  auto ss = std::stringstream(*file->Content());
  auto l = Tokenizer(ss, env);
  let parser = ncc::parse::Parser::Create(l, env);
  let ast = parser->Parse();

  if (l.HasError() || !ast.Check()) {
    return;
  }

  std::vector<ColorInformation> colors;

  for_each<Call>(ast.Get(), [&](let n) {
    if (n->GetFunc()->is(QAST_IDENT)) {
      let name = n->GetFunc()->template as<Ident>()->GetName();
      let args = n->GetArgs();

      std::optional<ColorInformation> element;
      if (name == "rgba" && args.size() == 4) {
        element = ParseColorFunction<4>(n, ColorMode::RGBA, l);
      } else if (name == "hsla" && args.size() == 4) {
        element = ParseColorFunction<4>(n, ColorMode::HSLA, l);
      } else if (name == "rgb" && args.size() == 3) {
        element = ParseColorFunction<3>(n, ColorMode::RGBA, l);
      } else if (name == "hsl" && args.size() == 3) {
        element = ParseColorFunction<3>(n, ColorMode::HSLA, l);
      }

      if (element.has_value()) {
        colors.push_back(element.value());
      } else {
        LOG(INFO) << "Failed to parse color function";
      }
    }
  });

  resp->SetArray();
  resp->Reserve(colors.size(), resp->GetAllocator());

  for (const auto& color : colors) {
    Value color_info(kObjectType);
    Value range(kObjectType);
    Value start(kObjectType);
    Value end(kObjectType);

    start.AddMember("line", color.m_range.m_start.m_line, resp->GetAllocator());
    start.AddMember("character", color.m_range.m_start.m_character,
                    resp->GetAllocator());
    end.AddMember("line", color.m_range.m_end.m_line, resp->GetAllocator());
    end.AddMember("character", color.m_range.m_end.m_character,
                  resp->GetAllocator());
    range.AddMember("start", start, resp->GetAllocator());
    range.AddMember("end", end, resp->GetAllocator());

    color_info.AddMember("range", range, resp->GetAllocator());
    color_info.AddMember("color", Value(kObjectType), resp->GetAllocator());
    color_info["color"].AddMember("red", color.m_red, resp->GetAllocator());
    color_info["color"].AddMember("green", color.m_green, resp->GetAllocator());
    color_info["color"].AddMember("blue", color.m_blue, resp->GetAllocator());
    color_info["color"].AddMember("alpha", color.m_alpha, resp->GetAllocator());

    resp->PushBack(color_info, resp->GetAllocator());
  }
}
