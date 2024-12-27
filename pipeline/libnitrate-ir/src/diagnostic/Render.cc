////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
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

#include <core/Diagnostic.hh>
#include <cstdint>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/Report.hh>
#include <nitrate-parser/AST.hh>
#include <sstream>

using namespace ncc;
using namespace ncc::ir;
using namespace ncc::lex;

///============================================================================///

using namespace ncc::ir;

uint64_t DiagDatum::hash() const {
  /* Not quite a PHF, but it's pretty close as long as there are not two many
   * subject strings */
  /* In the worst case messages will be discarded, but that can be fixed by
     passing a parameter to disable deduplication */

  struct BitPack {
    IC level : 3;
    IssueCode code : 10;
    uint64_t param_trunc : 7;
    uint32_t m_fileid : 20;
    uint64_t m_start : 24;
  } __attribute__((packed)) bp;

  bp.level = level;
  bp.code = code;
  bp.param_trunc = std::hash<std::string_view>{}(param);
  bp.m_start = start_offset;
  bp.m_fileid = fileid;

  return std::bit_cast<uint64_t>(bp);
}

void DiagnosticManager::report(IssueCode code, IC level,
                               std::vector<std::string_view> params,
                               std::tuple<uint32_t, uint32_t> loc) {
  std::string message;
  for (auto p : params) {
    message += std::string(p);
  }

  DiagDatum R(code, level, message, std::get<0>(loc), std::get<1>(loc));

  { /* Prevent duplicates and maintain order of messages */
    auto hash = R.hash();
    if (m_visited.contains(hash)) {
      return;
    }
    m_visited.insert(hash);
  }

  m_vec.emplace_back(std::move(R));
}

void DiagnosticManager::stream_reports(
    std::function<void(const ReportData &)> cb) {
  for (auto &item : m_vec) {
    ReportData datum;
    datum.code = item.code;
    datum.level = item.level;
    datum.param = item.param;
    datum.start_offset = item.start_offset;
    datum.fileid = item.fileid;

    cb(datum);
  }
}

static const std::unordered_map<IC, nr_level_t> issue_class_map = {
    {IC::Debug, IR_LEVEL_DEBUG},
    {IC::Info, IR_LEVEL_INFO},
    {IC::Warn, IR_LEVEL_WARN},
    {IC::Error, IR_LEVEL_ERROR},
};

CPP_EXPORT void ir::nr_diag_read(IRModule *nr, nr_diag_format_t format,
                                 nr_report_cb cb, uintptr_t data) {
  if (!cb) {
    return;
  }

  nr->getDiag()->stream_reports([&](IReport::ReportData R) {
    std::stringstream ss;

    switch (format) {
      case IR_DIAG_COLOR: {
        ss << ir::mint_modern_message(R);
        break;
      }
    }

    std::string message = ss.str();
    const uint8_t *ptr = (const uint8_t *)message.c_str();
    auto lvl = issue_class_map.at(R.level);

    cb(ptr, message.size(), lvl, data);
  });
}
