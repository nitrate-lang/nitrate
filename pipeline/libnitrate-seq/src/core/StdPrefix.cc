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

#include <nitrate-seq/Sequencer.hh>

const std::string_view ncc::seq::SEQUENCER_DIALECT_CODE_PREFIX = R"(@(
function comp_if(cond, terminator)
  -- Nothing to do if condition is true
  if cond then
    return
  end

  -- Default terminator
  if terminator == nil then
    terminator = 'comp_endif()'
  end

  -- Handle nested @comp_if blocks
  local rank = 1
  while true do
    local token = n.next()
    if token.ty == 'eof' then
      n.error('Unexpected EOF in conditional compilation block')
      break
    end

    if token.ty == 'macr' and token.v == terminator then
      rank = rank - 1
      if rank == 0 then
        break
      end
    end

    if (token.ty == 'macr' and string.match(token.v, '^comp_if(.*)$')) then
      rank = rank + 1
    end
  end
end
)

@(function comp_endif() end)

@(
function use()
  -- Read in expected lexical sequence
  local ver = n.next(); local semi = n.next();

  -- Verify that the sequence is correct
  if ver.ty ~= 'str' then
    n.abort('Expected version string after @use');
  end
  if semi.ty ~= 'sym' or semi.v ~= ';' then
    n.abort('Expected semicolon after version string');
  end

  -- For now only support v1.0
  if ver.v ~= 'v1.0' then
    n.abort('Unsupported Nitrate environment version: ', ver.v);
  end

  n.debug('Using Nitrate environment version: ', ver.v);
end
)

@(
  n.isset = function(name, value)
    if name == nil then
      n.abort('Expected name in @isset function');
    end

    local flag = n.get('flag.'.. name);
    if value == nil then
      return flag ~= nil;
    end
    return flag == value;
  end

  n.get_target = function()
    return n.get('this.target-triple');
  end

  n.get_host = function()
    return n.get('this.host-triple');
  end

  n.try_set = function(name, value)
    if name == nil then
      n.abort('Expected name in @req_flag_set function');
    end

    local mut_flags = n.get('flag.mutable');
    if mut_flags == nil then
      n.debug('The expected mutable flags table is not set');
      return false;
    end

    -- Split into array of names separated by ','
    local names = {};
    for name in string.gmatch(name, '([^,]+)') do
      table.insert(names, name);
    end

    if mut_flags[name] == nil then
      return false;
    end

    n.set(name, value);
    return true;
  end

  n.set_flag = function(name, value)
    if try_set(name, value) then
      return;
    end

    n.abort('Immutable flag could not be modified: ', name);
  end

  n.enstr = function(item)
    if item == nil then
      item = '';
    end

    item = tostring(item);

    local res = '"';

    for i = 1, #item do
      -- If the character is in the normal ASCII range, just add it
      if string.byte(item, i) >= 32 and string.byte(item, i) <= 126 then
        res = res .. string.sub(item, i, i);
      else -- Otherwise, add the escape sequence
        res = res .. string.format('\\x%02x', string.byte(item, i));
      end
    end

    return res .. '"';
  end

  n.destr = function(item)
    if item == nil then
      return '';
    end

    if #item < 2 then
      return item;
    end

    -- check for "'" or '"' at the beginning and end of the string
    local first = string.sub(item, 1, 1);
    local last = string.sub(item, #item, #item);

    if first ~= last or (first ~= '"' and first ~= "'") then
      error('String is not properly quoted');
    end

    item = string.sub(item, 2, #item - 1);

    local res = '';
    local i = 1;
    while i <= #item do
      local ch = string.sub(item, i, i);

      if ch == '\\' then
        local esc = string.sub(item, i + 1, i + 1);
        if esc == 'x' or esc == 'X' then
          local hex = string.sub(item, i + 2, i + 3);
          res = res .. string.byte(tonumber(hex, 16));
          i = i + 4;
        elseif esc == 'u' then
          if string.sub(item, i + 2, i + 2) == '{' then
            local hex = '';
            i = i + 3;
            while string.sub(item, i, i) ~= '}' do
              hex = hex .. string.sub(item, i, i);
              i = i + 1;
            end

            local code = tonumber(hex, 16);
            i = i + 1;

            -- Escape into UTF-8
            if code < 0x80 then
              res = res .. string.char(code);
            elseif code < 0x800 then
              res = res .. string.char(0xC0 | (code >> 6));
              res = res .. string.char(0x80 | (code & 0x3F));
            elseif code < 0x10000 then
              res = res .. string.char(0xE0 | (code >> 12));
              res = res .. string.char(0x80 | ((code >> 6) & 0x3F));
              res = res .. string.char(0x80 | (code & 0x3F));
            elseif code < 0x110000 then
              res = res .. string.char(0xF0 | (code >> 18));
              res = res .. string.char(0x80 | ((code >> 12) & 0x3F));
              res = res .. string.char(0x80 | ((code >> 6) & 0x3F));
              res = res .. string.char(0x80 | (code & 0x3F));
            else
              error('Invalid unicode escape');
            end
          else
            error('Invalid unicode escape');
          end
        elseif esc == 'o' then
          local oct = string.sub(item, i + 2, i + 4);
          res = res .. string.byte(tonumber(oct, 8));
          i = i + 5;
        elseif esc == 'b' then
          if string.sub(item, i + 2, i + 2) == '{' then
            local bin = '';
            i = i + 3;
            while string.sub(item, i, i) ~= '}' do
              bin = bin .. string.sub(item, i, i);
              i = i + 1;
            end

            while #bin % 8 ~= 0 do
              bin = '0' .. bin;
            end

            for j = 1, #bin, 8 do
              res = res .. string.char(tonumber(string.sub(bin, j, j + 7),
              2));
            end

            i = i + 1;
          else
            error('Invalid binary escape');
          end
        elseif esc == 'n' then
          res = res .. '\n';
          i = i + 2;
        elseif esc == 'r' then
          res = res .. '\r';
          i = i + 2;
        elseif esc == 't' then
          res = res .. '\t';
          i = i + 2;
        elseif esc == 'v' then
          res = res .. '\v';
          i = i + 2;
        elseif esc == 'f' then
          res = res .. '\f';
          i = i + 2;
        elseif esc == 'a' then
          res = res .. '\a';
          i = i + 2;
        elseif esc == '0' then
          res = res .. '\0';
          i = i + 2;
        elseif esc == '\\' then
          res = res .. '\\';
          i = i + 2;
        else
          error('Unknown escape sequence: \\' .. esc);
        end
      else
        res = res .. ch;
        i = i + 1;
      end
    end

    return res;
  end
)

@(
  n.env_keys = function()
    local data = n.get("this.keys")
    local key_length_buffer = ''
    local keys = {}
    local i = 1

    if data == nil or data == "" then
      return {}
    end

    while i < #data do
      local c = data:sub(i, i)
      i = i + 1
      
      if c:match("%d") then
        key_length_buffer = key_length_buffer .. c
      else
        local key_length = tonumber(key_length_buffer)
        key_length_buffer = ''

        if key_length == nil then
          return {}
        end

        table.insert(keys, data:sub(i, i + key_length - 1))

        i = i + key_length
      end
    end

    return keys
  end
)
)";
