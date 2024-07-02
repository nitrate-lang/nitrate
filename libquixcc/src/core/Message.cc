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
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <core/Exception.h>
#include <core/Logger.h>

LOGGER_SETUP();  // Define the logger

void libquixcc::LoggerGroup::push_message_to_job(quixcc_job_t &job,
                                                 libquixcc::E type,
                                                 const std::string &message) {
  // Create a new message
  quixcc_msg_t *msg = (quixcc_msg_t *)malloc(sizeof(quixcc_msg_t));
  if (msg == nullptr) throw std::bad_alloc();
  msg->line = 0;
  msg->column = 0;
  msg->message = strdup(message.c_str());
  if (msg->message == nullptr) throw std::bad_alloc();
  msg->m_level = (quixcc_msg_level_t)type;

  // Add the message to the job
  job.m_result.m_messages = (quixcc_msg_t **)realloc(
      job.m_result.m_messages,
      (job.m_result.m_count + 1) * sizeof(quixcc_msg_t *));
  if (job.m_result.m_messages == nullptr) throw std::bad_alloc();
  job.m_result.m_messages[job.m_result.m_count] = msg;
  job.m_result.m_count++;

  // Flush messages to stderr when debug is enabled
  if (job.m_debug) std::cerr << message << std::endl;

  // Throw an exception if the message is an error or fatal
  if (type == E::ERROR) {
    /// TODO: Don't use error handling as control flow
    job.m_tainted = true;
    throw core::Exception(message);
  } else if (type == E::FAILED || type == E::FATAL) {
    // This means we are in an invalid state, behavior is probably undefined.
    // If/When we segfault, the compiler will catch this upstream
    job.m_tainted = true;
  }
}

libquixcc::Logger &libquixcc::LoggerGroup::operator[](libquixcc::E level) {
  if (!m_job) {
    /// TODO: fix other code that errorously uses the logger without a job
    return *m_hole; /* No job, no logging */
  }
  if (!m_job->m_debug && level == E::DEBUG)
    return *m_hole;  // Null-object pattern

  return *m_loggers[level];
}

bool libquixcc::LoggerGroup::is_color_enabled() {
#if defined(_WIN32) || defined(_WIN64)
  // I don't know windows color codes
  return false;
#else
  static bool _G_is_color_enabled =
      (getenv("QUIXCC_COLOR") == nullptr) ||
      (std::string(getenv("QUIXCC_COLOR")) == "1");
  return _G_is_color_enabled;
#endif
}

std::string libquixcc::LoggerGroup::format_message_ansi(
    const std::string &message, libquixcc::E type,
    const libquixcc::Token &tok) {
  std::stringstream msg;

  /* Add the file, line, and column information */
  if (tok.loc().file != "") {
    msg << "\x1b[49;1m" << tok.loc().file << ":";
    msg << std::to_string(tok.loc().line) << ":"
        << std::to_string(tok.loc().col) << ":\x1b[0m ";
  }

  /* ANSI color codes */
  switch (type) {
    case E::DEBUG:
      msg << "\x1b[49;1mdebug:\x1b[0m " << message << "\x1b[0m";
      break;
    case E::SUCCESS:
      msg << "\x1b[32;49;1msuccess:\x1b[0m " << message << "\x1b[0m";
      break;
    case E::INFO:
      msg << "\x1b[37;49;1minfo:\x1b[0m \x1b[37;49m" << message << "\x1b[0m";
      break;
    case E::WARN:
      msg << "\x1b[35;49;1mwarning:\x1b[0m \x1b[37;49;1m" << message
          << "\x1b[0m";
      break;
    case E::ERROR:
    case E::FAILED:
      msg << "\x1b[31;49;1merror:\x1b[0m \x1b[37;49;1m" << message << "\x1b[0m";
      break;
    case E::FATAL:
      msg << "\x1b[31;49;1mINTERNAL COMPILER ERROR:\x1b[0m \x1b[37;49;1m"
          << message << "\x1b[0m";
      break;
    case E::RAW:
      msg << message;
      break;
  }

  return msg.str();
}

std::string libquixcc::LoggerGroup::format_message_nocolor(
    const std::string &message, libquixcc::E type,
    const libquixcc::Token &tok) {
  std::stringstream msg;

  /* Add the file, line, and column information */
  if (tok.loc().file != "") {
    msg << tok.loc().file << ":";
    msg << std::to_string(tok.loc().line) << ":"
        << std::to_string(tok.loc().col) << ": ";
  }

  switch (type) {
    case E::DEBUG:
      msg << "(debug): " << message;
      break;
    case E::SUCCESS:
      msg << "(success): " << message;
      break;
    case E::INFO:
      msg << "(info): " << message;
      break;
    case E::WARN:
      msg << "(WARNING): " << message;
      break;
    case E::ERROR:
    case E::FAILED:
      msg << "(ERROR): " << message;
      break;
    case E::FATAL:
      msg << "(FATAL): " << message;
      break;
    case E::RAW:
      msg << message;
      break;
  }

  return msg.str();
}
