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

#include <string_view>

std::string_view quix_code_prefix = R"(@(fn define() {
  -- Read in expected lexical sequence
  local name = quix.next(); local sepr = quix.next();
  local valu = quix.next(); local semi = quix.next();

  -- Verify that the sequence is correct
  if name.ty ~= 'name' then 
    quix.abort('Expected name in @define macro'); 
  end
  if sepr.ty ~= 'op' or sepr.v ~= '=' then 
    quix.abort('Expected = in @define macro'); 
  end
  if semi.ty ~= 'sym' or semi.v ~= ';' then 
    quix.abort('Expected ; in @define macro'); 
  end

  -- Set the define in the magic 'def.' namespace
  quix.set('def.'.. name.v, valu.v);
})

@(fn compile_if(cond, terminator) {
  -- Nothing to do if condition is true
  if cond then
    return
  end

  -- Default terminator
  if terminator == nil then
    terminator = 'compile_endif()'
  end

  -- Handle nested @compile_if blocks
  local rank = 1
  while true do
    local token = quix.next()
    if token.ty == 'eof' then
      quix.error('Unexpected EOF in conditional compilation block')
      break
    end

    if token.ty == 'macr' and token.v == terminator then
      rank = rank - 1
      if rank == 0 then
        break
      end
    end

    if (token.ty == 'macr' and string.match(token.v, '^compile_if(.*)$')) then
      rank = rank + 1
    end
  end
})

@(fn compile_endif() {})

@(fn use() {
  -- Read in expected lexical sequence
  local ver = quix.next(); local semi = quix.next(); 

  -- Verify that the sequence is correct
  if ver.ty ~= 'str' then
    quix.abort('Expected version string after @use');
  end
  if semi.ty ~= 'sym' or semi.v ~= ';' then
    quix.abort('Expected semicolon after version string');
  end

  -- For now only support v1.0
  if ver.v ~= 'v1.0' then
    quix.abort('Unsupported QUIX environment version: ', ver.v);
  end

  quix.debug('Using QUIX environment version: ', ver.v);
})

@(fn import() {
  -- Read in expected lexical sequence
  local name = quix.next(); local semi = quix.next();

  -- Verify that the sequence is correct
  if semi.ty ~= 'sym' or semi.v ~= ';' then
    quix.abort('Expected semicolon after module name');
  end
  if name.ty == 'str' then
    local content = quix.fetch(name.v);
    if content == nil then
      quix.abort('Failed to fetch module: ', quix.errno);
    end
    return content;
  end
  if name.ty ~= 'name' then
    quix.abort('Expected module name after @import.');
  end
  name = name.v;

  -- Begin processing the import
  quix.debug('Processing import of module: ', name);

  -- Create target URL
  name = string.gsub(name, '::', '/');
  name = string.format('http://localhost:%d/api/v1/fetch?job=%s&name=%s.qh', 
    tonumber(quix.get('this.srvport')),
    quix.get('this.job'),
    name);

  -- Fetch the module
  quix.debug('Attempting to open module: ', name);
  local content = quix.fetch(name);
  if content == nil then
    quix.abort('Failed to fetch module: ', quix.errno);
  end

  quix.debug(string.format('Fetched module: %s (%d bytes)', name, #content));

  return content;
})

@(
  function isset(name, value) 
    if name == nil then
      quix.abort('Expected name in @isset function');
    end

    local flag = quix.get('flag.'.. name);
    if value == nil then
      return flag ~= nil;
    end
    return flag == value;
  end

  function get_target() 
    return quix.get('this.target-triple');
  end

  function get_host() 
    return quix.get('this.host-triple');
  end

  function try_set(name, value)
    if name == nil then
      quix.abort('Expected name in @req_flag_set function');
    end

    local mut_flags = quix.get('flag.mutable');
    if mut_flags == nil then
      quix.debug('The expected mutable flags table is not set');
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

    quix.set(name, value);
    return true;
  end

  function putflag(name, value) 
    if try_set(name, value) then
      return;
    end

    quix.abort('Immutable flag could not be modified: ', name);
  end
)
)";
