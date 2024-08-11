#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

#include "hpdf.h"

using namespace std;

void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
  char msg[256] = {0};
  snprintf(msg, sizeof(msg) - 1, "ERROR: error_no=%04X, detail_no=%d\n", (HPDF_UINT)error_no,
           (HPDF_UINT)detail_no);

  throw std::runtime_error(msg);
}

class FontManager {
public:
  FontManager(HPDF_Doc pdf) : pdf_(pdf) {}

  HPDF_Font LoadFont(const char* font_path) {
    const char* font_name = HPDF_LoadTTFontFromFile(pdf_, font_path, HPDF_TRUE);
    return HPDF_GetFont(pdf_, font_name, NULL);
  }

  HPDF_Font Get(const char* font_name) {
    std::unordered_map<std::string_view, std::string_view> path_table{
        {"Poppins-Black", "/usr/share/fonts/truetype/google-poppins/Poppins-Black.ttf"},
        {"Poppins-BlackItalic", "/usr/share/fonts/truetype/google-poppins/Poppins-BlackItalic.ttf"},
        {"Poppins-Bold", "/usr/share/fonts/truetype/google-poppins/Poppins-Bold.ttf"},
        {"Poppins-BoldItalic", "/usr/share/fonts/truetype/google-poppins/Poppins-BoldItalic.ttf"},
        {"Poppins-ExtraBold", "/usr/share/fonts/truetype/google-poppins/Poppins-ExtraBold.ttf"},
        {"Poppins-ExtraBoldItalic",
         "/usr/share/fonts/truetype/google-poppins/Poppins-ExtraBoldItalic.ttf"},
        {"Poppins-ExtraLight", "/usr/share/fonts/truetype/google-poppins/Poppins-ExtraLight.ttf"},
        {"Poppins-ExtraLightItalic",
         "/usr/share/fonts/truetype/google-poppins/Poppins-ExtraLightItalic.ttf"},
        {"Poppins-Italic", "/usr/share/fonts/truetype/google-poppins/Poppins-Italic.ttf"},
        {"Poppins-Light", "/usr/share/fonts/truetype/google-poppins/Poppins-Light.ttf"},
        {"Poppins-LightItalic", "/usr/share/fonts/truetype/google-poppins/Poppins-LightItalic.ttf"},
        {"Poppins-Medium", "/usr/share/fonts/truetype/google-poppins/Poppins-Medium.ttf"},
        {"Poppins-MediumItalic",
         "/usr/share/fonts/truetype/google-poppins/Poppins-MediumItalic.ttf"},
        {"Poppins-Regular", "/usr/share/fonts/truetype/google-poppins/Poppins-Regular.ttf"},
        {"Poppins-SemiBold", "/usr/share/fonts/truetype/google-poppins/Poppins-SemiBold.ttf"},
        {"Poppins-SemiBoldItalic",
         "/usr/share/fonts/truetype/google-poppins/Poppins-SemiBoldItalic.ttf"},
        {"Poppins-Thin", "/usr/share/fonts/truetype/google-poppins/Poppins-Thin.ttf"},
        {"Poppins-ThinItalic", "/usr/share/fonts/truetype/google-poppins/Poppins-ThinItalic.ttf"},

        {"NotoSansMono-Regular", "/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf"},
    };

    auto it = path_table.find(font_name);
    if (it == path_table.end()) {
      throw std::runtime_error("Font not found");
    }

    return LoadFont(it->second.data());
  }

private:
  HPDF_Doc pdf_;
};

class QDocGen {
  enum class QDocLegal {
    Proprietary,
    Open,
  };

  HPDF_Doc pdf;
  std::vector<HPDF_Page> pages;
  std::unique_ptr<FontManager> fontmgr;

  void RegisterPage(HPDF_Page page) { pages.push_back(page); }

  void AddPageNumbers() {
    auto font = fontmgr->Get("Poppins-Regular");

    for (size_t i = 0; i < pages.size(); i++) {
      HPDF_Page page = pages[i];
      HPDF_REAL width = HPDF_Page_GetWidth(page);
      HPDF_REAL height = HPDF_Page_GetHeight(page);

      std::string text = std::to_string(i + 1) + "/" + std::to_string(pages.size());
      HPDF_REAL tw = HPDF_Page_TextWidth(page, text.data());
      HPDF_REAL x = width - tw - 40;
      HPDF_REAL y = 40;
      HPDF_Page_SetFontAndSize(page, font, 12);
      HPDF_Page_BeginText(page);
      HPDF_Page_MoveTextPos(page, x, y);
      HPDF_Page_ShowText(page, text.data());
      HPDF_Page_EndText(page);
    }
  }

  void DrawTextCentered(HPDF_Page page, HPDF_Font font, HPDF_REAL font_size, HPDF_REAL y,
                        std::string_view text) {
    HPDF_Page_SetFontAndSize(page, font, font_size);
    HPDF_Page_BeginText(page);
    HPDF_REAL tw = HPDF_Page_TextWidth(page, text.data());
    HPDF_REAL x = (HPDF_Page_GetWidth(page) - tw) / 2;
    HPDF_Page_MoveTextPos(page, x, y);
    HPDF_Page_ShowText(page, text.data());
    HPDF_Page_EndText(page);
  }

  HPDF_Font GetFont(const char* font_name) { return fontmgr->Get(font_name); }

  void render_first_page(std::string_view project_name, uint32_t revision_major,
                         uint32_t revision_minor, std::string entropy, std::string author_name,
                         QDocLegal legal) {
    HPDF_Page page = HPDF_AddPage(pdf);
    RegisterPage(page);

    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    {
      HPDF_Font font = GetFont("Poppins-SemiBold");

      DrawTextCentered(page, font, 28, 620, project_name);
    }

    {
      HPDF_Font font = GetFont("Poppins-SemiBold");

      std::string revision_str =
          "Revision " + std::to_string(revision_major) + "." + std::to_string(revision_minor);
      DrawTextCentered(page, font, 16, 587, revision_str);
    }

    {
      std::string date_str, time_str;
      {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = *std::gmtime(&now_c);
        char date_str_c[64];
        std::strftime(date_str_c, sizeof(date_str_c), "%m/%d/%Y", &now_tm);
        date_str = date_str_c;

        char time_str_c[64];
        std::strftime(time_str_c, sizeof(time_str_c), "%H:%M:%S", &now_tm);
        time_str = time_str_c;
        time_str += " UTC";
      }

      HPDF_Font font = GetFont("Poppins-Medium");

      std::string text = "Generated by QDOC";
      DrawTextCentered(page, font, 14, 450, text);

      text = "on " + date_str;
      DrawTextCentered(page, font, 14, 420, text);

      text = "at " + time_str;
      DrawTextCentered(page, font, 14, 390, text);
    }

    {
      static constexpr boost::uuids::uuid ns_id{};
      std::string text = boost::uuids::to_string(
          boost::uuids::name_generator_sha1{ns_id}(entropy.data(), entropy.size()));
      text = "{ " + text + " }";

      HPDF_Font font = GetFont("NotoSansMono-Regular");

      DrawTextCentered(page, font, 14, 265, text);
    }

    {
      std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      std::time_t now_c = std::chrono::system_clock::to_time_t(now);
      std::tm now_tm = *std::gmtime(&now_c);
      char date_str_c[64];
      std::strftime(date_str_c, sizeof(date_str_c), "%Y", &now_tm);
      std::string date_str = date_str_c;

      HPDF_Font font = GetFont("NotoSansMono-Regular");

      std::string text = "Copyright (C) " + date_str + " " + author_name + " and contributors";
      DrawTextCentered(page, font, 14, 140, text);

      if (legal == QDocLegal::Proprietary) {
        text = "This document is proprietary and confidential.";
        DrawTextCentered(page, font, 14, 110, text);

        text = "All rights reserved.";
        DrawTextCentered(page, font, 14, 80, text);
      } else {
        text = "This document is licensed under the CC BY-ND 4.0.";
        DrawTextCentered(page, font, 14, 110, text);

        text = "It is not permitted to distribute modified";
        DrawTextCentered(page, font, 14, 80, text);

        text = "versions of this document.";
        DrawTextCentered(page, font, 14, 50, text);
      }
    }
  }

  void render_table_of_contents_page() {
    HPDF_Page page = HPDF_AddPage(pdf);
    RegisterPage(page);

    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    HPDF_Font font = GetFont("Poppins-SemiBold");

    DrawTextCentered(page, font, 18, 760, "Table of Contents");
  }

  void render_overview_page(std::string_view overview_text) {
    HPDF_Page page = HPDF_AddPage(pdf);
    RegisterPage(page);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    { /* Overview heading 1 */
      HPDF_Font font = GetFont("Poppins-SemiBold");
      DrawTextCentered(page, font, 18, 760, "Overview");
    }

    { /* Lorem ipsum */

      HPDF_Font font = GetFont("Poppins-Medium");

      // I want to justify this text
      HPDF_Page_SetFontAndSize(page, font, 14);
      HPDF_Page_BeginText(page);
      HPDF_REAL width = HPDF_Page_GetWidth(page);
      HPDF_REAL height = HPDF_Page_GetHeight(page);
      HPDF_REAL margin = 60;
      HPDF_REAL x = margin;
      HPDF_REAL y = height - margin - 60;
      HPDF_Page_MoveTextPos(page, x, y);

      HPDF_Page_TextRect(page, x, y, width - margin, margin, overview_text.data(),
                         HPDF_TALIGN_JUSTIFY, nullptr);
      HPDF_Page_EndText(page);
    }
  }

  void render_exports_section() {
    HPDF_Page page = HPDF_AddPage(pdf);
    RegisterPage(page);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    HPDF_Font font = GetFont("Poppins-SemiBold");

    DrawTextCentered(page, font, 18, 760, "Exports");

    /// TODO:
  }

  void render_requirements_section() {
    HPDF_Page page = HPDF_AddPage(pdf);
    RegisterPage(page);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    HPDF_Font font = GetFont("Poppins-SemiBold");

    DrawTextCentered(page, font, 18, 760, "Requirements");

    /// TODO:
  }

  void render_developers_notes_section() {
    HPDF_Page page = HPDF_AddPage(pdf);
    RegisterPage(page);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    HPDF_Font font = GetFont("Poppins-SemiBold");

    DrawTextCentered(page, font, 18, 760, "Developer Notes");

    /// TODO:
  }

  void render_copyright_section() {
    HPDF_Page page = HPDF_AddPage(pdf);
    RegisterPage(page);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    HPDF_Font font = GetFont("Poppins-SemiBold");

    DrawTextCentered(page, font, 18, 760, "Copyright");

    /// TODO:
  }

public:
  bool GenerateAndSave(std::string_view output_file, std::string_view input_directory) {
    { /* Setup PDF */
      pdf = HPDF_New(error_handler, NULL);
      if (!pdf) {
        printf("ERROR: cannot create pdf object.\n");
        return false;
      }

      HPDF_UseUTFEncodings(pdf);
      HPDF_SetPageMode(pdf, HPDF_PAGE_MODE_USE_THUMBS);
    }

    { /* Setup font manager */
      fontmgr = std::make_unique<FontManager>(pdf);
    }

    QDocLegal legal = QDocLegal::Proprietary;

    const char* overview_body =
        "Lorem ipsum dolor sit amet fugiat ullamco laboris officia cupidatat incididunt id qui "
        "cillum, velit incididunt consequat sint sit deserunt irure officia et. Dolore mollit "
        "culpa deserunt consequat eiusmod eu incididunt,. Officia amet fugiat magna aliquip labore "
        "tempor aliqua aliquip reprehenderit officia enim nisi. Aliquip id et qui do aliquip enim "
        "fugiat culpa esse aliqua tempor amet ut minim laboris ea et ea veniam minim. Mollit "
        "exercitation elit eiusmod laboris do velit nulla voluptate ea commodo incididunt ipsum "
        "duis in sunt eiusmod mollit adipisicing sint aute dolore. Magna ipsum officia sit "
        "adipisicing aute sint deserunt ullamco minim et. Eu cillum reprehenderit est cillum id "
        "irure mollit est eu aute nisi cupidatat in irure deserunt. Voluptate ipsum culpa "
        "cupidatat et ipsum ut sunt excepteur ex ad ut dolore aliquip eu nostrud reprehenderit do "
        "pariatur anim aute dolor consequat, reprehenderit. Adipisicing laborum aliqua aute ad, "
        "veniam tempor. Eu non voluptate elit ut enim magna, eiusmod ea exercitation. Dolor ipsum "
        "excepteur aliqua dolor mollit exercitation commodo sunt voluptate. Eiusmod labore "
        "deserunt dolore qui mollit Lorem sit amet. Exercitation dolore non reprehenderit, esse "
        "voluptate. Culpa excepteur, adipisicing aute enim mollit dolor proident id officia.";

    { /* Generate content */
      render_first_page("Jones Software Suite", 1, 0, "6969-00", "Wesley Jones", legal);
      render_table_of_contents_page();
      render_overview_page(overview_body);

      render_exports_section();
      render_requirements_section();
      render_developers_notes_section();
      render_copyright_section();

      AddPageNumbers();
    }

    HPDF_SaveToFile(pdf, "test.pdf");

    HPDF_Free(pdf);

    return true;
  }
};

int main() {
  QDocGen gen;
  gen.GenerateAndSave("test.pdf", "test");
  return 0;
}