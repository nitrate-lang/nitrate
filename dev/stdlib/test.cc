#include <quix/int.hh>
#include <quix/stdint.hh>

#define W_FUNC(__decl, ...) \
  __decl { __VA_ARGS__ }

W_FUNC(int qapi_read(), { return -1; });
W_FUNC(int qapi_write(), { return -1; });
