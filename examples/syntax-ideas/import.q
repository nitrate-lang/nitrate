################################################################################
# File: orbit-core/macro/src/import.q                                          #
# Author: Wesley Jones                                                         #
# Development Notes:  None                                                     #
################################################################################

@(fn import() {
  local module = quix.api.next();
  local semi = quix.api.next();

  if module.ty ~= 'name' then
    quix.api.abort('Expected module name after @import.');
    return
  end

  if semi.ty ~= 'sym' or semi.v ~= ';' then
    quix.api.abort('Expected semicolon after module name');
    return
  end

  module = module.v;

  -- Begin processing the import
  quix.api.ilog('Processing import of module: ', module);

  module = string.gsub(module, '::', '/');
  module = string.format('http://localhost:%d/api/v1/fetch?job=%s&name=%s.qh', 
    10,
    10,
    module);

  quix.api.ilog('Attempting to open module: ', module);

  local content = quix.api.fetch(module);
  if content == nil then
    quix.api.abort('Failed to fetch module: ', quix.api.errno);
    return
  end

  quix.api.ilog('Fetched module: ', module);

  return content;
})

@import core::panic;
