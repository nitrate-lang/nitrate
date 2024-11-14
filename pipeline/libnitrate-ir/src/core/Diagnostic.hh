////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __NITRATE_QXIR_DIAGNOSTIC_H__
#define __NITRATE_QXIR_DIAGNOSTIC_H__

#include <nitrate-ir/IR.h>

#include <boost/bimap.hpp>
#include <cstdarg>
#include <functional>
#include <nitrate-ir/Report.hh>
#include <span>
#include <string_view>
#include <unordered_set>

namespace qparse {
  class Node;
}

namespace nr {
  struct IssueInfo {
    std::string_view flagname;
    std::string overview;
    std::vector<std::string_view> hints;

    bool operator<(const IssueInfo &rhs) const { return flagname < rhs.flagname; }
  };

  extern const boost::bimap<IssueCode, IssueInfo> issue_info;

  typedef std::function<void(std::string_view, IC)> DiagnosticMessageHandler;

  struct DiagDatum {
    IssueCode code;
    IC level;
    std::string param;
    uint32_t start_offset;
    uint32_t end_offset;
    std::string_view filename;

    DiagDatum(IssueCode _code, IC _level, std::string _param, uint32_t _start_offset,
              uint32_t _end_offset, std::string_view _filename)
        : code(_code),
          level(_level),
          param(std::move(_param)),
          start_offset(_start_offset),
          end_offset(_end_offset),
          filename(_filename) {}

    uint64_t hash() const;
  };

  std::string mint_clang16_message(const IReport::ReportData &R, ISourceView *B);
  std::string mint_plain_message(const IReport::ReportData &R, ISourceView *B);
  std::string mint_modern_message(const IReport::ReportData &R, ISourceView *B);

  class DiagnosticManager final : public IReport {
    std::vector<DiagDatum> m_vec;
    std::unordered_set<uint64_t> m_visited;

  public:
    DiagnosticManager() = default;

    virtual void report(IssueCode code, IC level, std::span<std::string_view> params = {},
                        uint32_t start_offset = 1, uint32_t end_offset = 0,
                        std::string_view filename = "") override;

    virtual void stream_reports(std::function<void(const ReportData &)> cb) override;

    virtual void erase_reports() override {
      m_vec.clear();
      m_visited.clear();
    }

    size_t size() { return m_vec.size(); }
  };

};  // namespace nr

#endif  // __NITRATE_QXIR_REPORT_H__
