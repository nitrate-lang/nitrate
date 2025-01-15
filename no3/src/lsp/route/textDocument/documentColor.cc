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
  size_t line = 0;
  size_t character = 0;
};

struct Range {
  Position start;
  Position end;
};

struct ColorInformation {
  Range range;

  double red = 0, green = 0, blue = 0, alpha = 0;
};

struct RGBA {
  float r, g, b, a;
};

enum class ColorMode {
  RGBA,
  HSLA,
};

static RGBA hslaToRgba(float h, float s, float l, float a) {
  // Clamp h to [0, 360), s and l to [0, 100], and a to [0, 1]
  h = fmod(h, 360.0f);
  if (h < 0) h += 360.0f;
  s = std::clamp(s, 0.0f, 100.0f);
  l = std::clamp(l, 0.0f, 100.0f);
  a = std::clamp(a, 0.0f, 1.0f);

  h /= 360.0f;
  s /= 100.0f;
  l /= 100.0f;

  float c = (1 - std::abs(2 * l - 1)) * s;
  float x = c * (1 - std::abs(fmod(h * 6.0f, 2.0f) - 1));
  float m = l - c / 2;

  float r, g, b;

  if (h >= 0 && h < 1.0f / 6.0f) {
    r = c;
    g = x;
    b = 0;
  } else if (h >= 1.0f / 6.0f && h < 2.0f / 6.0f) {
    r = x;
    g = c;
    b = 0;
  } else if (h >= 2.0f / 6.0f && h < 3.0f / 6.0f) {
    r = 0;
    g = c;
    b = x;
  } else if (h >= 3.0f / 6.0f && h < 4.0f / 6.0f) {
    r = 0;
    g = x;
    b = c;
  } else if (h >= 4.0f / 6.0f && h < 5.0f / 6.0f) {
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
static std::optional<ColorInformation> parse_color_function(
    ncc::FlowPtr<Call> N, ColorMode mode, IScanner& L) {
  static_assert(Argc == 3 || Argc == 4,
                "Invalid number of arguments. Indexs will be out-of-range.");

  let args = N->get_args();

  if (args.size() != Argc) {
    return std::nullopt;
  }

  float values[Argc];
  for (size_t i = 0; i < Argc; i++) {
    let arg_expr = args[i].second;

    std::string_view value;

    if (arg_expr->is(QAST_FLOAT)) {
      value = arg_expr->as<ConstFloat>()->get_value();
    } else if (arg_expr->is(QAST_INT)) {
      value = arg_expr->as<ConstInt>()->get_value();
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
  (void)hslaToRgba;

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
  //       .alpha = Argc == 4 ? values[3] : 1.0f,
  //   };
  // } else if (mode == ColorMode::HSLA) {
  //   let rgba = hslaToRgba(values[0], values[1], values[2],
  //                         Argc == 4 ? values[3] : 1.0f);

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

void do_documentColor(const lsp::RequestMessage& req,
                      lsp::ResponseMessage& resp) {
  if (!req.params().HasMember("textDocument")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing textDocument");
    return;
  }

  if (!req.params()["textDocument"].IsObject()) {
    resp.error(lsp::ErrorCodes::InvalidParams, "textDocument is not an object");
    return;
  }

  if (!req.params()["textDocument"].HasMember("uri")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing textDocument.uri");
    return;
  }

  if (!req.params()["textDocument"]["uri"].IsString()) {
    resp.error(lsp::ErrorCodes::InvalidParams,
               "textDocument.uri is not a string");
    return;
  }

  let uri = req.params()["textDocument"]["uri"].GetString();

  LOG(INFO) << "Requested document color box";

  let file_opt = SyncFS::the().open(uri);
  if (!file_opt.has_value()) {
    resp.error(lsp::ErrorCodes::InternalError, "Failed to open file");
    return;
  }
  let file = file_opt.value();

  auto env = std::make_shared<ncc::Environment>();
  auto ss = std::stringstream(*file->content());
  auto L = Tokenizer(ss, env);
  let parser = ncc::parse::Parser::Create(L, env);
  let ast = parser->parse();

  if (L.HasError() || !ast.check()) {
    return;
  }

  std::vector<ColorInformation> colors;

  for_each<Call>(ast.get(), [&](let N) {
    if (N->get_func()->is(QAST_IDENT)) {
      let name = N->get_func()->template as<Ident>()->GetName();
      let args = N->get_args();

      std::optional<ColorInformation> element;
      if (name == "rgba" && args.size() == 4) {
        element = parse_color_function<4>(N, ColorMode::RGBA, L);
      } else if (name == "hsla" && args.size() == 4) {
        element = parse_color_function<4>(N, ColorMode::HSLA, L);
      } else if (name == "rgb" && args.size() == 3) {
        element = parse_color_function<3>(N, ColorMode::RGBA, L);
      } else if (name == "hsl" && args.size() == 3) {
        element = parse_color_function<3>(N, ColorMode::HSLA, L);
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

    start.AddMember("line", color.range.start.line, resp->GetAllocator());
    start.AddMember("character", color.range.start.character,
                    resp->GetAllocator());
    end.AddMember("line", color.range.end.line, resp->GetAllocator());
    end.AddMember("character", color.range.end.character, resp->GetAllocator());
    range.AddMember("start", start, resp->GetAllocator());
    range.AddMember("end", end, resp->GetAllocator());

    color_info.AddMember("range", range, resp->GetAllocator());
    color_info.AddMember("color", Value(kObjectType), resp->GetAllocator());
    color_info["color"].AddMember("red", color.red, resp->GetAllocator());
    color_info["color"].AddMember("green", color.green, resp->GetAllocator());
    color_info["color"].AddMember("blue", color.blue, resp->GetAllocator());
    color_info["color"].AddMember("alpha", color.alpha, resp->GetAllocator());

    resp->PushBack(color_info, resp->GetAllocator());
  }

  return;
}
