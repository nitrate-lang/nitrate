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

#define LIBNITRATE_INTERNAL

#include <nitrate-core/Error.h>

#include <SerialUtil.hh>

std::string create_json_string(std::string_view input) {
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

bool read_json_string(std::istream &I, char **str, size_t &len) {
  size_t cap = 0;
  *str = nullptr;
  len = 0;

  int ch;

  if ((ch = I.get()) != '"') {
    return false;
  }

  while ((ch = I.get()) != EOF) {
    if (ch == '"') {
      *str = (char *)realloc(*str, len + 1);
      if (!*str) {
        return false;
      }

      (*str)[len] = '\0';
      return true;
    } else if (ch == '\\') {
      ch = I.get();
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
          for (int i = 0; i < 2; i++) {
            ch = I.get();
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

#define FPUTC(__x, __O)         \
  if (fputc(__x, __O) == EOF) { \
    return false;               \
  }

#define FGETC(__I)                           \
  if ((ch = __I.get()) == ((uint32_t)EOF)) { \
    return false;                            \
  }

bool msgpack_write_uint(FILE *O, uint64_t x) {
  if (x <= INT8_MAX) {
    FPUTC(x & 0x7f, O);
  } else if (x <= UINT8_MAX) {
    FPUTC(0xcc, O);
    FPUTC(x, O);
  } else if (x <= UINT16_MAX) {
    FPUTC(0xcd, O);
    FPUTC((x >> 8) & 0xff, O);
    FPUTC(x & 0xff, O);
  } else if (x <= UINT32_MAX) {
    FPUTC(0xce, O);
    FPUTC((x >> 24) & 0xff, O);
    FPUTC((x >> 16) & 0xff, O);
    FPUTC((x >> 8) & 0xff, O);
    FPUTC(x & 0xff, O);
  } else {
    FPUTC(0xcf, O);
    FPUTC((x >> 56) & 0xff, O);
    FPUTC((x >> 48) & 0xff, O);
    FPUTC((x >> 40) & 0xff, O);
    FPUTC((x >> 32) & 0xff, O);
    FPUTC((x >> 24) & 0xff, O);
    FPUTC((x >> 16) & 0xff, O);
    FPUTC((x >> 8) & 0xff, O);
    FPUTC(x & 0xff, O);
  }

  return true;
}

bool msgpack_read_uint(std::istream &I, uint64_t &x) {
  x = 0;

  uint32_t ch;
  FGETC(I);

  if ((ch & 0x80) == 0) {
    x = ch;
  } else if (ch == 0xcc) {
    FGETC(I);
    x = ch;
  } else if (ch == 0xcd) {
    FGETC(I);
    x = ch << 8;
    FGETC(I);
    x |= ch;
  } else if (ch == 0xce) {
    x = 0;
    FGETC(I);
    x |= ch << 24;
    FGETC(I);
    x |= ch << 16;
    FGETC(I);
    x |= ch << 8;
    FGETC(I);
    x |= ch;
  } else if (ch == 0xcf) {
    x = 0;
    FGETC(I);
    x |= (uint64_t)ch << 56;
    FGETC(I);
    x |= (uint64_t)ch << 48;
    FGETC(I);
    x |= (uint64_t)ch << 40;
    FGETC(I);
    x |= (uint64_t)ch << 32;
    FGETC(I);
    x |= (uint64_t)ch << 24;
    FGETC(I);
    x |= (uint64_t)ch << 16;
    FGETC(I);
    x |= (uint64_t)ch << 8;
    FGETC(I);
    x |= (uint64_t)ch;
  } else {
    return false;
  }

  return true;
}

bool msgpack_write_str(FILE *O, std::string_view str) {
  size_t sz = str.size();

  if (sz <= 31) {
    FPUTC(0b10100000 | sz, O);
  } else if (sz <= UINT8_MAX) {
    FPUTC(0xd9, O);
    FPUTC(sz, O);
  } else if (sz <= UINT16_MAX) {
    FPUTC(0xda, O);
    FPUTC((sz >> 8) & 0xff, O);
    FPUTC(sz & 0xff, O);
  } else if (sz <= UINT32_MAX) {
    FPUTC(0xdb, O);
    FPUTC((sz >> 24) & 0xff, O);
    FPUTC((sz >> 16) & 0xff, O);
    FPUTC((sz >> 8) & 0xff, O);
    FPUTC(sz & 0xff, O);
  }

  if (fwrite(str.data(), 1, sz, O) != sz) {
    return false;
  }

  return true;
}

bool msgpack_read_str(std::istream &I, char **str, size_t &len) {
  len = 0;

  uint32_t ch;
  FGETC(I);

  if ((ch & 0b10100000) == 0b10100000) {
    len = ch & 0b00011111;
  } else if (ch == 0xd9) {
    FGETC(I);
    len = ch;
  } else if (ch == 0xda) {
    FGETC(I);
    len = ch << 8;
    FGETC(I);
    len |= ch;
  } else if (ch == 0xdb) {
    len = 0;
    FGETC(I);
    len |= ch << 24;
    FGETC(I);
    len |= ch << 16;
    FGETC(I);
    len |= ch << 8;
    FGETC(I);
    len |= ch;
  } else {
    return false;
  }

  *str = (char *)malloc(len + 1);
  if (!*str) {
    return false;
  }

  if (!I.read(*str, len)) {
    qcore_panic("msgpack_read_str: fread failed");
    free(*str);
    return false;
  }

  (*str)[len] = '\0';

  return true;
}
