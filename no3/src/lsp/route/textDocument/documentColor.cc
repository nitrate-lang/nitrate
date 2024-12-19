#include <rapidjson/document.h>

#include <lsp/core/SyncFS.hh>
#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>
#include <nitrate-lexer/Base.hh>
#include <nitrate-lexer/Lexer.hh>
#include <regex>
#include <sstream>
#include <string>

using namespace rapidjson;
using namespace ncc::lex;

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

  std::string uri = req.params()["textDocument"]["uri"].GetString();

  LOG(INFO) << "Requested document color box";

  auto file_opt = SyncFS::the().open(uri);
  if (!file_opt.has_value()) {
    resp.error(lsp::ErrorCodes::InternalError, "Failed to open file");
    return;
  }
  auto file = file_opt.value();

  std::stringstream ss(*file->content());

  auto env = std::make_shared<ncc::core::Environment>();
  auto L = Tokenizer(SourceFileFromSeekableStream(ss, uri), env);
  NCCToken tok;
  std::vector<ColorInformation> colors;

  while ((tok = (L.Next())).ty != qEofF) {
    if (tok.ty != qMacr) {
      continue;
    }

    uint32_t start_line = L.StartLine(tok), start_col = L.StartColumn(tok);
    uint32_t end_line = L.EndLine(tok), end_col = L.EndColumn(tok);

    if (start_line == QLEX_EOFF || start_col == QLEX_EOFF ||
        end_line == QLEX_EOFF || end_col == QLEX_EOFF) {
      LOG(WARNING) << "Failed to get source location";
      continue;
    }

    std::string_view value = tok.as_string();

    if (value.starts_with("rgba(") && value.ends_with(")")) {
      value.remove_prefix(5);
      value.remove_suffix(1);

      std::regex rgx(R"((\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+))");
      std::cmatch match;
      if (!std::regex_search(value.begin(), value.end(), match, rgx) ||
          match.size() != 5) {
        continue;
      }

      uint32_t r = 0, g = 0, b = 0, a = 0;
      if (std::sscanf(match[1].str().c_str(), "%u", &r) != 1 ||
          std::sscanf(match[2].str().c_str(), "%u", &g) != 1 ||
          std::sscanf(match[3].str().c_str(), "%u", &b) != 1 ||
          std::sscanf(match[4].str().c_str(), "%u", &a) != 1) {
        continue;
      }

      ColorInformation color;
      color.range.start.line = start_line - 1;
      color.range.start.character = start_col - 1;
      color.range.end.line = end_line - 1;
      color.range.end.character = end_col - 1;
      color.red = r >= 255 ? 1.0 : r / 255.0;
      color.green = g >= 255 ? 1.0 : g / 255.0;
      color.blue = b >= 255 ? 1.0 : b / 255.0;
      color.alpha = a >= 255 ? 1.0 : a / 255.0;

      colors.push_back(color);
    } else if (value.starts_with("rgb(")) {
      value.remove_prefix(4);
      value.remove_suffix(1);

      std::regex rgx(R"((\d+)\s*,\s*(\d+)\s*,\s*(\d+))");
      std::cmatch match;
      if (!std::regex_search(value.begin(), value.end(), match, rgx) ||
          match.size() != 4) {
        continue;
      }

      uint32_t r = 0, g = 0, b = 0;
      if (std::sscanf(match[1].str().c_str(), "%u", &r) != 1 ||
          std::sscanf(match[2].str().c_str(), "%u", &g) != 1 ||
          std::sscanf(match[3].str().c_str(), "%u", &b) != 1) {
        continue;
      }

      ColorInformation color;
      color.range.start.line = start_line - 1;
      color.range.start.character = start_col - 1;
      color.range.end.line = end_line - 1;
      color.range.end.character = end_col - 1;
      color.red = r >= 255 ? 1.0 : r / 255.0;
      color.green = g >= 255 ? 1.0 : g / 255.0;
      color.blue = b >= 255 ? 1.0 : b / 255.0;
      color.alpha = 1.0;

      colors.push_back(color);
    } else if (value.starts_with("hsla(")) {
      value.remove_prefix(5);
      value.remove_suffix(1);

      std::regex rgx(R"((\d+)\s*,\s*(\d+)%?\s*,\s*(\d+)%?\s*,\s*(\d+)%?)");
      std::cmatch match;
      if (!std::regex_search(value.begin(), value.end(), match, rgx) ||
          match.size() != 5) {
        continue;
      }

      uint32_t h = 0, s = 0, l = 0, a = 0;
      if (std::sscanf(match[1].str().c_str(), "%u", &h) != 1 ||
          std::sscanf(match[2].str().c_str(), "%u", &s) != 1 ||
          std::sscanf(match[3].str().c_str(), "%u", &l) != 1 ||
          std::sscanf(match[4].str().c_str(), "%u", &a) != 1) {
        continue;
      }

      RGBA rgba = hslaToRgba(h, s, l, a / 100.0);

      ColorInformation color;
      color.range.start.line = start_line - 1;
      color.range.start.character = start_col - 1;
      color.range.end.line = end_line - 1;
      color.range.end.character = end_col - 1;
      color.red = rgba.r / 255.0;
      color.green = rgba.g / 255.0;
      color.blue = rgba.b / 255.0;
      color.alpha = rgba.a / 255.0;

      colors.push_back(color);
    } else if (value.starts_with("hsl(")) {
      value.remove_prefix(4);
      value.remove_suffix(1);

      std::regex rgx(R"((\d+)\s*,\s*(\d+)%?\s*,\s*(\d+)%?)");
      std::cmatch match;
      if (!std::regex_search(value.begin(), value.end(), match, rgx) ||
          match.size() != 4) {
        continue;
      }

      uint32_t h = 0, s = 0, l = 0;
      if (std::sscanf(match[1].str().c_str(), "%u", &h) != 1 ||
          std::sscanf(match[2].str().c_str(), "%u", &s) != 1 ||
          std::sscanf(match[3].str().c_str(), "%u", &l) != 1) {
        continue;
      }

      RGBA rgba = hslaToRgba(h, s, l, 1.0);

      ColorInformation color;
      color.range.start.line = start_line - 1;
      color.range.start.character = start_col - 1;
      color.range.end.line = end_line - 1;
      color.range.end.character = end_col - 1;
      color.red = rgba.r / 255.0;
      color.green = rgba.g / 255.0;
      color.blue = rgba.b / 255.0;
      color.alpha = 1.0;

      colors.push_back(color);
    }
  }

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
