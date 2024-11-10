################################################################################
# File: orbit-core/macro/src/import.q                                          #
# Author: Wesley Jones                                                         #
# Development Notes:  None                                                     #
################################################################################

@(fn import() {
  local module = nit.api.next();
  local semi = nit.api.next();

  if module.ty ~= 'name' then
    nit.api.abort('Expected module name after @import.');
    return
  end

  if semi.ty ~= 'sym' or semi.v ~= ';' then
    nit.api.abort('Expected semicolon after module name');
    return
  end

  module = module.v;

  -- Begin processing the import
  nit.api.ilog('Processing import of module: ', module);

  module = string.gsub(module, '::', '/');
  module = string.format('http://localhost:%d/api/v1/fetch?job=%s&name=%s.qh', 
    10,
    10,
    module);

  nit.api.ilog('Attempting to open module: ', module);

  local content = nit.api.fetch(module);
  if content == nil then
    nit.api.abort('Failed to fetch module: ', nit.api.errno);
    return
  end

  nit.api.ilog('Fetched module: ', module);

  return content;
})

@import core::panic;
