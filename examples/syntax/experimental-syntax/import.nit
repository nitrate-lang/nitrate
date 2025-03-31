################################################################################
# File: orbit-core/macro/src/import.q                                          #
# Author: Wesley Jones                                                         #
# Development Notes:  None                                                     #
################################################################################

@(fn import() {
  local module = n.next();
  local semi = n.next();

  if module.ty ~= 'name' then
    n.abort('Expected module name after @import.');
    return
  end

  if semi.ty ~= 'sym' or semi.v ~= ';' then
    n.abort('Expected semicolon after module name');
    return
  end

  module = module.v;

  -- Begin processing the import
  n.ilog('Processing import of module: ', module);

  module = string.gsub(module, '::', '/');
  module = string.format('http://localhost:%d/api/v1/fetch?job=%s&name=%s.qh',
    10,
    10,
    module);

  n.ilog('Attempting to open module: ', module);

  local content = n.fetch(module);
  if content == nil then
    n.abort('Failed to fetch module: ', n.errno);
    return
  end

  n.ilog('Fetched module: ', module);

  return content;
})

@import core::panic;
