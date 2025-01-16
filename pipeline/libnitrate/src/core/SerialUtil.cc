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

#include <core/SerialUtil.hh>
#include <nitrate-core/Logger.hh>

auto CreateJsonString(std::string_view input) -> std::string {
  std::string output = "\"";
  output.reserve(input.length() * 2);

  for (char ch : input) {
    switch (ch) {
      case '"':
        output += "\\\"";
        break;
      case '\\':
        output += "\\\\";
        break;
      case '\b':
        output += "\\b";
        break;
      case '\f':
        output += "\\f";
        break;
      case '\n':
        output += "\\n";
        break;
      case '\r':
        output += "\\r";
        break;
      case '\t':
        output += "\\t";
        break;
      case '\0':
        output += "\\0";
        break;
      default:
        if (ch >= 32 && ch < 127) {
          output += ch;
        } else {
          char hex[5];
          snprintf(hex, sizeof(hex), "\\x%02x", (int)(uint8_t)ch);
          output += hex;
        }
        break;
    }
  }

  output += "\"";

  return output;
}

auto ReadJsonString(std::istream &i, char **str, size_t &len) -> bool {
  size_t cap = 0;
  *str = nullptr;
  len = 0;

  int ch;

  if ((ch = i.get()) != '"') {
    return false;
  }

  while ((ch = i.get()) != EOF) {
    if (ch == '"') {
      *str = (char *)realloc(*str, len + 1);
      if (!*str) {
        return false;
      }

      (*str)[len] = '\0';
      return true;
    } else if (ch == '\\') {
      ch = i.get();
      if (ch == EOF) {
        return false;
      }

      switch (ch) {
        case 'b':
          ch = '\b';
          break;
        case 'f':
          ch = '\f';
          break;
        case 'n':
          ch = '\n';
          break;
        case 'r':
          ch = '\r';
          break;
        case 't':
          ch = '\t';
          break;
        case '0':
          ch = '\0';
          break;
        case 'x': {
          int hex = 0;
          for (int j = 0; j < 2; j++) {
            ch = i.get();
            if (ch == EOF) {
              return false;
            }

            if (ch >= '0' && ch <= '9') {
              hex = (hex << 4) | (ch - '0');
            } else if (ch >= 'a' && ch <= 'f') {
              hex = (hex << 4) | (ch - 'a' + 10);
            } else if (ch >= 'A' && ch <= 'F') {
              hex = (hex << 4) | (ch - 'A' + 10);
            } else {
              return false;
            }
          }

          ch = hex;
          break;
        }
        default:
          break;
      }
    }

    if (len >= cap) {
      cap = cap ? cap * 2 : 64;
      *str = (char *)realloc(*str, cap);
      if (!*str) {
        return false;
      }
    }

    (*str)[len++] = ch;
  }

  return false;
}

#define FGETC(__I)                           \
  if ((ch = __I.get()) == ((uint32_t)EOF)) { \
    return false;                            \
  }

auto MsgpackWriteUint(std::ostream &o, uint64_t x) -> bool {
  if (x <= INT8_MAX) {
    o.put(x & 0x7f);
  } else if (x <= UINT8_MAX) {
    o.put(0xcc);
    o.put(x);
  } else if (x <= UINT16_MAX) {
    o.put(0xcd);
    o.put((x >> 8) & 0xff);
    o.put(x & 0xff);
  } else if (x <= UINT32_MAX) {
    o.put(0xce);
    o.put((x >> 24) & 0xff);
    o.put((x >> 16) & 0xff);
    o.put((x >> 8) & 0xff);
    o.put(x & 0xff);
  } else {
    o.put(0xcf);
    o.put((x >> 56) & 0xff);
    o.put((x >> 48) & 0xff);
    o.put((x >> 40) & 0xff);
    o.put((x >> 32) & 0xff);
    o.put((x >> 24) & 0xff);
    o.put((x >> 16) & 0xff);
    o.put((x >> 8) & 0xff);
    o.put(x & 0xff);
  }

  return true;
}

auto MsgpackReadUint(std::istream &i, uint64_t &x) -> bool {
  x = 0;

  uint32_t ch;
  FGETC(i);

  if ((ch & 0x80) == 0) {
    x = ch;
  } else if (ch == 0xcc) {
    FGETC(i);
    x = ch;
  } else if (ch == 0xcd) {
    FGETC(i);
    x = ch << 8;
    FGETC(i);
    x |= ch;
  } else if (ch == 0xce) {
    x = 0;
    FGETC(i);
    x |= ch << 24;
    FGETC(i);
    x |= ch << 16;
    FGETC(i);
    x |= ch << 8;
    FGETC(i);
    x |= ch;
  } else if (ch == 0xcf) {
    x = 0;
    FGETC(i);
    x |= (uint64_t)ch << 56;
    FGETC(i);
    x |= (uint64_t)ch << 48;
    FGETC(i);
    x |= (uint64_t)ch << 40;
    FGETC(i);
    x |= (uint64_t)ch << 32;
    FGETC(i);
    x |= (uint64_t)ch << 24;
    FGETC(i);
    x |= (uint64_t)ch << 16;
    FGETC(i);
    x |= (uint64_t)ch << 8;
    FGETC(i);
    x |= (uint64_t)ch;
  } else {
    return false;
  }

  return true;
}

auto MsgpackWriteStr(std::ostream &o, std::string_view str) -> bool {
  size_t sz = str.size();

  if (sz <= 31) {
    o.put(0b10100000 | sz);
  } else if (sz <= UINT8_MAX) {
    o.put(0xd9);
    o.put(sz);
  } else if (sz <= UINT16_MAX) {
    o.put(0xda);
    o.put((sz >> 8) & 0xff);
    o.put(sz & 0xff);
  } else if (sz <= UINT32_MAX) {
    o.put(0xdb);
    o.put((sz >> 24) & 0xff);
    o.put((sz >> 16) & 0xff);
    o.put((sz >> 8) & 0xff);
    o.put(sz & 0xff);
  }

  o.write(str.data(), sz);

  return true;
}

auto MsgpackReadStr(std::istream &i, char **str, size_t &len) -> bool {
  len = 0;

  uint32_t ch;
  FGETC(i);

  if ((ch & 0b10100000) == 0b10100000) {
    len = ch & 0b00011111;
  } else if (ch == 0xd9) {
    FGETC(i);
    len = ch;
  } else if (ch == 0xda) {
    FGETC(i);
    len = ch << 8;
    FGETC(i);
    len |= ch;
  } else if (ch == 0xdb) {
    len = 0;
    FGETC(i);
    len |= ch << 24;
    FGETC(i);
    len |= ch << 16;
    FGETC(i);
    len |= ch << 8;
    FGETC(i);
    len |= ch;
  } else {
    return false;
  }

  *str = (char *)malloc(len + 1);
  if (!*str) {
    return false;
  }

  if (!i.read(*str, len)) {
    free(*str);
    return false;
  }

  (*str)[len] = '\0';

  return true;
}
