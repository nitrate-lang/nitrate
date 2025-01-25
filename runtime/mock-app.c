#include <stdint.h>
#include <unistd.h>

#include "nitrate-rt.h"

static size_t strlen(const char *str) {
  size_t len = 0;
  while (*str++) {
    len++;
  }
  return len;
}

static void print(const char *str) { syscall(1, 1, str, strlen(str)); }

///=============================================================================

char object_a;
int object_a_ctor(void *obj) {
  (void)obj;
  print("Hello, object_a_ctor!\n");
  return 0;
}

int object_a_dtor(void *obj) {
  (void)obj;
  print("Hello, object_a_dtor!\n");
  return 0;
}

///=============================================================================

char object_b;
int object_b_ctor(void *obj) {
  (void)obj;
  print("Hello, object_b_ctor!\n");
  return 0;
}

int object_b_dtor(void *obj) {
  (void)obj;
  print("Hello, object_b_dtor!\n");
  // _NLR_panic(0);
  return 0;
}

///=============================================================================

static struct _NLR_symbol_t symbols_raii[] = {
    {&object_a, object_a_ctor, object_a_dtor},
    {&object_b, object_b_ctor, object_b_dtor},
};

#define OBJECT_COUNT (sizeof(symbols_raii) / sizeof(struct _NLR_symbol_t))

struct _NLR_symbol_t *_NLR_symbols_start = &symbols_raii[0];
struct _NLR_symbol_t *_NLR_symbols_end = &symbols_raii[0] + OBJECT_COUNT;

int _Q3app4mainFimPPhPPhE_0(_NLR_uword_t argc, _NLR_argv_pp_t argv,
                            _NLR_env_pp_t envp) {
  const int buf_size = 15;
  uint8_t *ptr = (uint8_t *)_NLR_malloc(buf_size);

  if (_NLR_csprng((_NLR_uword_t)ptr, buf_size) != buf_size) {
    print("Error: _NLR_csprng failed\n");
    return 1;
  }

  print("_NLR_csprng: ");
  for (int i = 0; i < buf_size; i++) {
    static const char hex[] = "0123456789ABCDEF";

    char buf[3] = {hex[ptr[i] >> 4], hex[ptr[i] & 0xF], '\0'};
    print(buf);
  }

  print("\n");

  _NLR_free((_NLR_uword_t)ptr);

  (void)argc;
  (void)argv;
  (void)envp;

  return 0;
}
